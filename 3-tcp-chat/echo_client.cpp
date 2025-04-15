#include <string>
#include <iostream>

#include "c++23/print.hpp"

#include "wsa.hpp"

int main() {
    Socket connect_socket = CreateConnectSocket();
    if (!connect_socket.valid()) {
        Println("Failed to create client socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    Println(" ----- CLIENT ----- ");
    Println();
    
    Print(">>> ");
    std::string input_buffer;
    std::getline(std::cin, input_buffer);

    auto send_res = connect_socket.SendData(input_buffer);
    if (!send_res) {
        Println("Failed to send data");
        return 1;
    }

    if (shutdown(connect_socket, SD_SEND) == SOCKET_ERROR) {
        Println("Failed to shutdwon connect socket, error code {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    auto recv_data = connect_socket.RecieveData();
    if (!recv_data) {
        Println("Failed to recieve data, error code: {}: {}", recv_data.error(), wsa.ErrorCodeToString(recv_data.error()));
        return 1;
    }
    
    Println("Recieved back: '{}'", recv_data.value());
    
    return 0;
}
