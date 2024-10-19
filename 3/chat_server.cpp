#include <string>
#include <print>
#include <list>
#include <thread>
#include <csignal>

#include "wsa.hpp"
#include "thread_pool.hpp"

ThreadPool pool(100); // global so it cleans up on SIGINT

std::vector<Socket> client_sockets;
std::mutex client_sockets_mutex;

std::size_t GetClientsCount() {
    std::lock_guard<std::mutex> lock(client_sockets_mutex);

    return client_sockets.size();
}

const Socket &GetClientSocket(std::size_t idx) {
    std::lock_guard<std::mutex> lock(client_sockets_mutex);

    return client_sockets[idx];
}

std::string SocketsAsString() {
    std::string res = "{";
    for (std::size_t i = 0; i < GetClientsCount(); ++i) {
        res += std::to_string(static_cast<std::int64_t>(GetClientSocket(i)));
        if (i != client_sockets.size() - 1) {
            res += ", ";
        }
    }
    res += "}";
    return res;
}

// returns index of added socket
std::size_t AddSocket(Socket socket) {
    std::lock_guard<std::mutex> lock(client_sockets_mutex);

    client_sockets.push_back(std::move(socket));
    return client_sockets.size() - 1;
}

Socket PopSocket(std::size_t idx) {
    std::lock_guard<std::mutex> lock(client_sockets_mutex);

    std::swap(client_sockets[idx], client_sockets.back());
    Socket popped = std::move(client_sockets.back());
    client_sockets.pop_back();
    return popped;
}

void SendEveryoneExcept(SOCKET sender, const std::string &message) {
    for (std::size_t i = 0; i < GetClientsCount(); ++i) {
        const Socket &reciever = GetClientSocket(i);

        if (reciever == sender) {
            continue;
        }

        auto send_res = SendData(reciever, message);
        if (!send_res && send_res.error() != WSAEWOULDBLOCK) {
            std::println("Failed to send message to socket {}, error code: {}, {}", static_cast<std::int64_t>(SOCKET(reciever)), wsa.GetLastError(), wsa.GetLastErrorAsString());
            Socket popped = PopSocket(i);
            std::println("Popped socket {}", static_cast<std::int64_t>(SOCKET(popped)));
        } else if (send_res) {
            std::println("Sent {} bytes", send_res.value());
        } else {} // WSAEWOULDBLOCK here
    }
}

std::atomic<bool> g_managing_sockets = true;

void ManageSockets() {
    while (g_managing_sockets.load(std::memory_order_relaxed)) {
        for (std::size_t i = 0; i < GetClientsCount(); ++i) {
            const Socket &socket = GetClientSocket(i);

            auto recv_data = RecieveData(socket);

            if (recv_data) {
                pool.NewTask(SendEveryoneExcept, SOCKET(socket), std::move(recv_data.value()));
                // SendEveryoneExcept(socket, recv_data.value());
            } else if (recv_data.error() != WSAEWOULDBLOCK) {
                std::println("Failed to recieve from socket {}, error code: {}: {}", static_cast<std::int64_t>(socket), wsa.GetLastError(), wsa.GetLastErrorAsString());
                Socket popped = PopSocket(i);
                std::println("Popped socket {}", static_cast<std::int64_t>(SOCKET(popped)));
            }
        }
    }
}

int main() {
    std::signal(SIGINT, [](int) {
        // basically cleans up WSA and ThreadPool (what if ~WSA() before ~ThreadPool() ???)
        // not the best way to do it
        // but it will at least make some info printed to stdout
        g_managing_sockets.store(false, std::memory_order_relaxed);
        std::exit(0);
    });

    pool.SetLoggingFlag(false);

    Socket listen_socket = CreateListenSocket();
    if (listen_socket == INVALID_SOCKET) {
        std::println("Could not create listen socket, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    if (int err = listen_socket.SetBlocking(false); err != 0) {
        std::println("ioctlsocket failed, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
        return 1;
    }

    std::println(" ----- SERVER ----- ");
    std::println();

    pool.NewTask(ManageSockets);
    std::println("Waiting for connections...");
    while (true) {
        Socket client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            if (wsa.GetLastError() != WSAEWOULDBLOCK) {
                std::println("accept returned INVALID_SOCKET, error code: {}: {}", wsa.GetLastError(), wsa.GetLastErrorAsString());
            }
            continue;
        }

        std::println("Connected socket {}", SOCKET(client_socket));

        AddSocket(std::move(client_socket));
        std::println("All connected sockets: {}", SocketsAsString());
    }       

    return 0;
}