import socket
import threading
import struct

class NetworkClient:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.is_connected = False
        self.on_message_received = None # 這是一個回呼函數 (Callback)

    def connect(self, ip, port):
        try:
            print(f"Connecting to {ip}:{port}...")
            self.sock.connect((ip, port))
            self.is_connected = True
            print("Connected!")
            
            # 連線成功後，啟動一個子執行緒專門負責 "聽"
            listen_thread = threading.Thread(target=self._listen_loop, daemon=True)
            listen_thread.start()
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False

    def send_raw(self, data_bytes):
        """發送原始 bytes 數據給伺服器"""
        if not self.is_connected:
            return
        try:
            self.sock.sendall(data_bytes)
        except Exception as e:
            print(f"Send error: {e}")
            self.is_connected = False

    def _listen_loop(self):
        """(內部使用) 持續監聽伺服器傳來的數據"""
        try:
            while self.is_connected:
                # 1. 讀取 4-byte 標頭 (長度)
                header = self.sock.recv(4)
                if not header: break # 伺服器斷線
                
                # 2. 解碼長度 (Big Endian '!')
                msg_len = struct.unpack('!I', header)[0]

                # 3. 讀取剩下的 N bytes (確保讀滿)
                data = b""
                while len(data) < msg_len:
                    packet = self.sock.recv(msg_len - len(data))
                    if not packet: return
                    data += packet
                
                # 4. 收到完整封包後，通知 Protocol 層處理
                if self.on_message_received:
                    self.on_message_received(data)

        except Exception as e:
            print(f"Disconnected: {e}")
        finally:
            self.is_connected = False
            self.sock.close()