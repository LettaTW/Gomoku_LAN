#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <iostream>

// Windows Socket API
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class NetworkServer {
public:
    NetworkServer(int port);
    ~NetworkServer();
    void start();
    void broadcast(const std::string& message);

private:
    void accept_loop();
    void handle_client(SOCKET client_socket, int player_id);

    SOCKET server_socket;
    int port;
    std::vector<SOCKET> client_sockets;
    std::mutex clients_mutex;
};