#include <cstdint>
#include <cstdio>
#include <cstring>

#include <string>
#include <fstream>

#include <gzip/decompress.hpp>

#include "app.hpp"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "sendbufs.h"

std::string recieve_data(const char *name, const char *port, const char *sendbuf);
void        get_file_from_network(const char *sendbuf, const char *out_filename);

int main() {
    // Recieve the files from network
    get_file_from_network(REQ_HTML, "assets/index.html");
    get_file_from_network(REQ_STYLE_CSS, "assets/css/style.css");
    get_file_from_network(REQ_RESET_CSS, "assets/css/reset.css");
    get_file_from_network(REQ_GRID_CSS, "assets/css/grid.css");

    // Run the app which renders the file
    Application app;
    printf("App created!\n");
    printf("Running!\n");
    app.Run();
    printf("Done!\n");

    return 0;
}

std::string recieve_data(const char *name, const char *port, const char *sendbuf) {
    WSADATA wsa_data;
    SOCKET connect_socket = INVALID_SOCKET;

    // Initialize Winsock
    if (int err = WSAStartup(MAKEWORD(2,2), &wsa_data); err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return {};
    }

    struct addrinfo hints = {0},
                    *result = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    if (int err = getaddrinfo(name, port, &hints, &result); err != 0) {
        printf("getaddrinfo failed with error: %d\n", err);
        WSACleanup();
        return {};
    }

    // Attempt to connect to an address until one succeeds
    for(struct addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (connect_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return {};
        }

        // Connect to server.
        if (connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(connect_socket);
            connect_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (connect_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return {};
    }

    // Send an initial buffer

    int bytes_sent = send(connect_socket, sendbuf, (int)std::strlen(sendbuf), 0);
    if (bytes_sent == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connect_socket);
        WSACleanup();
        return {};
    }

    printf("\n\nBytes Sent: %ld\n", bytes_sent);

    // shutdown the connection since no more data will be sent
    if (shutdown(connect_socket, SD_SEND) == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connect_socket);
        WSACleanup();
        return {};
    }

    // Receive until the peer closes the connection
    std::string recieved_bytes;
    const int recvbuflen = 512;
    char recvbuf[recvbuflen];
    int bytes_recieved;
    do {
        bytes_recieved = recv(connect_socket, (char *)recvbuf, recvbuflen, 0);
        if (bytes_recieved > 0) {
            printf("Bytes received: %d\n", bytes_recieved);
            recieved_bytes.append(recvbuf, bytes_recieved);
        }
        else if (bytes_recieved == 0) {
            printf("Connection closed\n");
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }

    } while(bytes_recieved > 0);

    printf("\nBytes Recieved: %lld\n", recieved_bytes.size());

    // cleanup
    closesocket(connect_socket);
    WSACleanup();

    return recieved_bytes;
}

void get_file_from_network(const char *sendbuf, const char *out_filename) {

    auto load_string = [](const std::string &string, const std::string &filename) {
        std::ofstream file(filename, std::ios_base::out | std::ios_base::binary);
        if (!file) {
            printf("Error loading '%s'\n", filename.c_str());
            std::exit(1);
        }

        file.write(string.data(), string.size());
        if (file.fail()) {
            printf("Error writing to '%s'\n", filename.c_str());
            std::exit(1);
        }

        file.close();
    };

    // Recieve data
    auto bytes = recieve_data("pmk.tversu.ru", "80", sendbuf);

    if (bytes.empty()) {
        printf("Could not recieve data\n");
        std::exit(1);
    }
    
    // Save raw recieved data
    static int n = 1;
    load_string(bytes, std::string("raw_bytes_") + std::to_string(n++));

    // Get compressed part
    std::size_t header_ends = bytes.find("\r\n\r\n");
    printf("Header ends at position: %lld\n", header_ends);
    
    // Decompress
    auto decompressed = gzip::decompress(bytes.data() + header_ends + 4, bytes.size() - header_ends - 4);

    // Load decompressed data to file
    load_string(decompressed, out_filename);
}