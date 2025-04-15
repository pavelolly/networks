cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME UDPEcho)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("/W4")

add_library(wsa STATIC wsa.cpp)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server wsa)
target_link_libraries(client wsa)
