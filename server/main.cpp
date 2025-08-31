#include "crow.h"
#include "llama.h"
#include <iostream>
#include <vector>

const std::string model_path = "../models/capybarahermes-2.5-mistral-7b.Q4_K_M.gguf";
const std::string prompt= "the capital of portugal is";
const int ngl = 99;
const int n_predict = 32;

int main() {
	ggml_backend_load_all();
	llama_model_params model_params = llama_model_default_params();
	model_params.n_gpu_layers = ngl;
	
	llama_model* model = llama_model_load_from_file(model_path.c_str(), model_params);
	if (model == NULL) {
		printf("error: unable to load model");
		return 1;
	}
	const llama_vocab* vocab = llama_model_get_vocab(model);
	const int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);

	std::vector<llama_token> prompt_tokens(n_prompt);
	if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
		printf("error: failed to tokenize prompt\n");
		return 1;
	}

	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = n_prompt + n_predict -1;
	ctx_params.n_batch = n_prompt;
	ctx_params.no_perf = false;

	llama_context* ctx = llama_init_from_model(model, ctx_params);

	    // initialize the sampler

    auto sparams = llama_sampler_chain_default_params();
    sparams.no_perf = false;
    llama_sampler * smpl = llama_sampler_chain_init(sparams);

    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    // print the prompt token-by-token

    for (auto id : prompt_tokens) {
        char buf[128];
        int n = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, true);
        if (n < 0) {
            fprintf(stderr, "%s: error: failed to convert token to piece\n", __func__);
            return 1;
        }
        std::string s(buf, n);
        printf("%s", s.c_str());
    }

    // prepare a batch for the prompt

    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());

    // main loop

    const auto t_main_start = ggml_time_us();
    int n_decode = 0;
    llama_token new_token_id;

    for (int n_pos = 0; n_pos + batch.n_tokens < n_prompt + n_predict; ) {
        // evaluate the current batch with the transformer model
        if (llama_decode(ctx, batch)) {
            fprintf(stderr, "%s : failed to eval, return code %d\n", __func__, 1);
            return 1;
        }

        n_pos += batch.n_tokens;

        // sample the next token
        {
            new_token_id = llama_sampler_sample(smpl, ctx, -1);

            // is it an end of generation?
            if (llama_vocab_is_eog(vocab, new_token_id)) {
                break;
            }

            char buf[128];
            int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
            if (n < 0) {
                fprintf(stderr, "%s: error: failed to convert token to piece\n", __func__);
                return 1;
            }
            std::string s(buf, n);
            printf("%s", s.c_str());
            fflush(stdout);

            // prepare the next batch with the sampled token
            batch = llama_batch_get_one(&new_token_id, 1);

            n_decode += 1;
        }
    }

    printf("\n");

    const auto t_main_end = ggml_time_us();

    fprintf(stderr, "%s: decoded %d tokens in %.2f s, speed: %.2f t/s\n",
            __func__, n_decode, (t_main_end - t_main_start) / 1000000.0f, n_decode / ((t_main_end - t_main_start) / 1000000.0f));

    fprintf(stderr, "\n");
    llama_perf_sampler_print(smpl);
    llama_perf_context_print(ctx);
    fprintf(stderr, "\n");

    /*llama_sampler_free(smpl);
    llama_free(ctx);
    llama_model_free(model);*/

    	
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([](){
		return "hello world";
	});

	app.port(8000).multithreaded().run();
	
}
