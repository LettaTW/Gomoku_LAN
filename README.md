# Gomoku_LAN

# 多人五子棋 (Gomoku LAN Project)

這是一個基於 C++/Python 的區域網路多人五子棋遊戲。

## 技術棧
* **伺服器 (Server):** C++
* **客戶端 (Client):** Python
* **GUI:** Pygame
* **通訊:** TCP Sockets
* **數據格式:** JSON (帶有 4-byte 長度前綴)

## 團隊分工
* **成員1 (邏輯):** `server/src/GameLogic.*`
* **成員2 (資料):** `server/src/Protocol.*`, `client/protocol.py`, `server/lib/json.hpp`
* **成員3 (GUI):** `client/game_gui.py`
* **您 (網路):** `server/src/NetworkServer.*`, `client/network_client.py`
