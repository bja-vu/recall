#include "crow.h"
#include "llama.h"
#include <iostream>
#include <vector>

const std::string model_path = "../models/capybarahermes-2.5-mistral-7b.Q4_K_M.gguf"; // #TODO get model path from args
const std::string prompt= "the capital of portugal is";
const int ngl = 99;
const int n_predict = 128;

const std::string prompt_tune =     
	"Always answer in two sentences or under 50 words. No extra explanation. No notes.\n"
	"Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
	"If the question refers to something that does not exist or is incorrect, say so. Do not answer untruthfully.\n"
	"If the question is programming related, be pragmatic with your answers. Opt for code instead of descriptions.\n"
	"Reply using markdown syntax only. Use one asterisk (*text*) for italics, two asterisks (**text**) for bold, and backticks (`text`) for inline code.\n"
	"Infer missing context from previous messages. Never ask for clarification.\n";

llama_model* model;
llama_context* ctx;
const llama_vocab* vocab;
llama_sampler* smpl;

int init_model() {
	ggml_backend_load_all();
	llama_model_params model_params = llama_model_default_params();
	model_params.n_gpu_layers = ngl;

	model = llama_model_load_from_file(model_path.c_str(), model_params);
	if (model == NULL) {
		printf("error: unable to load model.\n");
		return 1;
	}

	vocab = llama_model_get_vocab(model);

	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 2048;
	ctx_params.n_batch = 512;

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

std::string run_llm(const std::string& prompt) {
	std::string final_prompt = "\nUser: " + prompt + "\nAssistant: ";

	int n_prompt = -llama_tokenize(vocab, final_prompt.c_str(), final_prompt.size(), NULL, 0, true, true);
	std::vector<llama_token> prompt_tokens(n_prompt);
	if (llama_tokenize(vocab, final_prompt.c_str(), final_prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
		// TODO: error handling
		printf("error: failed to tokenize prompt.\n");
		return "";
		}
	llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());

	std::string output;
	llama_token new_token_id;

	for (int n_pos = 0; n_pos+batch.n_tokens < n_prompt + n_predict;) {
		if (llama_decode(ctx, batch)) {
			printf("error: llama_decode failed");
			return "";
		}
	n_pos += batch.n_tokens;
	new_token_id = llama_sampler_sample(smpl, ctx, -1);
	if (llama_vocab_is_eog(vocab, new_token_id)) break;

	char buf[128];
	int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
	if (n < 0) continue;

	output.append(buf, n);

	batch = llama_batch_get_one(&new_token_id, 1);
	}
	return output;
}

int main() {
    	if (init_model() == 1) {
		return 1;
	}
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([](){
		return "hello world";
	});

	CROW_ROUTE(app, "/generate").methods("POST"_method)([](const crow::request& req) {
		auto body = crow::json::load(req.body);
		if (!body) { return crow::response(400, "Invalid JSON"); }

		std::string prompt = body["prompt"].s();

		std::string response = run_llm(prompt);
		if (response == "") {
			crow::json::wvalue err;
			err["error"] = "error: response was empty.";
			return crow::response(500, err.dump());
		}
		crow::json::wvalue res;
		res["msg"] = response;
		return crow::response(res);
	});

	app.port(8000).multithreaded().run();
	
}
