FROM nvidia/cuda:11.8.0-cudnn8-devel-ubuntu22.04

# Install build dependencies
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

# Copy llama.cpp source
COPY external ./external

# Set up CUDA stubs for build
RUN ln -s /usr/local/cuda/lib64/stubs/libcuda.so /usr/local/cuda/lib64/stubs/libcuda.so.1 && \
    echo "/usr/local/cuda/lib64/stubs" > /etc/ld.so.conf.d/cuda-stubs.conf && \
    ldconfig

# Build llama.cpp with CUDA
RUN cd external/llama.cpp && \
    rm -rf build && mkdir build && cd build && \
    cmake .. -DGGML_CUDA=ON -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath-link,/usr/local/cuda/lib64/stubs" && \
    make -j$(nproc) && make install

# Copy server code
COPY server ./server

# Build server
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

# Cleanup stubs AFTER all linking is done
RUN rm -f /etc/ld.so.conf.d/cuda-stubs.conf || true && \
    rm -f /usr/local/cuda/lib64/stubs/libcuda.so* || true && \
    ldconfig

CMD ["./recall_server"]
