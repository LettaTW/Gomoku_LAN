#pragma once
#include <string>
#include <vector>
#include <winsock2.h> // for htonl
#include "nlohmann\json.hpp"

using json = nlohmann::json;

class Protocol {
public:
	// 封裝訊息：在 JSON 字串前加上 4-byte 長度標頭
    static std::string pack_message(const std::string& json_str);

	// 解析訊息：讀取 4-byte 長度標頭並解析後續的 JSON 字串
    static std::pair<std::string, json> parse(const std::string& json_str);

	// 建立 connect_ok 訊息
    static std::string create_connect_ok(int pid);

	// 建立 game_start 訊息
    static std::string create_game_start(void);

	// 建立 game_update 訊息
    static std::string create_game_update(int board[15][15], int next_turn);

	// 建立 game_over 訊息
    static std::string create_game_over(int winner);
};