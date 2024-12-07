#include "c++23/print.hpp"

#include "wsa.hpp"

int WSA::GetLastError() {
    return WSAGetLastError();
}

std::string WSA::ErrorCodeToString(int error_code) {
    LCID lcid = 0;
    char *error_as_c_str = nullptr;
    const int flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

    GetLocaleInfoEx(L"en-US", LOCALE_RETURN_NUMBER | LOCALE_ILANGUAGE, (LPWSTR)&lcid, sizeof(lcid));
    FormatMessageA(flags, NULL, error_code, lcid, reinterpret_cast<LPSTR>(&error_as_c_str), 0, NULL);
    
    std::string error_as_string(error_as_c_str);
    LocalFree(error_as_c_str);
    return error_as_string;
}

std::string WSA::GetLastErrorAsString() {
    return ErrorCodeToString(WSA::GetLastError());
}

Expected<RecievedData, int> Socket::RecieveData() const {
    const int recvbuf_len = 1024;
    char recvbuf[recvbuf_len] = {0};
    
    struct sockaddr_in from;
    int fromlen = sizeof(from);
    int bytes_recieved = 0;
    

    bytes_recieved = recvfrom(sock, recvbuf, recvbuf_len, 0, (struct sockaddr *)&from, &fromlen);
    if (bytes_recieved == SOCKET_ERROR) {
        return Unexpected(WSA::GetLastError());
    }

    Println("Received {} bytes datagram from {}", bytes_recieved, inet_ntoa(from.sin_addr));

    return RecievedData {
        .from = from,
        .data = std::string(recvbuf, bytes_recieved)
    };
}

Expected<int, int> Socket::SendData(const std::string &data, struct sockaddr_in to) const {
    int bytes_sent = sendto(sock, data.c_str(), static_cast<int>(data.size()), 0, (struct sockaddr *)&to, sizeof(to));
    if (bytes_sent == SOCKET_ERROR) {
        return Unexpected(WSA::GetLastError());
    }

    Println("Bytes sent: {}", bytes_sent);

    return bytes_sent;
}

Socket CreateListenSocket() {
    // Create socket
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    Socket listen_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (!listen_socket.valid()) {
        Println("Could not create socket, error code: {}: {}", WSA::GetLastError(), WSA::GetLastErrorAsString());
        return INVALID_SOCKET;
    }

    // Resolve address
    struct addrinfo *addrinfo = nullptr;
    if (int err = getaddrinfo(NULL, PORT, &hints, &addrinfo); err != 0) {
        Println("getaddrinfo failed with error code: {}", err);
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }

    // Bind socket to address
    if (bind(listen_socket, addrinfo->ai_addr, static_cast<int>(addrinfo->ai_addrlen)) == SOCKET_ERROR) {
        Println("bind failed with error code: {}: {}", WSA::GetLastError(), WSA::GetLastErrorAsString());
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }

    // Don't need this anymore
    freeaddrinfo(addrinfo);

    // You don't actuallty listen on UPD socket

    return listen_socket;
}

Socket CreateConnectSocket() {
    // Create socket
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;


    Socket connect_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

    // You don't really need to resolve address and establish connection with UDP
    // Although you can do that to 'mimic' TCP

    /*
    struct addrinfo *addrinfo = nullptr;
    if (int err = getaddrinfo(NULL, PORT, &hints, &addrinfo); err != 0) {
        Println("getaddrinfo failed with error code: {}", err);
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }
    
    if (connect(connect_socket, addrinfo->ai_addr, static_cast<int>(addrinfo->ai_addrlen)) == SOCKET_ERROR) {
        connect_socket.close();
        return INVALID_SOCKET;
    }

    freeaddrinfo(addrinfo);

    */

    return connect_socket;
}