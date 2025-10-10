IMAGE = recall
CONTAINER = recall_container

CUDA ?= ON
TARGET ?= server
GPUS ?= all
CTX ?= 10

build:
	docker build \
	--build-arg ENABLE_CUDA=$(ENABLE_CUDA) \
	--build-arg BUILD_TARGET=$(TARGET) \
	-t $(IMAGE) .

run:
	docker run -p 8000:8000 \
	-v $(PWD)/models:/app/models \
	-v $(PWD)/data:/app/data \
	$(if $(GPUS),--gpus $(GPUS)) \
	--rm -t $(IMAGE) ./recall_$(TARGET)

compose-build:
	TARGET=$(TARGET) ENABLE_CUDA=$(CUDA) CTX_LIMIT=$(CTX) docker compose build

compose-up:
	TARGET=$(TARGET) ENABLE_CUDA=$(CUDA) CTX_LIMIT=$(CTX) docker compose up

compose-down:
	docker compose down

cb: compose-build

cu: compose-up

test: clean-db test-memoization test-benchmarks

clean-db:
	sqlite3 data/memory.db "DELETE FROM prompts;"

test-memoization:
	python3 tests/memoise.py

test-benchmarks:
	python3 tests/benchmark.py
