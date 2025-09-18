## recall - an offline command-line assistant

### Building
recall can be built using Docker. The dockerfile can produce wither recall_test or recall_server.

`docker run -p 8000:8000 -v $(pwd)/models:/app/models -v $(pwd)/data:/app/data --gpus all -t recall ./recall_test`

`docker build --build-arg ENABLE_CUDA=ON --build-arg BUILD_TARGET=test -t recall .`

GPU running is significantly faster. To enable, add the arg `--gpus all` to the Docker run command.

Test numbers:
total time =   11222.49 ms /    71 tokens -- 10 layers
total time =   20689.93 ms /   122 tokens -- 0 layers (CPU)
total time =    1063.58 ms /    71 tokens -- 35 layers

> Note that I've set up llama.cpp to use the maximum amount of GPU layers.

To use GPU, you need CUDA toolkit installed (both on windows and in WSL). Then, build:
https://github.com/ggml-org/llama.cpp/blob/master/docs/build.md#cuda


### Aliasing
using `alias` allows for clean usage of the tool in the CLI, but zsh restricts the usage of question marks (?) due to their usage in globbing.

My setups:
recall='python3 main.py r'

chat='python3 main.py c'

history='python3 main.py h'

---

## TODO
- [x] Custom C++ server using Crow, llama.cpp
- [x] SQLite integration
- [ ] String embedding and cosine similarity using a model from hugging face
- [x] Chat mode capabilities
- [ ] Docker caching properly to avoid rebuilding llama.cpp

- [ ] Inbuilt summarisation of last n messages when context size reaches limit

- [ ] Contextual inference and specialised model/prompt pairs for certain tasks
    - e.g. different models and prompt tunes for conceptual prompts vs programming
