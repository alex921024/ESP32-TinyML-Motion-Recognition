#include <Motion-Recognition_inferencing.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

const int MPU_ADDR = 0x68;

const char* ssid = "ESP32_AI_Motion"; 
const char* password = "password123"; 

WebServer server(80);
String currentAction = "idle"; 

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;
unsigned long lastSampleTime = 0; 

// ==========================================
// 網頁前端程式碼 
// ==========================================
const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 AI 動作辨識</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #1e1e1e; color: white; text-align: center; margin-top: 50px; }
    .status-box { background-color: #333; padding: 20px; border-radius: 10px; display: inline-block; min-width: 300px; box-shadow: 0 4px 8px rgba(0,0,0,0.3); }
    h1 { color: #4CAF50; }
    #action-text { font-size: 28px; font-weight: bold; margin: 20px 0; color: #FF9800; }
    
    /* 手部動畫容器 */
    .animation-area { height: 250px; display: flex; justify-content: center; align-items: center; margin-top: 30px; }
    .hand { font-size: 80px; display: inline-block; }

    /* --- 定義三種 CSS 軌跡動畫 --- */
    /* 1. 畫圈 (保持手掌朝上旋轉) */
    @keyframes circle-path {
      0% { transform: rotate(0deg) translateX(60px) rotate(0deg); }
      100% { transform: rotate(360deg) translateX(60px) rotate(-360deg); }
    }
    .anim-circle { animation: circle-path 1.5s linear infinite; }

    /* 2. 畫三角形 */
    @keyframes triangle-path {
      0% { transform: translate(0px, -60px); }
      33% { transform: translate(60px, 60px); }
      66% { transform: translate(-60px, 60px); }
      100% { transform: translate(0px, -60px); }
    }
    .anim-triangle { animation: triangle-path 1.5s linear infinite; }

    /* 3. 靜止 (輕微呼吸浮動) */
    @keyframes idle-path {
      0%, 100% { transform: translateY(0); }
      50% { transform: translateY(-15px); }
    }
    .anim-idle { animation: idle-path 3s ease-in-out infinite; }
  </style>
</head>
<body>
  <div class="status-box">
    <h1>TinyML 離線即時辨識</h1>
    <div id="action-text">狀態：連線成功，等待數據...</div>
  </div>

  <div class="animation-area">
    <div id="hand-icon" class="hand anim-idle">🤚</div>
  </div>

  <script>
    // 每 0.3 秒向 ESP32 詢問一次目前的動作
    setInterval(() => {
      // ★ 防快取解法 1：加上時間戳記，確保每次網址都不同，強迫瀏覽器抓新資料
      fetch('/data?t=' + new Date().getTime())
        .then(response => response.json())
        .then(data => {
          const action = data.action;
          const handEl = document.getElementById('hand-icon');
          const textEl = document.getElementById('action-text');

          // 重置動畫 class
          handEl.className = 'hand'; 

          if (action === 'circle_data') {
            textEl.innerText = "⭕ 正在：畫圈 (Circle)";
            handEl.classList.add('anim-circle');
          } else if (action === 'triangle_data') {
            textEl.innerText = "🔺 正在：畫三角形 (Triangle)";
            handEl.classList.add('anim-triangle');
          } else {
            textEl.innerText = "🛑 正在：靜止 (Idle)";
            handEl.classList.add('anim-idle');
          }
        })
        .catch(err => console.error(err));
    }, 300);
  </script>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); 

  // 喚醒 MPU-6500
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);

  Serial.println("\n正在啟動 ESP32 專屬 Wi-Fi 熱點...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.println("=================================");
  Serial.println("熱點建立成功！");
  Serial.print("1. 請用手機連線至 Wi-Fi： ");
  Serial.println(ssid);
  Serial.print("2. 密碼為： ");
  Serial.println(password);
  Serial.print("3. 打開瀏覽器輸入網址： http://");
  Serial.println(myIP);
  Serial.println("=================================");

  // --- 設定網頁路由 ---
  server.on("/", []() {
    server.send(200, "text/html", html_page);
  });
  
  server.on("/data", []() {
    String json = "{\"action\":\"" + currentAction + "\"}";
    // ★ 防快取解法 2：從伺服器端強制命令手機不准快取
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
    server.send(200, "application/json", json);
  });
  
  server.begin();
}


void loop() {
  // 處理網頁使用者的連線請求
  server.handleClient();

  // --- 非阻塞計時：嚴格維持 50Hz (每 20 毫秒) 讀取一次感測器 ---
  if (millis() - lastSampleTime >= 20) {
    lastSampleTime = millis();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true); 

    if (Wire.available() == 14) {
      int16_t ax = Wire.read() << 8 | Wire.read();
      int16_t ay = Wire.read() << 8 | Wire.read();
      int16_t az = Wire.read() << 8 | Wire.read();
      int16_t tmp = Wire.read() << 8 | Wire.read(); 
      int16_t gx = Wire.read() << 8 | Wire.read();
      int16_t gy = Wire.read() << 8 | Wire.read();
      int16_t gz = Wire.read() << 8 | Wire.read();

      // 將讀取到的資料存進特徵陣列
      features[feature_ix++] = ax;
      features[feature_ix++] = ay;
      features[feature_ix++] = az;
      features[feature_ix++] = gx;
      features[feature_ix++] = gy;
      features[feature_ix++] = gz;

      // 當收集滿一筆 AI 需要的 Frame Size 時，開始推論
      if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        
        signal_t signal;
        numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        
        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

        if (res == EI_IMPULSE_OK) {
          
          Serial.println("\n=== AI 預測結果 ===");
          
          float highest_prob = 0.0;
          String best_label = "idle"; 

          for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            Serial.print("  ");
            Serial.print(result.classification[i].label);
            Serial.print(": ");
            Serial.print(result.classification[i].value * 100, 1);
            Serial.println("%");

            if (result.classification[i].value > highest_prob) {
              highest_prob = result.classification[i].value;
              best_label = result.classification[i].label;
            }
          }
          Serial.println("==================");
          
          if (highest_prob > 0.6) {
             currentAction = best_label;
          }
        }
        feature_ix = 0;
      }
    }
  }
}