#include "NetworkServer.h"
#include "GameLogic.h"
#include "Protocol.h"

NetworkServer::NetworkServer(int port) : port(port) {

	// 初始化 Winsock
	// WSAStartup 初始化 Winsock DLL
	// MAKEWORD(2,2) 指定使用 Winsock 2.2 版本
	// WSAData 結構接收有關 Windows Sockets 實作的資訊
	// 如果 WSAStartup 成功，之後就必須呼叫 WSACleanup 來釋放資源
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::runtime_error("WSAStartup failed");
	}

	// 創建伺服器 socket 
	// AF_INET = IPv4
	// SOCK_STREAM = TCP socket
	// IPPROTO_TCP = TCP protocol
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET) {
		WSACleanup();
		throw std::runtime_error("Socket creation failed");
	}

	// 綁定伺服器 socket 到指定的 port
	// 
	// IPv4 地址結構 
	// AF_INET = IPv4
	// INADDR_ANY = 接受任何來自網路的連線
	// htons = 將 port 轉換為網路位元組序
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

// TODO (網路):
// 啟動伺服器並開始接受連線
void NetworkServer::start() {
	// 開始監聽連線
	// listen 使伺服器 socket 進入監聽狀態
	// SOMAXCONN = 系統允許的最大連線數
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		throw std::runtime_error("Listen failed");
	}
	std::cout << "Server started on port " << port << std::endl;

	// 開始接受連線
	accept_loop();

}

// TODO (網路):
// 管理接受連線與處理多個客戶端
void NetworkServer::accept_loop() {
	int player_id_count = 1;
	while (true)
	{
		sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		// 接受新的客戶端連線並取得其ip位址
		SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
		// 檢查是否接受成功
		if (client_socket == INVALID_SOCKET) {
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		// 轉換ip位址和port為字串
		// INET_ADDRSTRLEN 定義在 ws2tcpip.h 中，表示 IPv4 位址的最大字串長度
		// inet_ntop 將二進位的 IP 位址轉換為可讀的字串格式
		// ntohs 將網路位元組序的 port 轉換為主機位元組序
		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
		int client_port = ntohs(client_addr.sin_port);

		int current_player_count = 0;
		{
			// 鎖定列表，安全地讀取大小
			std::lock_guard<std::mutex> lock(clients_mutex);
			current_player_count = client_sockets.size();
		}

		if (current_player_count >= 3) {
			std::cout << "[Rejected] Connection from " << client_ip << ":" << client_port << ". Server is full." << std::endl;
			closesocket(client_socket);
			continue;
		}

		std::cout << "[Accepted] Connection from " << client_ip << ":" << client_port 
				  << " | Assigned Player ID:" << player_id_count << std::endl;


		// 將確定新的客戶端 socket 加入列表
		{
			// 確保只有一個Thread可以同時修改client_sockets
			std::lock_guard<std::mutex> lock(clients_mutex);
			client_sockets.push_back(client_socket);
		}

		
		// 為每個客戶端創建一個新的Thread來處理
		// 傳入客戶端 socket 和玩家ID 以及處理單一客戶端函式
		std::thread client_thread(&NetworkServer::handle_client, this, client_socket, player_id_count);
		client_thread.detach(); // 分離Thread，使其在完成後自動清理資源
		if (player_id_count == 3) {
			std::cout << "3 players connected. Starting game..." << std::endl;
			std::cout << "Broadcasting game_start message to all clients." << std::endl;
			std::string start_msg = Protocol::create_game_start();
			std::string packed_msg = Protocol::pack_message(start_msg);
			broadcast(packed_msg);
		}
		player_id_count++;
	}
}

// TODO (網路):
// 處理單一客戶端
void NetworkServer::handle_client(SOCKET client_socket, int player_id) {
	try {
		std::string connect_ok_msg = Protocol::create_connect_ok(player_id);
		std::string packed_msg = Protocol::pack_message(connect_ok_msg);
		send(client_socket, packed_msg.c_str(), packed_msg.size(), 0);
	}
	catch (const std::exception& e) {
		std::cerr << "Error sending welcome msg to client " << player_id << ": " << e.what() << std::endl;
	}
}

// TODO 
// 廣播訊息給所有已連線的客戶端
void NetworkServer::broadcast(const std::string& message) {
	std::lock_guard<std::mutex> lock(clients_mutex);

}