# ESP32 TinyML: Motion Recognition (微型機器學習動作辨識) 🚀

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue)
![MPU6500](https://img.shields.io/badge/Sensor-MPU6500-orange)
![TinyML](https://img.shields.io/badge/AI-Edge_Impulse-lightgreen)
![Python](https://img.shields.io/badge/Script-Python_3-yellow)

這是一個基於 **ESP32-WROOM-32** 與 **MPU-6500 (六軸感測器)** 的 TinyML (微型機器學習) 專案。
透過收集加速度計與陀螺儀的連續時間序列數據，並使用 **Edge Impulse** 進行頻譜分析與神經網路訓練，最終將模型 (Float32, 99.1% 準確率) 部署回 ESP32，實現**完全離線、低延遲的即時動作辨識**。

目前支援辨識的動作類別包含：
* ⭕ 畫圈 (`circle`)
* 🔺 畫三角形 (`triangle`)
* 🛑 靜止/雜訊 (`idle`)

---

## 🛠️ 硬體接線 (Hardware Wiring)

採用 I2C 通訊協定連接 ESP32 與 MPU-6500：

| MPU-6500 Pin | ESP32 Pin | 說明 |
| :---: | :---: | :--- |
| **VCC** | 3.3V | ⚠ 嚴禁接 5V 以免燒毀感測器 |
| **GND** | GND | 共地 |
| **SDA** | GPIO 21 | I2C 資料線 |
| **SCL** | GPIO 22 | I2C 時脈線 |

---

## 🚀 專案實作步驟 (How it works)

### Step 1: 數據收集 (Data Collection)
1. 將 `data_collection/esp32_data_forwarder` 燒錄至 ESP32，設定採樣率為 50Hz，輸出 `ax,ay,az,gx,gy,gz` 的 CSV 格式。
2. 關閉 Arduino 監控視窗，執行 `data_collection/collect_data.py` (需安裝 `pyserial`)。
3. 腳本會自動擷取 COM Port 數據並存成不同動作的 CSV 標籤檔。

### Step 2: 模型訓練 (Edge Impulse Studio)
1. 將 CSV 資料集上傳至 [Edge Impulse](https://studio.edgeimpulse.com/)。
2. 建立 Time-series 模型，設定 Window Size。
3. 使用 **Spectral Analysis** 提取頻域特徵 (Feature Extraction)。
4. 訓練 Neural Network (Keras) 分類器，並選擇 **Unoptimized (float32)** 版本以保留最高特徵精度，避免 int8 量化失真。
*(實測準確率可達 99% 以上，且模型大小僅需 17.7K Flash / 1.6K RAM)*

### Step 3: 邊緣部署 (Offline Deployment)
1. 從 Edge Impulse 將模型匯出為 **Arduino Library** (`.zip`)。
2. 將函式庫匯入 Arduino IDE。
3. 燒錄 `deployment/esp32_offline_inference` 程式碼至 ESP32。
4. 打開序列埠監控視窗 (115200 baud)，即可開始離線即時推論！

---

## 📊 成果展示 (Demo)

> **📝 建議：** 您可以在這裡放上一張 Edge Impulse 的 `Feature Explorer` 3D 特徵分布截圖，或是實際在監控視窗印出 99% 準確率的畫面截圖。

![在此替換為你的特徵分布圖](url_to_your_image)

---

## 👤 作者 (Author)

**Alex**
* GitHub: [@alex921024](https://github.com/alex921024)

歡迎對 IoT 邊緣運算或 TinyML 有興趣的開發者交流討論！如果這個專案對你有幫助，請給我一個 ⭐ Star！