
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include "c++23/expected.hpp"
#include "c++23/print.hpp"

#include "wsa.hpp"
#include "thread_pool.hpp"

void Echo(const Socket &socket, const std::string &message, struct sockaddr_in to) {
    Println("Echoing '{}'", message);
    socket.SendData(message, to);
}

ThreadPool pool(10);

int main() {
    WSA wsa;

    Socket listen_socket = CreateListenSocket();
    if (!listen_socket.valid()) {
        Println("Could not create listen socket, error code: {}: {}", WSA::GetLastError(), WSA::GetLastErrorAsString());
        return 1;
    }

    while (true) {
        // You don't accept on UPD socket

        auto recv_data = listen_socket.RecieveData();
        
        if (!recv_data.has_value()) {
            Println("Failed to recieve from socket {}, error code: {}: {}",
                    static_cast<std::int64_t>(listen_socket), WSA::GetLastError(), WSA::GetLastErrorAsString());
            continue;
        }

        pool.NewTask(Echo, std::cref(listen_socket), recv_data.value().data, recv_data.value().from);
    }

    return 0;
}