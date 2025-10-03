#include "db.h"
#include "crow.h"
#include "llama.h"
#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <string>
#include <chrono>

const char* mp = std::getenv("MODEL_PATH");
const std::string model_path = mp ? std::string(mp) : "/app/models/Mistral-Nemo-Instruct-2407-Q4_K_M.gguf";
const int ngl = 99;
const int n_predict = 256; //128

const std::string prompt_tune =     
	//"### SYSTEM INSTRUCTIONS\n"
	"You are a concise assistant.\n"
	"Always answer in two sentences or under 50 words. \n"
	"Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
	"If something doesn't exist or is wrong, say so briefly.\n"
	"For programming questions, show code instead of explanations.\n"
	"Use markdown: *italics*, **bold**, `code`, ```code blocks```.\n"
	"Infer missing context from previous messages. Never ask for clarification.\n";
	//"### END SYSTEM INSTRUCTIONS\n";

llama_model* model;
llama_context* ctx;
const llama_vocab* vocab;
llama_sampler* smpl;

int init_model() {
	ggml_backend_load_all();
	llama_model_params model_params = llama_model_default_params();
	model_params.n_gpu_layers = ngl;
	if (llama_supports_gpu_offload()) {
	    printf("GPU backend is available!\n");
	} else {
	    printf("No GPU backend detected.\n");
	}
	model = llama_model_load_from_file(model_path.c_str(), model_params);
	if (model == NULL) {
		printf("error: unable to load model.\n");
		return 1;
	}

	vocab = llama_model_get_vocab(model);

	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 4096; //2048
	ctx_params.n_batch = 1024; //512

	ctx = llama_init_from_model(model, ctx_params);
	if (ctx == NULL) {
		printf("error: failed to create context.\n");
		return 1;
	}

	auto sparams = llama_sampler_chain_default_params();
	smpl = llama_sampler_chain_init(sparams);

	// match python tuning
	//llama_sampler_chain_add(smpl, llama_sample_init_temp(0.2f));
	// or just do greedy
	llama_sampler_chain_add(smpl, llama_sampler_init_greedy());
	
	return 0;
}

void reset_ctx() {
	if (ctx != NULL) {
		llama_free(ctx);
	}
	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 4096; // 2048
	ctx_params.n_batch = 1024; // 512
	ctx = llama_init_from_model(model, ctx_params);

	if (ctx == NULL) {
		printf("error: failed to reset context.\n");
	}
}

std::string run_llm(const std::string& prompt) {
	std::string final_prompt = "[INST] <<SYS>>\n" + prompt_tune + "<</SYS>>\n" + prompt + " [/INST]";
	int n_prompt = -llama_tokenize(vocab, final_prompt.c_str(), final_prompt.size(), NULL, 0, true, true);
	std::vector<llama_token> prompt_tokens(n_prompt);

	if (llama_tokenize(vocab, final_prompt.c_str(), final_prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
	    printf("error: failed to tokenize prompt.\n");
	    return "";
	}

	llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
	std::string output;
	llama_token new_token_id;

	for (int n_pos = 0; n_pos + batch.n_tokens < n_prompt + n_predict;) {
	    if (llama_decode(ctx, batch)) { printf("error: llama_decode failed"); return ""; }
	    n_pos += batch.n_tokens;
	    new_token_id = llama_sampler_sample(smpl, ctx, -1);
	    if (llama_vocab_is_eog(vocab, new_token_id)) break;

	    char buf[128];
	    int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
	    if (n < 0) continue;

	    output.append(buf, n);
	    batch = llama_batch_get_one(&new_token_id, 1);
	}

	// clear kv cache to avoid running out of memory slots
	//llama_kv_cache_clear(ctx); // OUTDATED
	//llama_kv_cache_seq_rm(ctx, -1, 0, -1);
	llama_sampler_reset(smpl);
	reset_ctx();
	return output;
}

// based on method from https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t cb(void* data, size_t size, size_t nmemb, void* clientp) {
	size_t total = size * nmemb;
	std::string* s = static_cast<std::string*>(clientp);
	s->append(static_cast<char*>(data), total);
	return total;
}

std::vector<float> get_embedding(std::string text) {
	CURL* curl = curl_easy_init();
	if (!curl) { 
		printf("error: curl init failed.\n"); 
		return std::vector<float>();
	}

	std::string response;
	std::string json_body = "{\"prompt\":\"" + text + "\"}";

	curl_easy_setopt(curl,CURLOPT_URL, "http://embed-service:5001/embed");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, "Content-Type: application/json"));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		printf("error: curl failed.\n");
		return std::vector<float>();
	}
	auto start = response.find('[');
	auto end = response.find(']');
	if (start == std::string::npos || end == std::string::npos) {
		printf("error: bad json.\n");
		return std::vector<float>();
	}

	std::string arr = response.substr(start + 1, end - start - 1);

	std::vector<float> vec;
	std::stringstream ss(arr);
	std::string num;
	while (std::getline(ss, num, ',')) {
		vec.push_back(std::stof(num));
	}
	printf("prompt encoded.\n");
	return vec;
}

float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) {
	if (a.size() != b.size()) {
		printf("error: vectors must be the same length.\n");
	}
	float dot = 0.0f;
	float n_a = 0.0f;
	float n_b = 0.0f;

	for (size_t i = 0; i < a.size(); i++) {
		dot += a[i] * b[i];
		n_a += a[i] * a[i];
		n_b += b[i] * b[i];
	}
	if (n_a == 0.0f || n_b == 0.0f) { return 0.0f; } // dont div by zero
	return dot / (std::sqrt(n_a) * std::sqrt(n_b));
}

std::pair<float, int> find_similar_response(const Database& db, const std::vector<float>& emb) {
	// extract embeddings column
	// iterate through running cosine similarity on arg and each embedding
	// store highest score and its associated index (which equates to the col index)
	// return
	if (emb.size() == 0) { return {0.0f, 0}; }
	std::vector<std::vector<float>> embeddings = db.get_embeddings();
	float high_score = -1.0f;
	int idx = -1;
	for (int i = 0; i < embeddings.size(); i++) {
		float score = cosine_similarity(emb, embeddings[i]);
		if (score > high_score) { high_score = score; idx = i; }
	}
	std::pair<float, int> p = {high_score, idx};
	return p;
}

int main(int argc, char* argv[]) {
	int ctx_limit=10;
	if (argc > 1) { //should always be
	ctx_limit = std::stoi(argv[1]);
	}
	printf("%d\n\n\n",ctx_limit);

    	if (init_model() == 1) {
		return 1;
	}
	Database db("data/memory.db");
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([](){
		return "hello world";
	});

	CROW_ROUTE(app, "/recall").methods("POST"_method)([&db](const crow::request& req) {
		std::string lang = ""; // TODO: ADD HEURISTIC

		auto start_time = std::chrono::high_resolution_clock::now();

		auto body = crow::json::load(req.body);
		if (!body) return crow::response(400, "invalid input");
		std::string prompt = body["prompt"].s();

		// encoding
		std::vector<float> vec = get_embedding(prompt);
		if (vec.empty()) {
			printf("error: failed to generate embedding.\n");
			// return crow::response(500, "failed to generate embedding");
		}
		auto embed_time = std::chrono::high_resolution_clock::now();

		std::pair<float, int> most_sim = find_similar_response(db, vec);
		if (!(most_sim.first == 0.0f && most_sim.second == 0)) {
			std::pair<std::string, std::string> pr = db.get_entry(most_sim.second);
			printf("most similar prompt: (%s)\n", pr.first.c_str());
			printf("score: %f\n", most_sim.first);
			//printf("generated resp: (%s)\n", pr.second.c_str());
			printf("\n-----------\n\n");
		}
		auto search_time = std::chrono::high_resolution_clock::now();
		
		std::string resp = run_llm(prompt);
		
		auto gen_time = std::chrono::high_resolution_clock::now();

		db.savePrompt(prompt, resp, "recall", vec, lang);
		crow::json::wvalue res;
		res["text"] = resp.empty() ? "error: generation failed" : resp;

		auto embed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(embed_time - start_time).count();
		auto search_ms = std::chrono::duration_cast<std::chrono::milliseconds>(search_time - embed_time).count();
		auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gen_time - search_time).count();
		auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gen_time - start_time).count();

		res["timing"]["embed"] = embed_ms;
		res["timing"]["search"] = search_ms;
		res["timing"]["gen"] = gen_ms;
		res["timing"]["total"] = total_ms;

		printf("timing: embed=%ldms, search=%ldms, gen=%ldms, total=%ldms\n", embed_ms, search_ms, gen_ms, total_ms);
		return crow::response(res);
	});

	CROW_ROUTE(app, "/chat").methods("POST"_method)([&db, ctx_limit](const crow::request& req) {
		// variables used for memoisation, chat prompts aren't memoised
		std::string lang = "";
		std::vector<float> empty_vec;

		auto start_time = std::chrono::high_resolution_clock::now();

		auto body = crow::json::load(req.body);
		if (!body) return crow::response(400, "invalid input");
		std::string userPrompt = body["prompt"].s();

		// TODO: not needed, just delete
		/*
		std::vector<float> vec = get_embedding(userPrompt);
		if (vec.empty()) {
			printf("error: failed to generate embedding.\n");
			// return crow::response(500, "failed to generate embedding");
		}
		*/

		std::string history = db.chatHistoryStr(ctx_limit);
		
		auto history_time = std::chrono::high_resolution_clock::now();

		std::string prompt = history + "User: " + userPrompt + "\nAssistant: ";
		printf("\n---FULL PROMPT SENT TO LLM---\n%s\n---END PROMPT---\n\n", prompt.c_str());
		std::string resp = run_llm(prompt);

		auto gen_time = std::chrono::high_resolution_clock::now();

		db.savePrompt(userPrompt, resp, "chat", empty_vec, lang);
		crow::json::wvalue res;
		res["text"] = resp.empty() ? "error: generation failed" : resp;

		auto history_ms = std::chrono::duration_cast<std::chrono::milliseconds>(history_time - start_time).count();
		auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gen_time - history_time).count();
		auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gen_time - start_time).count();

		res["timing"]["history"] = history_ms;
		res["timing"]["gen"] = gen_ms;
		res["timing"]["total"] = total_ms;

		printf("timing: history=%ldms, gen=%ldms, total=%ldms\n", history_ms, gen_ms, total_ms);

		return crow::response(res);
	});

	CROW_ROUTE(app, "/history").methods("POST"_method)([&db](const crow::request& req) {
		auto body = crow::json::load(req.body);
		if (!body) return crow::response(400, "invalid input");

		std::string search;
		int limit;
		
		if (body.has("search")) {
			std::cout << "search" << search << std::endl;
			search = body["search"].s();
		}
		if (body.has("limit")) {
			std::cout << "limit: " << limit << std::endl;
			limit = body["limit"].i();
		}
		
		auto rows = db.historySearch(search, limit);
		// convert vec<pair<str,str>> to json list
		crow::json::wvalue res = crow::json::wvalue::list(rows.size());
		for (size_t i = 0; i < rows.size(); i++) {
			auto& e = rows[i];
			crow::json::wvalue entry;
			entry["prompt"] = e.first;
			entry["response"] = e.second;
			res[i] = std::move(entry); // wvalue needs move
		}
		return crow::response(res);
	});

	app.port(8000).multithreaded().run();
}
