import json
import struct

class Protocol:
    def __init__(self, network, gui):
        self.network = network
        self.gui = gui
        
        # 設定網路收到訊息時，要呼叫我的哪個函數
        self.network.on_message_received = self.handle_server_message

    # --- 發送端 (GUI -> Server) ---
    
    def send_place_move(self, x, y):
        """將下棋動作打包成 JSON 並發送"""
        msg = {
            "type": "place_move",
            "x": x,
            "y": y
        }
        self._pack_and_send(msg)

    def _pack_and_send(self, msg_dict):
        """將 dict 轉為 JSON -> 加上長度標頭 -> 發送"""
        json_str = json.dumps(msg_dict)
        data_bytes = json_str.encode('utf-8')
        
        # 加上 4-byte 標頭 (Big Endian)
        header = struct.pack('!I', len(data_bytes))
        
        # 呼叫網路層發送
        self.network.send_raw(header + data_bytes)

    # --- 接收端 (Server -> GUI) ---

    def handle_server_message(self, data_bytes):
        """解析 Server 來的 JSON，並更新 GUI"""
        try:
            json_str = data_bytes.decode('utf-8')
            msg = json.loads(json_str)
            msg_type = msg.get("type")

            print(f"[Protocol] Received: {msg_type}") # 除錯用

            if msg_type == "connect_ok":
                pid = msg["player_id"]
                self.gui.set_player_id(pid)
            
            elif msg_type == "game_start":
                # 如果有支援動態棋盤大小，可以在這裡讀取 msg["board_size"]
                self.gui.start_game()
            
            elif msg_type == "game_update":
                board = msg["board"]
                next_turn = msg["next_turn"]
                self.gui.update_board(board, next_turn)
            
            elif msg_type == "game_over":
                winner = msg["winner"]
                self.gui.show_winner(winner)

        except Exception as e:
            print(f"Protocol error: {e}")