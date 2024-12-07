#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include "c++23/expected.hpp"
#include "c++23/print.hpp"

#include "wsa.hpp"
#include "thread_pool.hpp"

int main() {
    WSA wsa;

    Socket connect_socket = CreateConnectSocket();
    if (!connect_socket.valid()) {
        Println("Could not create connect socket, error code: {}: {}", WSA::GetLastError(), WSA::GetLastErrorAsString());
        return 1;
    }

    struct sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = inet_addr("127.0.0.1");
    to.sin_port = htons(static_cast<u_short>(std::atoi(PORT)));

    std::string input;
    while (true) {
        Print(">>> ");
        std::getline(std::cin, input);
        
        connect_socket.SendData(input, to);

        auto recv_data = connect_socket.RecieveData();
        if (!recv_data.has_value()) {
            Println("Could not recieve data from server, error code: {}: {}", WSA::GetLastError(), WSA::GetLastErrorAsString());
            continue;
        }

        Println("Recieved '{}'", recv_data.value().data);
    }

    return 0;
}