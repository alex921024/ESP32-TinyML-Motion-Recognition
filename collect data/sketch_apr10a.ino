#include <Wire.h>
const int MPU_ADDR = 0x68;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); 
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);
}

void loop() {
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

    // 嚴格按照 CSV 格式輸出，中間用逗號隔開 (這樣 Python 才會收)
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.println(gz);
  }
  // 維持 50Hz 採樣率
  delay(20); 
}