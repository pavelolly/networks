project(networks_2 C CXX)

add_executable(client client.c)
add_executable(server server.cpp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/../)
install(TARGETS client server DESTINATION .)