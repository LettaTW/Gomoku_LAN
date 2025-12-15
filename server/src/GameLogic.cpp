#include "GameLogic.h"
#include "Protocol.h"
#include "NetworkServer.h"

GameLogic::GameLogic() : current_turn_player(1) {
	// 初始化棋盤
	for (int i = 0; i < 15; ++i) {
		for (int j = 0; j < 15; ++j) {
			board[i][j] = 0; // 0 表示空格
		}
	}
}

// 處理來自玩家的訊息
void GameLogic::process_message(int player_id, const std::string &type, const nlohmann::json &data) {
	std::lock_guard<std::mutex> lock(game_mutex);

	// 選擇對應的訊息類型進行處理
	if (type == "place_move") {
		int x = data["x"];
		int y = data["y"];
		// 驗證資料是否合法
		if (player_id != current_turn_player) {
			std::cout << "Not Player " << player_id << "'s turn" << std::endl;
			return;
		}

		if (x < 0 || x >= 15 || y < 0 || y >= 15 || board[x][y] != 0) {
			return; // 無效移動
		}

		board[x][y] = player_id; // 放置棋子
		std::cout << "Player " << player_id << " moved to (" << x << "," << y << ")" << std::endl;

		// 檢查是否有玩家獲勝
		// 有，廣播 game_over 訊息
		// 沒有，切換到下一位玩家並廣播 game_update 訊息
		if (check_win(x, y, player_id)) {
			std::cout << "Player " << player_id << " wins!" << std::endl;
			std::string game_update_msg = Protocol::create_game_update(board, current_turn_player);
			server->broadcast(Protocol::pack_message(game_update_msg));

			std::string game_over_msg = Protocol::create_game_over(player_id);
			server->broadcast(Protocol::pack_message(game_over_msg));
		}
		else {
			next_turn();
			std::string game_update_msg = Protocol::create_game_update(board, current_turn_player);
			server->broadcast(Protocol::pack_message(game_update_msg));
		}
	}
}

bool GameLogic::check_win(int x, int y, int player) {
	// 檢查玩家是否在 (x, y) 位置下子後獲勝
	// 水平
	int count = 0;
	for (int i = 0; i < 15; i++) {
		if (board[i][y] == player) count++;
		else count = 0;
		if (count == 5) return true;
	}
	// 垂直
	count = 0;
	for (int j = 0; j < 15; j++) {
		if (board[x][j] == player) count++;
		else count = 0;
		if (count == 5) return true;
	}

	// 斜線 (\)
	count = 0;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (board[i][j] == player) {
				int k = 1;
				while (i + k < 15 && j + k < 15 && board[i + k][j + k] == player) {
					k++;
				}
				if (k >= 5) return true;
			}
		}
	}

	// 斜線 (/)
	count = 0;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (board[i][j] == player) {
				int k = 1;
				while (i + k < 15 && j - k >= 0 && board[i + k][j - k] == player) {
					k++;
				}
				if (k >= 5) return true;
			}
		}
	}
	
	return false;
}

void GameLogic::next_turn() {
	// 切換到下一位玩家的回合 (支援 3 位或更多玩家)
	// 這裡假設最多 3 位玩家，可根據需要調整
	current_turn_player = (current_turn_player % 3) + 1;
}

