# ESP32 TinyML: Motion Recognition (微型機器學習動作辨識) 🚀

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue)
![MPU6500](https://img.shields.io/badge/Sensor-MPU6500-orange)
![TinyML](https://img.shields.io/badge/AI-Edge_Impulse-lightgreen)
![Python](https://img.shields.io/badge/Script-Python_3-yellow)
![Web](https://img.shields.io/badge/UI-HTML/CSS/JS-red)

這是一個基於 **ESP32** 與 **MPU-6500 (六軸感測器)** 的 TinyML (微型機器學習) 專案。
本專案示範了從「原始數據收集」、「Edge Impulse 雲端模型訓練」，到最終「ESP32 離線邊緣運算部署」的完整流程。

最終成果：ESP32 將化身為一個獨立的 Wi-Fi 基地台 (AP 模式)，並架設 Web Server。使用者連線後，即可在手機網頁上看到與手部動作即時同步的 CSS 動畫。

目前支援辨識的動作：
* ⭕ 畫圈 (`circle`)
* 🔺 畫三角形 (`triangle`)
* 🛑 靜止 (`idle`)

---

## 📁 專案資料夾結構

```text
ESP32-TinyML-Motion-Recognition/
│
├── collect data/                  # 階段一：存放資料收集專用的 Arduino 程式碼 (Data Forwarder)
├── data/                          # 階段一：存放錄製完成的特徵資料集 (CSV 格式)
├── main script/                   # 階段三：存放最終結合 AI 模型與 Web Server 的 ESP32 主程式
├── model/                         # 階段二：存放從 Edge Impulse 下載的 Arduino AI 模型程式庫 (.zip 或解壓縮檔)
│
├── collect_data.py                # 階段一：自動擷取序列埠數據並存成 CSV 的 Python 腳本
├── .gitattributes                 # Git 屬性設定檔
├── LICENSE                        # 專案開源授權條款
└── README.md                      # 專案說明文件
## 🛠️ 硬體接線 (Hardware Wiring)

採用 I2C 通訊協定連接 ESP32 與 MPU-6500：

| MPU-6500 Pin | ESP32 Pin | 說明 |
| :---: | :---: | :--- |
| **VCC** | 3.3V | ⚠ 嚴禁接 5V 以免燒毀感測器 |
| **GND** | GND | 共地 |
| **SDA** | GPIO 21 | I2C 資料線 |
| **SCL** | GPIO 22 | I2C 時脈線 |

-----

## 🚀 階段一：詳細數據收集流程 (Data Collection)

> ⚠️ **重要觀念：** ESP32 無法同時「收集訓練用原始數據」又「執行 AI 推論」。因此在收集數據時，必須燒錄專用的 Data Forwarder 程式碼。

### 1\. 燒錄數據輸出程式

將 `data_collection/esp32_data_forwarder/esp32_data_forwarder.ino` 燒錄至 ESP32。
這支程式唯一的工作，就是以 **50Hz (每 20ms 一次)** 的頻率，將感測器的六軸數據轉換成 `ax,ay,az,gx,gy,gz` 的純文字逗號分隔格式 (CSV) 並透過 USB 傳輸。

### 2\. 關閉佔用通訊埠的軟體

燒錄完成後，**請務必關閉 Arduino IDE 內建的「序列埠監控視窗」與「序列埠繪圖家」**。若不關閉，接下來的 Python 腳本會報錯（`COM Port is busy or doesn't exist`）。

### 3\. 執行 Python 錄製腳本

開啟終端機 (Terminal / PowerShell)，執行專案中的 Python 腳本：

```bash
python collect_data.py
```

  * **設定參數：** 在腳本中，確認 `COM_PORT` 設定為你的 ESP32 埠號 (如 `COM3`)，`BAUD_RATE` 為 `115200`。
  * **開始錄製：** 程式執行後，拿起感測器開始重複做同一個動作（例如不斷畫圈）。
  * **結束錄製：** 收集到足夠數據後（建議每種動作至少收集 2\~3 分鐘），按下 `Ctrl+C` 停止錄製。資料將自動儲存為 `dataset/` 目錄下的 `.csv` 檔案。

-----

## 🧠 階段二：Edge Impulse 模型訓練

1.  前往 [Edge Impulse Studio](https://studio.edgeimpulse.com/) 建立新專案。
2.  **Data Acquisition:** 透過 CSV Wizard 將剛才錄製好的 `.csv` 檔案上傳，並標註對應的 Label (`circle`, `triangle`, `idle`)。
3.  **Impulse Design:**
      * Window size: 設定為 `1500 ms` (視動作長度而定)。
      * Processing block: 選擇 **Spectral Analysis** 提取頻域與時域特徵。
      * Learning block: 選擇 **Classification (Keras)**。
4.  **模型訓練:** 開始訓練神經網路。建議使用 **Unoptimized (float32)** 版本以保留最高特徵精度，避免 int8 量化導致特徵失真。*(本專案 Float32 模型實測準確率可達 99.1%)*。
5.  **部署匯出:** 至 Deployment 頁面，選擇 **Arduino Library** 匯出 `.zip` 檔，並將其匯入你的 Arduino IDE 中。

-----

## 🌐 階段三：邊緣部署與 Web 互動展示

將含有 AI 模型與 Web Server 功能的最終程式碼部署回硬體上。

### 執行步驟

1.  確認你已經將 Edge Impulse 匯出的 `.zip` 程式庫匯入 Arduino IDE。
2.  開啟 `deployment/esp32_offline_inference/esp32_offline_inference.ino`。
3.  確保程式碼上方有正確引入你的專案標頭檔：`#include <你的專案名稱_inferencing.h>`。
4.  燒錄至 ESP32。燒錄完成後，**按一下 ESP32 上的 `EN` / `RST` 按鈕重新啟動**。

### 觀看成果

1.  打開手機或電腦的 Wi-Fi 設定。
2.  尋找並連線至 ESP32 發射的獨立熱點：
      * **SSID:** `ESP32_AI_Motion`
      * **Password:** `password123`
3.  （重要）為了避免手機自動切換回行動網路，**建議暫時關閉手機的 4G/5G 行動數據**。
4.  打開瀏覽器，輸入網址：`http://192.168.4.1`
5.  開始揮動感測器，享受即時連動的 AI 手勢辨識動畫！

-----

## 👤 作者 (Author)

**Alex**

  * GitHub: [@alex921024](https://www.google.com/search?q=https://github.com/alex921024)
