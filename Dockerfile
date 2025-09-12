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
    libcurl4-openssl-dev \ 
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN ln -s /usr/local/cuda/lib64/stubs/libcuda.so /usr/local/cuda/lib64/stubs/libcuda.so.1 && \
    echo "/usr/local/cuda/lib64/stubs" > /etc/ld.so.conf.d/cuda-stubs.conf && \
    ldconfig

# Copy dependencies
COPY external ./external

# Build llama.cpp
RUN cd external/llama.cpp && \
    mkdir -p build && \
    cd build && \
    cmake .. -DGGML_CUDA=${ENABLE_CUDA} -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install

# Build Crow
RUN cd external/Crow && \
    mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Copy server code
COPY server ./server

# Hardcoded g++ build
RUN g++ -std=c++17 -O3 \
    -I/usr/local/include \
    -Iexternal/Crow/include \
    -Iexternal/asio/asio/include \
    server/main.cpp \
    -L/usr/local/lib \
    -lllama -lggml \
    -lssl -lcrypto -lpthread \
    -Wl,-rpath,/usr/local/lib \
    -o recall_server


EXPOSE 8000
CMD ["./recall_server"]
