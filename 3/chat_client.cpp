#include <string>
#include <iostream>
#include <print>
#include <csignal>
#include <future>
#include <chrono>

#include "wsa.hpp"

std::atomic<bool> g_stdin_input_ready = false;
std::string g_stdin_input;

int main() {
    std::signal(SIGINT, [](int) {
        // basically cleans up WSA
        // not the best way to do it
        // but it will at least make some info printed to stdout
        std::exit(0);
    });

    Socket connect_socket = CreateConnectSocket();
    if (connect_socket == INVALID_SOCKET) {
        std::println("Failed to create connect socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    
    if (int err = connect_socket.SetBlocking(false); err != 0) {
        std::println("ioctlsocket failed, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    std::println(" ----- CLIENT ----- ");
    std::println();
    
    std::thread stdin_input_thread([]() {
        std::set_terminate([]() {
            // this seems very dumb, but i guess it's ok for now
            std::exit(0);
        });

        while (true) {
            if (!g_stdin_input_ready.load(std::memory_order_acquire)) {
                std::print(">>> ");
                std::getline(std::cin, g_stdin_input);
                g_stdin_input_ready.store(true, std::memory_order_release);
            }
        }
    });

    while (true) {
        if (g_stdin_input_ready.load(std::memory_order_acquire)) {
            // We have stdin --> Send it
            auto send_res = SendData(connect_socket, g_stdin_input);
            // Error
            if (!send_res && send_res.error() != WSAEWOULDBLOCK) {
                std::println("Failed to send data, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
                return 1;
            }
            // Success --> Wait for new input
            else if (send_res) {
                std::println("Sent {} bytes", send_res.value());
                g_stdin_input_ready.store(false, std::memory_order_release);
            } else {} // WSAEWOULBLOCK case --> do nothing
        }

        auto recv_data = RecieveData(connect_socket);
        if (!recv_data && recv_data.error() != WSAEWOULDBLOCK) {
            std::println("Failed to recieve data, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
            return 1;
        }

        if (recv_data) {
            std::println("Recieved: {}", recv_data.value());
            std::print(">>> ");
        }
    }
    
    return 0;
}