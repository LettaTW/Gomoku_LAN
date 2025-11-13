#pragma once
#include <vector>
#include <mutex>
#include <nlohmann/json.hpp>

class GameLogic {
public:
	GameLogic();
	void set_server(NetworkServer* s) { server = s; };
	void process_message(int player_id, const std::string &type, const nlohmann::json &data);
private:
	int current_turn_player;
	int board[15][15];
	NetworkServer* server;

	bool check_win(int x, int y, int player);
	void next_turn();

	std::mutex game_mutex;
};