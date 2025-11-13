#include "NetworkServer.h"
#include "GameLogic.h"
#include "Protocol.h"

NetworkServer::NetworkServer(int port) : port(port) {

	// 初始化 Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::runtime_error("WSAStartup failed");
	}

	// 創建伺服器 socket
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET) {
		WSACleanup();
		throw std::runtime_error("Socket creation failed");
	}

	// 綁定伺服器 socket 到指定的 port
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		closesocket(server_socket);
		WSACleanup();
		throw std::runtime_error("Bind failed");
	}
}

NetworkServer::~NetworkServer() {
	// 關閉所有客戶端 socket 避免有新連線同時進行
	{
		std::lock_guard<std::mutex> lock(clients_mutex);
		for (SOCKET client_socket : client_sockets) {
			closesocket(client_socket);
		}
	}
	
	// 關閉伺服器 socket
	closesocket(server_socket);
	// 清理 Winsock
	WSACleanup();

	std::cout << "Server shutdown complete." << std::endl;
}

// TODO 
// 啟動伺服器並開始接受連線
void NetworkServer::start() {


}

// TODO 
// 管理接受連線與處理多個客戶端
void NetworkServer::accept_loop() {


}

// TODO 
// 處理單一客戶端
void NetworkServer::handle_client(SOCKET client_socket, int player_id) {


}

// TODO 
// 廣播訊息給所有已連線的客戶端
void NetworkServer::broadcast(const std::string& message) {
	std::lock_guard<std::mutex> lock(clients_mutex);

}