cmake_minimum_required(VERSION 3.20)
project(WebService)

set(CMAKE_CXX_STANDARD 20)

add_executable(WebService main.cpp)
target_link_libraries(WebService ws2_32.lib)