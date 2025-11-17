import socket

SERVER_IP = "192.168.0.69" 
SERVER_PORT = 8686       


print(f"INFO: 正在建立 socket...")
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print(f"INFO: Socket 建立成功。")
except Exception as e:
    print(f"ERROR: Socket 建立失敗: {e}")
    exit() 

print(f"INFO: 正在嘗試連線到 {SERVER_IP}:{SERVER_PORT} ...")
try:
    s.connect((SERVER_IP, SERVER_PORT))
    print("連線成功！")
    

except ConnectionRefusedError:
    print("連線失敗")
    print("連線被拒絕。")
except Exception as e:
    print(f"\n[ERROR] 發生未知錯誤: {e}")

finally:
    print("INFO: 關閉 socket。")
    s.close()