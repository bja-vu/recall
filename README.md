# Recall

A lightweight, fully local CLI assistant for programmers. Instant syntax reminders and conceptual clarification without leaving the terminal.

## About
Recall addresses the friction of cloud-based AI tools by providing:
- Fast, local responses
- Privacy
- Concise answers
- Two modes:
    - recall: Stateless queries for quick lookups
    - chat: Stateful queries for deeper exploration

Built for programmers who want AI assistance without heavy reliance on generated solutions

## Requirements

### Hardware
- GPU: NVIDIA GPU with CUDA support (RTX 3060 or better recommended)
- CPU: Any modern CPU (runs significantly slower, 10s+ response times)
- 16GB+ RAM recommended

### Software
- Docker and Docker Compose
- NVIDIA Container Toolkit
- Make (optional, recommended)

### Language Model
- Download a GGUF model file (recommended: Mistral-Nemo-Instruct-2407)
- Get models from [Hugging Face](https://huggingface.co/bartowski/Mistral-Nemo-Instruct-2407-GGUF)

## Installation

### 1. Clone the repository
```bash
git clone https://github.com/bja-vu/recall.git
cd recall
```
### 2. Download a model
```bash
mkdir -p models
cd models
# Download a Language Model in GGUF format
# Example: wget https://huggingface.co/bartowski/Mistral-Nemo-Instruct-2407-GGUF/resolve/main/Mistral-Nemo-Instruct-2407-Q4_K_M.gguf
cd ..
```
### 3. Configure model path (optional)
If not utilising Mistral-Nemo-Instruct-2407, update the model path in `server/main.cpp`
```cpp
// Line ~12, look for:
const std::string model_path = "/app/models/your-model-name.gguf";
```
### 4. Build + Run
GPU Mode (Recommended)
```bash
# Build
make compose-build CUDA=ON
# Shorthand for Build
make cb

# Run
make compose-up CUDA=ON
#Shorthand for Run
make cu
# Or customize context limit (default 10)
make compose-up CUDA=ON CTX=20
```
CPU Mode
```bash
# Build
make compose-build CUDA=OFF

# Run
make compose-up CUDA=OFF
```
Without Make
```bash
# GPU mode
ENABLE_CUDA=ON CTX_LIMIT=10 docker compose --profile gpu up

# CPU mode
ENABLE_CUDA=OFF CTX_LIMIT=10 docker compose --profile no-gpu up
```
### 5. Client script
```bash
# Make the client script executable
chmod +x main.py

# Optional: Create aliases for convenience
echo "alias recall='python3 $(pwd)/main.py recall'" >> ~/.bashrc
echo "alias chat='python3 $(pwd)/main.py chat'" >> ~/.bashrc
source ~/.bashrc
```
## Usage
### Recall mode (quick lookups)
```bash
recall Python dictionaries
recall c++ vectors
recall rust ownership
```
### Chat mode (follow-up Questions)
```bash
recall Python decorators
chat how do I use them with classes?
chat what are some common use cases?
```
## Performance
### With GPU (RTX 4070):
- Simple queries: ~400-500ms
- Complex queries: ~2-4s

### CPU-only:
- Simple queries: ~10-20s
- Complex queries: ~30s+

Memoization helps with repeated queries.
## Testing
```bash
# Run all tests
make test

# Individual tests
make test-memoization  # Test semantic caching
make test-benchmarks   # Performance benchmarks

# Clean database
make clean-db
```
## Limitations
- Knowledge cutoff: ~2023 (model-dependent)
- Niche frameworks: May default to popular alternatives (38% accuracy on post-2023/niche tools in testing)
- GPU requirement: Practically requires NVIDIA GPU for reasonable performance
- Short prompt memoization: Very short prompts (< 3 words) may not memoize well across language aliases

## Future Work
- RAG implementation: Supplement with up-to-date documentation
- Improved memoization: Dynamic thresholds based on prompt length
- Configuration system: Model selection via config file
- IDE integration: Neovim/VSCode plugins for seamless workflow
- Better context management: Running summaries for longer conversations

## License
Capstone project by Bailey A.
