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
- [ ] save prompts and responses, print last 5-10 if just main.py is run with no args
- [ ] semi-automatic performance improvement
> store all prompts and responses
> when a prompt shares >*n*% of words with an existing prompt, ask user if previous response is alright, if so, add prompt to list of associated prompts and avoid re-creating a response
> if not, create a response
> metric of similarity is inconsistent, but it's simple to implement
> I could also strip "filler" words to extract important ones (e.g. how do i improve performance of my local llm >> "improve", "performance" "local", "llm") and compare those, but it'd still miss some context

- [x] conversation mode, toggleable by user after response is sent (might be annoying, instead could be a different command that uses the prior response which would be cached somewhere and overridden on the next NON-CONVO response)
- [ ] fuzzy search like "fetch hash" returns all prompt:response combos with hash in them (up to a limit)
