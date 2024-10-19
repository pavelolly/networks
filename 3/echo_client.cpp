#include <string>
#include <iostream>
#include <print>

#include "wsa.hpp"

int main() {
    Socket connect_socket = CreateConnectSocket();
    if (connect_socket == INVALID_SOCKET) {
        std::println("Failed to create client socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    std::println(" ----- CLIENT ----- ");
    std::println();
    
    std::print(">>> ");
    std::string input_buffer;
    std::getline(std::cin, input_buffer);

    auto send_res = SendData(connect_socket, input_buffer);
    if (!send_res) {
        std::println("Failed to send data");
        return 1;
    }

    if (shutdown(connect_socket, SD_SEND) == SOCKET_ERROR) {
        std::println("Failed to shutdwon connect socket, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    auto recv_data = RecieveData(connect_socket);
    if (!recv_data) {
        std::println("Failed to recieve data, error code: {}: {}", recv_data.error(), wsa.ErrorCodeToString(recv_data.error()));
        return 1;
    }
    
    std::println("Recieved back: '{}'", recv_data.value());
    
    return 0;
}
