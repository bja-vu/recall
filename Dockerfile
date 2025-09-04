FROM nvidia/cuda:11.8.0-cudnn8-devel-ubuntu22.04

RUN apt-get update && apt-get install -y \
	build-essential \
	cmake \
	git \
	pkg-config \
	libssl-dev \
	curl \
	wget \
	python3 \
	python3-dev \
	python3-pip \
	libasio-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN ln -s /usr/local/cuda/lib64/stubs/libcuda.so /usr/local/cuda/lib64/stubs/libcuda.so.1 && \
    echo "/usr/local/cuda/lib64/stubs" > /etc/ld.so.conf.d/cuda-stubs.conf && \
    ldconfig

RUN mkdir -p build && \
	cd build && \
	cmake .. -DGGML_CUDA=ON && \
	make -j$(nproc)

EXPOSE 8000

CMD ["./server"]
