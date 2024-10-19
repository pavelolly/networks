cmake_minimum_required(VERSION 3.5)
project(TCPChat CXX)

set(CMAKE_CXX_STANDARD 23)

add_compile_options("/W4")

add_library(wsa STATIC wsa.cpp wsa.hpp)

# add_executable(echo_server echo_server.cpp thread_pool.hpp)
# add_executable(echo_client echo_client.cpp)
# 
# target_link_libraries(echo_server wsa)
# target_link_libraries(echo_client wsa)

add_executable(chat_server chat_server.cpp thread_pool.hpp)
add_executable(chat_client chat_client.cpp)

target_link_libraries(chat_server wsa)
target_link_libraries(chat_client wsa)

# add_executable(thread_pool_test thread_pool_test.cpp thread_pool.hpp)