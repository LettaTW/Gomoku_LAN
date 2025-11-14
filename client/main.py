from network_client import NetworkClient
from protocol import Protocol
from game_gui import GameGUI

# 伺服器設定
SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080

def main():
    # 1. 建立各個模組
    network = NetworkClient()
    gui = GameGUI()
    protocol = Protocol(network, gui)

    # 2. 互相綁定
    # (GUI 需要呼叫 protocol 來發送下棋動作)
    gui.set_protocol(protocol)

    # 3. 連線到伺服器
    if network.connect(SERVER_IP, SERVER_PORT):
        # 4. 啟動遊戲主迴圈 (這會卡住，直到視窗關閉)
        gui.run()
    else:
        print("無法連線到伺服器，程式結束。")

if __name__ == "__main__":
    main()