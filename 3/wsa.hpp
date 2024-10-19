#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>
#include <expected>

#pragma comment (lib, "Ws2_32.lib")

#define PORT "8000"

struct WSA {
    WSADATA wsadata;

    WSA() {
        if (int err = WSAStartup(MAKEWORD(2, 2), &wsadata); err != 0) {
            std::println("WSAStartup failed with error code: {}", err);
            std::println("Terminating...");
            std::exit(1);
        }
        std::println("WSAStartup successful");
    };
    ~WSA() {
        if (int err = WSACleanup(); err == 0) {
            std::println("WSACleanup successful");
        } else {
            std::println("WSACleanup returned non-zero: {}", err);
        }
        
    };

    int GetLastError();
    std::string ErrorCodeToString(int error_code);
    std::string GetLastErrorAsString();
};

struct Socket {
    Socket() : Socket(INVALID_SOCKET) {}
    Socket(SOCKET s) : sock(s) {
        if (s != INVALID_SOCKET) {
            std::println("Socket ({}) created", static_cast<std::int64_t>(sock));
        }
    }

    Socket(const Socket &) = delete;
    Socket(Socket &&other) noexcept {
        sock = other.sock;
        other.sock = INVALID_SOCKET;
    }

    Socket &operator =(const Socket &) = delete;
    Socket &operator =(Socket &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        close();

        sock = other.sock;
        other.sock = INVALID_SOCKET;

        return *this;
    }

    // bool operator ==(const Socket &other) const = default;

    ~Socket() {
        close();
    }

    int SetBlocking(bool value) {
        u_long mode = value ? 0 : 1;
        b_is_blocking = value;
        return ioctlsocket(sock, FIONBIO, &mode);
    }

    bool IsBlocking() const {
        return b_is_blocking;
    }

    int close() {
        int ret_val = closesocket(sock);
        if (sock != INVALID_SOCKET) {
            std::println("Socket ({}) closed", sock);
        }
        sock = INVALID_SOCKET;
        return ret_val;
    }

    operator SOCKET() const {
        return sock;
    }

private:
    SOCKET sock;
    bool b_is_blocking = true;
};

extern WSA wsa;

std::expected<std::string, int> RecieveData(const Socket &socket);
std::expected<int, int> SendData(const Socket &socket, const std::string &data);

Socket CreateListenSocket();
Socket CreateConnectSocket();