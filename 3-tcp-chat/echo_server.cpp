#include <string>
#include <list>
#include <thread>
#include <csignal>

#include "c++23/print.hpp"

#include "wsa.hpp"
#include "thread_pool.hpp"

void Echo(Socket socket) {
    // Recieve
    auto recv_data = socket.RecieveData();
    if (!recv_data) {
        Print("Failed to recieve data, error code: {}: {}", recv_data.error(), wsa.ErrorCodeToString(recv_data.error()));
        return;
    }

    std::string data = std::move(recv_data.value());
    Println("Recieved Data: '{}'", data);

    // Send back the same
    auto send_res = socket.SendData(data);
    if (!send_res) {
        Println("Failed to send data back, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return;
    }
    
    // Shutdown, we don't send anymore
    if (shutdown(socket, SD_SEND) == SOCKET_ERROR) {
        Println("Failed to shutdwon socket, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
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
    if (!listen_socket.valid()) {
        Println("Could not create listen socket");
        return 1;
    }

    Println(" ----- SERVER ----- ");
    Println();

    while (true) {
        Println("Waiting for connection...");

        Socket client_socket = accept(listen_socket, NULL, NULL);
        if (!client_socket.valid()) {
            Println("accept returned invalid socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
            continue;
        }

        pool.NewTask(Echo, std::move(client_socket));
    }

    return 0;
}