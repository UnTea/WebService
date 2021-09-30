CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

add_library(Address STATIC Address.cpp)

target_include_directories(Address PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Address ws2_32.lib)