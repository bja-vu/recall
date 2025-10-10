#include "db.h"
#include "llama.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include <string>
#include <cmath>

// embedding helper funcs
float cosine(const std::vector<float>& a, const std::vector<float>&b) {
	float dot = 0;
	float na = 0;
	float nb = 0;
	for (size_t i = 0; i < a.size(); i++) {
		dot += a[i]*b[i];
		na += a[i] * a[i];
		nb += b[i] * b[i];
	}
	return dot / (std::sqrt(na) * std::sqrt(nb));
}

std::vector<float> embed(llama_model* model, llama_context* ctx, const std::string& text) {
    const llama_vocab* vocab = llama_model_get_vocab(model);

    std::vector<llama_token> tokens(text.size() + 8);
    int n_tokens = llama_tokenize(
        vocab,
        text.c_str(),
        text.length(),
        tokens.data(),
        tokens.size(),
        true,   // add BOS
        false   // no special tokens
    );
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i]     = tokens[i];
        batch.pos[i]       = i;
        batch.n_seq_id[i]  = 1;
        batch.seq_id[i][0] = 0;
    }
    batch.n_tokens = n_tokens;

    if (llama_encode(ctx, batch) != 0) {
        std::cerr << "llama_encode() failed\n";
        return {};
    }

    const float* emb = llama_get_embeddings(ctx);
    int dim = llama_model_n_embd(model);

    return std::vector<float>(emb, emb + dim);
}


int main() {
	std::string TEST_DIR = "data/test.db";
	Database db(TEST_DIR);

	// insert with vec
	std::vector<float> vec = {1.0f, 2.0f, 3.0f};
	db.savePrompt("hello?", "world!", "recall", vec, "en");

	// insert without vec
	db.savePrompt("ping", "pong", "chat", {}, "en");

	// add more entries
	db.savePrompt("ping", "135ms", "recall", {}, "en");
	db.savePrompt("ping", "150ms", "recall", {}, "en");
	db.savePrompt("foo", "bar", "chat", {}, "en");
	// history
	auto rows = db.historySearch(std::nullopt,3);
	std::cout << "got " << rows.size() << " rows\n";
	for (auto& row : rows) {
		std::cout << "Q: " << row.first << "\nA: " << row.second << "\n---\n";
	}
	// search
	std::string query = "ping";
	auto searchRows = db.historySearch(query, 2);
	std:: cout << "got " << rows.size() << " rows \n";

	for (auto& row : searchRows) {
		std::cout << "Q: " << row.first << "\nA: " << row.second << "\n---\n";
	}

	db.close();

	// cleanup
	if (std::remove(TEST_DIR.c_str()) == 0) {
		std::cout << "test db removed.\n";
	} else {
		std::cout << "test db not removed.\n";
	}

	// EMBEDDING TESTS
	llama_model_params mp = llama_model_default_params();
	mp.n_gpu_layers = 0;
	llama_model* em = llama_model_load_from_file("models/all-MiniLM-L6-v2-Q4_K_M.gguf", mp);
	llama_context_params cp = llama_context_default_params();
	cp.n_ctx = 512;
	llama_context* ec = llama_new_context_with_model(em, cp);

	auto v1 = embed(em, ec, "how do I use dicts in python");
	auto v2 = embed(em, ec, "how do I make a dictionary in python");
	auto v3 = embed(em, ec, "what is the capital of France");

	std::cout << "sim(v1,v2) = " << cosine(v1, v2) << "\n";
	std::cout << "sim(v1,v3) = " << cosine(v1, v3) << "\n";

	return 0;
}
