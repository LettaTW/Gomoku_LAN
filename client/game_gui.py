import tkinter as tk
from tkinter import messagebox

class GameGUI:
    def __init__(self):
        # 1. 建立主視窗
        self.root = tk.Tk()
        self.root.title("Gomoku Client (Tkinter)")
        self.root.resizable(False, False)

        # 2. 設定參數
        self.GRID_SIZE = 40       # 格子大小
        self.BOARD_DIM = 15       # 15x15
        self.OFFSET = 40          # 邊界留白 (讓棋盤好看一點)
        
        # 計算視窗大小
        window_size = self.GRID_SIZE * (self.BOARD_DIM - 1) + self.OFFSET * 2
        
        # 3. 建立畫布 (Canvas) - 用來畫線和棋子
        self.canvas = tk.Canvas(self.root, width=window_size, height=window_size, bg="#D2B48C") # 木頭色背景
        self.canvas.pack()

        # 4. 建立下方的狀態標籤
        self.status_label = tk.Label(self.root, text="Connecting...", font=("Arial", 14), pady=10)
        self.status_label.pack(side=tk.BOTTOM, fill=tk.X)

        # 5. 綁定滑鼠點擊事件
        self.canvas.bind("<Button-1>", self._on_canvas_click)
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        # 遊戲資料
        self.board = [[0] * self.BOARD_DIM for _ in range(self.BOARD_DIM)]
        self.my_id = 0
        self.current_turn = 0
        self.protocol = None
        self.running = True

        # 先畫一次空的棋盤
        self._draw_board()

    def set_protocol(self, protocol):
        self.protocol = protocol

    # --- 被 Protocol 呼叫的方法 (更新狀態) ---
    
    def set_player_id(self, pid):
        self.my_id = pid
        title = f"Gomoku - Player {pid} "
        if pid == 1: title += "(Black)"
        elif pid == 2: title += "(White)"
        elif pid == 3: title += "(Red)"
        
        self.root.title(title)
        self.status_label.config(text=f"Connected! You are Player {pid}")

    def start_game(self):
        self.status_label.config(text="Game Started!")
        self._draw_board()

    def update_board(self, new_board, next_turn):
        self.board = new_board
        self.current_turn = next_turn
        
        # 更新狀態文字
        if self.current_turn == self.my_id:
            self.status_label.config(text="YOUR TURN! (Click to place)", fg="red")
        else:
            self.status_label.config(text=f"Player {self.current_turn}'s Turn", fg="black")
        
        # 重畫棋盤
        self._draw_board()

    def show_winner(self, winner):
        self._draw_board()
        msg = ""
        if winner == self.my_id:
            msg = "YOU WIN !!!"
            self.status_label.config(text=msg, fg="green")
            messagebox.showinfo("Game Over", "Congratulations! You Won!")
        else:
            msg = f"Player {winner} WINS!"
            self.status_label.config(text=msg, fg="blue")
            messagebox.showinfo("Game Over", f"Player {winner} has won the game.")

    # --- 內部邏輯 ---

    def run(self):
        # Tkinter 的主迴圈
        self.root.mainloop()

    def _on_close(self):
        self.running = False
        self.root.destroy()

    def _on_canvas_click(self, event):
        # 只有輪到自己才能點
        if self.current_turn != self.my_id:
            return

        # 1. 計算點擊的是哪一個交叉點
        #    利用四捨五入 (round) 來判定離哪個點最近
        gx = round((event.x - self.OFFSET) / self.GRID_SIZE)
        gy = round((event.y - self.OFFSET) / self.GRID_SIZE)

        # 2. 邊界檢查
        if 0 <= gx < self.BOARD_DIM and 0 <= gy < self.BOARD_DIM:
            # 3. 檢查該位置是否已經有棋子 (Client 端先簡單擋一下，避免無效封包)
            if self.board[gx][gy] == 0:
                print(f"Clicked at grid ({gx}, {gy})")
                if self.protocol:
                    self.protocol.send_place_move(gx, gy)

    def _draw_board(self):
        # 清空畫布
        self.canvas.delete("all")

        # 1. 畫線 (縱向 & 橫向)
        for i in range(self.BOARD_DIM):
            # 座標計算：Offset + 格數 * 間距
            pos = self.OFFSET + i * self.GRID_SIZE
            start = self.OFFSET
            end = self.OFFSET + (self.BOARD_DIM - 1) * self.GRID_SIZE
            
            # 橫線 (y 改變，x 從頭到尾)
            self.canvas.create_line(start, pos, end, pos)
            # 縱線 (x 改變，y 從頭到尾)
            self.canvas.create_line(pos, start, pos, end)

        # 2. 畫棋子
        radius = 16 # 棋子半徑
        for x in range(self.BOARD_DIM):
            for y in range(self.BOARD_DIM):
                val = self.board[x][y]
                if val != 0:
                    # 計算圓心
                    cx = self.OFFSET + x * self.GRID_SIZE
                    cy = self.OFFSET + y * self.GRID_SIZE
                    
                    color = "black"
                    outline = "black"
                    if val == 2: 
                        color = "white"
                        outline = "black" # 白棋加個黑邊才看得到
                    elif val == 3: 
                        color = "red"
                        outline = "darkred"

                    # 畫圓 (左上角座標, 右下角座標)
                    self.canvas.create_oval(
                        cx - radius, cy - radius,
                        cx + radius, cy + radius,
                        fill=color, outline=outline
                    )

        # (可選) 標記最後一步棋 (如果有傳送 last_move 的話)