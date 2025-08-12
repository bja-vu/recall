## recall - an offline command-line assistant

server command: ./external/llama.cpp/build/bin/llama-server -m "models/capybarahermes-2.5-mistral-7b.Q4_K_M.gguf"

optional: `--verbose`

GPU running is significantly faster. To enable, add the arg `--n-gpu-layers x`

Test numbers:
total time =   11222.49 ms /    71 tokens -- 10 layers
total time =   20689.93 ms /   122 tokens -- 0 layers (CPU)
total time =    1063.58 ms /    71 tokens -- 35 layers

To use GPU, you need CUDA toolkit installed (both on windows and in WSL). Then, build:
https://github.com/ggml-org/llama.cpp/blob/master/docs/build.md#cuda

models typically display the max number of layers.
## TODO
- [x] formatting parser converting markdown into rich markup
- [x] Database integration and history printing
> Note: the history currently includes chat messages
- [ ] semi-automatic performance improvement
> Basic Memoisation using prompt comparison (Full-Auto)
> Memoisation using prompt "*keyword*" comparison and User Confirmation (Semi-Auto)
> Cosine Similarity using ML techniques and converting prompts to vectors (Full-Auto)
- [x] Conversation mode with basic context
- [ ] Fuzzy search using `history` with an argument, returns *n* prompt/response pairs with the arg featured in the prompt or response.
