CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

add_library(Socket STATIC Socket.cpp)

target_include_directories(Socket PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Socket ws2_32.lib Address)