#include <string>
#include <print>
#include <list>
#include <thread>
#include <csignal>

#include "wsa.hpp"
#include "thread_pool.hpp"

void Echo(Socket socket) {
    // Recieve
    auto recv_data = RecieveData(socket);
    if (!recv_data) {
        std::print("Failed to recieve data, error code: {}: {}", recv_data.error(), wsa.ErrorCodeToString(recv_data.error()));
        return;
    }

    std::string data = std::move(recv_data.value());
    std::println("Recieved Data: '{}'", data);

    // Send back the same
    auto send_res = SendData(socket, data);
    if (!send_res) {
        std::println("Failed to send data back, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return;
    }
    
    // Shutdown, we don't send anymore
    if (shutdown(socket, SD_SEND) == SOCKET_ERROR) {
        std::println("Failed to shutdwon socket, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return;
    }
}

ThreadPool pool(100); // global so it cleans up on SIGINT

int main() {
    std::signal(SIGINT, [](int) {
        // basically cleans up WSA and ThreadPool (what if ~WSA() before ~ThreadPool() ???)
        // not the best way to do it
        // but it will at least make some info printed to stdout
        std::exit(0);
    });

    pool.SetLoggingFlag(false);

    Socket listen_socket = CreateListenSocket();
    if (listen_socket == INVALID_SOCKET) {
        std::println("Could not create listen socket");
        return 1;
    }

    std::println(" ----- SERVER ----- ");
    std::println();

    while (true) {
        std::println("Waiting for connection...");

        Socket client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::println("accept returned INVALID_SOCKET, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
            continue;
        }

        pool.NewTask(Echo, std::move(client_socket));
    }

    return 0;
}