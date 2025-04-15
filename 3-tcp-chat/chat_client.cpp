#include <string>
#include <iostream>
#include <csignal>
#include <future>
#include <chrono>

#include "c++23/print.hpp"

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
    if (!connect_socket.valid()) {
        Println("Failed to create connect socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    
    if (int err = connect_socket.SetBlocking(false); err != 0) {
        Println("ioctlsocket failed, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    Println(" ----- CLIENT ----- ");
    Println();
    
    std::thread stdin_input_thread([]() {
        std::set_terminate([]() {
            // this seems very dumb, but i guess it's ok for now
            std::exit(0);
        });

        while (true) {
            if (!g_stdin_input_ready.load(std::memory_order_acquire)) {
                Print(">>> ");
                std::getline(std::cin, g_stdin_input);
                g_stdin_input_ready.store(true, std::memory_order_release);
            }
        }
    });

    while (true) {
        if (g_stdin_input_ready.load(std::memory_order_acquire)) {
            // We have stdin --> Send it
            auto send_res = connect_socket.SendData(g_stdin_input);
            // Error
            if (!send_res && send_res.error() != WSAEWOULDBLOCK) {
                Println("Failed to send data, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
                return 1;
            }
            // Success --> Wait for new input
            else if (send_res) {
                Println("Sent {} bytes", send_res.value());
                g_stdin_input_ready.store(false, std::memory_order_release);
            } else {} // WSAEWOULBLOCK case --> do nothing
        }

        auto recv_data = connect_socket.RecieveData();
        if (!recv_data && recv_data.error() != WSAEWOULDBLOCK) {
            Println("Failed to recieve data, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
            return 1;
        }

        if (recv_data) {
            Println("Recieved: {}", recv_data.value());
            Print(">>> ");
        }
    }
    
    return 0;
}