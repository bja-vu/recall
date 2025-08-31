cmake_minimum_required(VERSION 3.16)
project(recall CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/llama.cpp)
add_subdirectory(external/Crow)

add_executable(server server/main.cpp)

target_include_directories(server PRIVATE
	external/llama.cpp/include
	external/Crow/include
	external/asio/asio/include
)

target_link_libraries(server PRIVATE llama)
