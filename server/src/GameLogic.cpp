#include "GameLogic.h"
#include "Protocol.h"
#include "NetworkServer.h"

GameLogic::GameLogic() : current_turn_player(1) {
	// TODO (邏輯)
	// 初始化棋盤
}

// 處理來自玩家的訊息
void GameLogic::process_message(int player_id, const std::string &type, const nlohmann::json &data) {
	std::lock_guard<std::mutex> lock(game_mutex);

	// 選擇對應的訊息類型進行處理
	if (type == "place_move") {
		int x = data["x"];
		int y = data["y"];
		// TODO (邏輯)
		// 驗證資料是否合法

		// TODO (邏輯)
		// 檢查是否有玩家獲勝
		// 有，廣播 game_over 訊息
		// 沒有，切換到下一位玩家並廣播 game_update 訊息

	}
}

bool GameLogic::check_win(int x, int y, int player) {
	// TODO (邏輯)
	// 檢查玩家是否在 (x, y) 位置下子後獲勝

	return false;
}

void GameLogic::next_turn() {
	// TODO (邏輯)
	// 切換到下一位玩家的回合
}

