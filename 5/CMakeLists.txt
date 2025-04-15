cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME ping)
project(${PROJECT_NAME} LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_executable(ping Ping.c resolve.c)

