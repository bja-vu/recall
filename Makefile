IMAGE = recall
CONTAINER = recall_container

CUDA ?= OFF
TARGET ?= server
GPUS ?= all

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
	TARGET=$(TARGET) ENABLE_CUDA=$(CUDA) docker compose build

compose-up:
	TARGET=$(TARGET) ENABLE_CUDA=$(CUDA) docker compose up

compose-down:
	docker compose down
