#include <print>

#include "wsa.hpp"

WSA wsa{};

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
    return ErrorCodeToString(wsa.GetLastError());
}

std::expected<std::string, int> RecieveData(const Socket &socket) {
    const int recvbuf_len = 1024;
    char recvbuf[recvbuf_len] = {0};
    
    int bytes_recieved = 0;
    std::string recv_data;
    do {
        bytes_recieved = recv(socket, recvbuf, recvbuf_len, 0);

        if (bytes_recieved > 0) {
            std::println("Recieved {} bytes", bytes_recieved);
            recv_data.append(recvbuf, recvbuf + bytes_recieved);
        } else if (bytes_recieved == 0) {
            std::println("Recieve success");
        } else {
            if (wsa.GetLastError() == WSAEWOULDBLOCK && !recv_data.empty()) {
                break;
            }
            
            return std::unexpected(wsa.GetLastError());
        }
    } while (bytes_recieved > 0);

    return std::move(recv_data);
}

std::expected<int, int> SendData(const Socket &socket, const std::string &data) {
    int bytes_sent = send(socket, data.c_str(), static_cast<int>(data.size()), 0); 
    if (bytes_sent == SOCKET_ERROR) {
        return std::unexpected(wsa.GetLastError());
    }

    std::println("Bytes sent: {}", bytes_sent);

    return bytes_sent;
}

Socket CreateListenSocket() {
    // Create socket
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    Socket listen_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        std::println("Could not create socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return INVALID_SOCKET;
    }

    // Resolve address
    struct addrinfo *addrinfo = nullptr;
    if (int err = getaddrinfo(NULL, PORT, &hints, &addrinfo); err != 0) {
        std::println("getaddrinfo failed with error code: {}", err);
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }

    // Bind socket to address
    if (bind(listen_socket, addrinfo->ai_addr, static_cast<int>(addrinfo->ai_addrlen)) == SOCKET_ERROR) {
        std::println("bind failed with error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }

    // Don't need this anymore
    freeaddrinfo(addrinfo);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::println("listen failed with error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return INVALID_SOCKET;
    }

    return listen_socket;
}

Socket CreateConnectSocket() {
    // Create socket
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve address
    struct addrinfo *addrinfo = nullptr;
    if (int err = getaddrinfo(NULL, PORT, &hints, &addrinfo); err != 0) {
        std::println("getaddrinfo failed with error code: {}", err);
        freeaddrinfo(addrinfo);
        return INVALID_SOCKET;
    }

    Socket connect_socket;
    // Try different connection
    for (struct addrinfo *p = addrinfo; p != nullptr; p = p->ai_next) {
        // Create socket
        connect_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (connect_socket == INVALID_SOCKET) {
            freeaddrinfo(addrinfo);
            return INVALID_SOCKET;
        }

        // Try to connect
        if (connect(connect_socket, p->ai_addr, static_cast<int>(p->ai_addrlen)) == SOCKET_ERROR) {
            connect_socket.close();
            connect_socket = INVALID_SOCKET;
            continue;
        }

        // Connection succeeded
        break;
    }

    freeaddrinfo(addrinfo);

    return connect_socket;
}