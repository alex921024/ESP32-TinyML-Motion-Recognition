import serial
import time
import csv

# 設定你的 COM Port 
COM_PORT = 'COM20'
BAUD_RATE = 115200

# 每次錄製前，修改這裡的動作名稱 
ACTION_NAME = "idle" #triangle, circle, idle

print(f"即將錄製動作：【{ACTION_NAME}】")
print(f"請確認已經關閉 Arduino IDE 的「序列埠繪圖家」或「監控視窗」！")
print("3秒後開始...")
time.sleep(3)

try:
    # 連接 ESP32
    ser = serial.Serial(COM_PORT, BAUD_RATE)
    print("連線成功！開始錄製... (請開始做動作，完成後按 Ctrl+C 停止)")
    
    filename = f"{ACTION_NAME}_data.csv"
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['timestamp', 'ax', 'ay', 'az', 'gx', 'gy', 'gz'])
        
        start_time = time.time()
        
        while True:
            if ser.in_waiting > 0:
                # 讀取一行數據並解碼
                line = ser.readline().decode('utf-8').strip()
                
                # 確認這行資料格式正確 
                if line.count(',') == 5:
                    current_time = int((time.time() - start_time) * 1000) # 計算毫秒時間戳
                    data = line.split(',')
                    writer.writerow([current_time] + data)
                    print(f"錄製中... {line}")

except KeyboardInterrupt:
    print(f"\n錄製結束！資料已成功存成 {filename}")
except Exception as e:
    print(f"\n發生錯誤: {e}")
    print("提示：確定 Arduino IDE 裡的監控視窗已經關閉了嗎？")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()