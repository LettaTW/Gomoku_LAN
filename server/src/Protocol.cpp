#include "Protocol.h"



// TODO (資料) :
// 封裝訊息：在 JSON 字串前加上 4-byte 長度標頭
std::string Protocol::pack_message(const std::string& json_str) {
    uint32_t len = json_str.length();
    uint32_t net_len = htonl(len);
    std::string header(4, 0);
    memcpy(&header[0], &net_len, 4);
    return header + json_str;
}

// TODO (資料) :
// 解析訊息：讀取 4-byte 長度標頭並解析後續的 JSON 字串
std::pair<std::string, json> Protocol::parse(const std::string& json_str) {

}

// TODO (資料) :
// 建立 connect_ok 訊息
// 包含玩家 ID (pid)
std::string Protocol::create_connect_ok(int pid) {
    
}

// TODO (資料) :
// 建立 game_start 訊息
std::string Protocol::create_game_start(void) {

}

// TODO (資料) :
// 建立 game_update 訊息
// 包含棋盤狀態(board)與下一回合玩家 ID (next_turn)
// 棋盤為 15x15 的二維陣列，0 表示空格，1 表示玩家 1 的棋子，2 表示玩家 2 的棋子
std::string Protocol::create_game_update(int board[15][15], int next_turn) {

}

// TODO (資料) :
// 建立 game_over 訊息
// 包含獲勝玩家 ID (winner)
// 如果是平手則 winner 為 0 
std::string Protocol::create_game_over(int winner) {

}