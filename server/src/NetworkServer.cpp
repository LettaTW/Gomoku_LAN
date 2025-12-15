#include "NetworkServer.h"
#include "GameLogic.h"
#include "Protocol.h"

GameLogic game;

NetworkServer::NetworkServer(int port) : port(port) {

	// 初始化 Winsock
	// WSAStartup 初始化 Winsock DLL
	// MAKEWORD(2,2) 指定使用 Winsock 2.2 版本
	// WSAData 結構接收有關 Windows Sockets 實作的資訊
	// 如果 WSAStartup 成功，之後就必須呼叫 WSACleanup 來釋放資源
	std::cout << "Initializing Winsock" << std::endl;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::runtime_error("WSAStartup failed");
	}

	// 創建伺服器 socket 
	// AF_INET = IPv4
	// SOCK_STREAM = TCP socket
	// IPPROTO_TCP = TCP protocol
	std::cout << "Creating server socket" << std::endl;
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
	std::cout << "Binding server socket to port " << port << std::endl;

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		closesocket(server_socket);
		WSACleanup();
		throw std::runtime_error("Bind failed");
	}
	
	// 設定伺服器的遊戲邏輯指針
	game.set_server(this);
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


// 啟動伺服器並開始接受連線
void NetworkServer::start() {
	// 開始監聽連線
	// listen 使伺服器 socket 進入監聽狀態
	// SOMAXCONN = 系統允許的最大連線數
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		throw std::runtime_error("Listen failed");
	}
	std::cout << "Server started on port " << port << std::endl;
	accept_loop();
}


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
			
			std::cout << "Broadcasting initial game_update message." << std::endl;
			std::string update_msg = Protocol::create_game_update(game.board, 1);  // 玩家 1 先手
			std::string packed_update = Protocol::pack_message(update_msg);
			broadcast(packed_update);
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
		if (send(client_socket, packed_msg.c_str(), (int)packed_msg.size(), 0) == SOCKET_ERROR) {
			throw std::runtime_error("Send 'connect_ok' failed.");
		}

		// 接收客戶端的訊息
		char buffer[4096];
		int bytes_received = 0;
		std::string remaining_data = "";

		while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
			remaining_data.append(buffer, bytes_received);

			// 處理緩衝區內的資料
			while (remaining_data.size() >= 4) {

				// 1. 先「偷看」前 4 個 bytes 來取得訊息長度
				//    (這裡還不能 erase，因為如果資料不夠，我們還需要保留這 4 bytes)
				//    使用 const uint32_t* 解決編譯錯誤
				uint32_t msg_len = ntohl(*reinterpret_cast<const uint32_t*>(remaining_data.data()));

				// 2. 【關鍵】檢查資料是否完整
				//    我們需要： 4 byte 標頭 + msg_len byte 內容
				if (remaining_data.size() < 4 + msg_len) {
					// 資料還不夠 (例如只收到了一半的 JSON)
					// 跳出內部迴圈，繼續去外層 recv 更多資料
					break;
				}

				// 3. 資料完整了！取出 JSON 本體
				//    substr(開始位置, 長度) -> 跳過前 4 bytes，取 msg_len 長度
				std::string json_str = remaining_data.substr(4, msg_len);

				// 4. 解析 JSON 並交給遊戲邏輯
				try {
					// Protocol::parse 只需要純 JSON 字串
					auto [type, data] = Protocol::parse(json_str);
					game.process_message(player_id, type, data);
				}
				catch (const std::exception& e) {
					std::cerr << "JSON Parsing Error from Player " << player_id << ": " << e.what() << std::endl;
					// 即使解析失敗，也要移除這段有問題的資料，避免無窮迴圈
				}

				// 5. 從緩衝區移除這則已處理完畢的訊息 (標頭 4 + 內容 msg_len)
				remaining_data.erase(0, 4 + msg_len);
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error handling client " << player_id << ": " << e.what() << std::endl;
	}

	closesocket(client_socket);

	{
		std::lock_guard<std::mutex> lock(clients_mutex);
		for (auto it = client_sockets.begin(); it != client_sockets.end(); ++it) {
			if (*it == client_socket) {
				client_sockets.erase(it);
				std::cout << "Player " << player_id << " is removed from active client list." << std::endl;
				break;
			}
		}
	}
}


void NetworkServer::broadcast(const std::string& message) {
	std::lock_guard<std::mutex> lock(clients_mutex);
	for (SOCKET s : client_sockets) {
		send(s, message.c_str(), message.length(), 0);
	}
}