#include <Motion-Recognition_inferencing.h>
#include <Wire.h>

const int MPU_ADDR = 0x68;

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0; // 陣列的索引計數器

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); 

  // 喚醒 MPU-6500
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);
  
  // 等待序列埠穩定
  delay(2000); 
  Serial.println("AI 模型載入成功");
  Serial.println("請開始做動作...");
}

void loop() {
  // --- 1. 讀取感測器數據 ---
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

    // --- 2. 將數據依序塞入特徵陣列  ---
    features[feature_ix++] = ax;
    features[feature_ix++] = ay;
    features[feature_ix++] = az;
    features[feature_ix++] = gx;
    features[feature_ix++] = gy;
    features[feature_ix++] = gz;

    // --- 3. 如果陣列塞滿了 (代表收集完一個完整動作的時間了) ---
    if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      
      // 建立信號物件
      signal_t signal;
      int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
      if (err != 0) {
        Serial.println("無法建立信號緩衝區");
        return;
      }

      // 執行 Edge Impulse 推論大腦！
      ei_impulse_result_t result = { 0 };
      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

      // --- 4. 印出辨識結果 ---
      if (res == EI_IMPULSE_OK) {
        Serial.println("\n=========================================");
        Serial.print("推論耗時: "); 
        Serial.print(result.timing.classification); 
        Serial.println(" 毫秒");
        
        // 跑迴圈印出每一個動作的「機率」(0.0000 ~ 1.0000)
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
          Serial.print("  ");
          Serial.print(result.classification[i].label);
          Serial.print(": ");
          // 將機率換算成百分比方便觀看
          Serial.print(result.classification[i].value * 100, 1); 
          Serial.println(" %");
        }
      }
      
      // 辨識完畢，把計數器歸零，準備錄製下一次的動作
      feature_ix = 0;
    }
  }

  // 延遲 20 毫秒，嚴格維持與訓練時相同的 50Hz 取樣頻率！
  delay(20); 
}