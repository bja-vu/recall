cmake_minimum_required(VERSION 3.16)
project(recall CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/llama.cpp)

add_executable(server server/main.cpp)

target_include_directories(server PRIVATE
	server/include
	external/llama.cpp/include
)

target_link_libraries(server PRIVATE llama)
