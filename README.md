## recall - an offline command-line assistant

cmake .. -DGGML_CUDA=OFF
make  -j

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

## Aliasing
using `alias` allows for clean usage of the tool in the CLI, but zsh restricts the usage of question marks (?) due to their usage in globbing.

My setups:
recall='python3 main.py r'

chat='python3 main.py c'

history='python3 main.py h'
## TODO
- [ ] semi-automatic performance improvement
> Basic Memoisation using prompt comparison (Full-Auto)
> Memoisation using prompt "*keyword*" comparison and User Confirmation (Semi-Auto)
> Cosine Similarity using ML techniques and converting prompts to vectors (Full-Auto)

- [ ] Implement Vectorisation and memoisation into main
- [ ] Test memoisation and fine-tune

- [ ] Contextual inference and specialised model/prompt pairs for certain tasks
    - e.g. different models and prompt tunes for conceptual prompts vs programming
