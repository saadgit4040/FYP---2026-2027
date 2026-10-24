// MPU6050 test on ESP32 (no library needed)
// Wiring: VCC -> 3.3V, GND -> GND, SDA -> GPIO21, SCL -> GPIO22, AD0 -> GND (address 0x68)
// Serial Monitor 115200, or Tools > Serial Plotter to watch the curves.
// Lying flat and still you should see az ~ +1.0 g and gyro values close to 0.

#include <Wire.h>

#define MPU_ADDR 0x68   // AD0 to GND = 0x68, AD0 to 3.3V = 0x69
#define SDA_PIN  21
#define SCL_PIN  22

#define ACC_LSB_PER_G    4096.0   // +/-8 g
#define GYRO_LSB_PER_DPS 32.8     // +/-1000 deg/s

void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool mpuRead(float *a, float *g, float *tempC) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                                   // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14) != 14) return false;

  int16_t v[7];
  for (int i = 0; i < 7; i++) {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    v[i] = (int16_t)((hi << 8) | lo);
  }
  a[0] = v[0] / ACC_LSB_PER_G;
  a[1] = v[1] / ACC_LSB_PER_G;
  a[2] = v[2] / ACC_LSB_PER_G;
  *tempC = v[3] / 340.0 + 36.53;
  g[0] = v[4] / GYRO_LSB_PER_DPS;
  g[1] = v[5] / GYRO_LSB_PER_DPS;
  g[2] = v[6] / GYRO_LSB_PER_DPS;
  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  // WHO_AM_I: genuine MPU6050 returns 0x68 (clones may return other values and still work)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)1) != 1) {
    while (true) {
      Serial.println("MPU6050 NOT FOUND - check wiring (SDA=21, SCL=22, 3.3V, GND, AD0)");
      delay(1000);
    }
  }
  Serial.printf("MPU6050 found, WHO_AM_I = 0x%02X\n", Wire.read());

  mpuWrite(0x6B, 0x00);   // wake up
  mpuWrite(0x1C, 0x10);   // accel +/-8 g
  mpuWrite(0x1B, 0x10);   // gyro +/-1000 deg/s
  mpuWrite(0x1A, 0x03);   // low-pass filter ~44 Hz
}

void loop() {
  float a[3], g[3], t;
  if (mpuRead(a, g, &t)) {
    Serial.printf("ax:%.2f ay:%.2f az:%.2f gx:%.1f gy:%.1f gz:%.1f temp:%.1f\n",
                  a[0], a[1], a[2], g[0], g[1], g[2], t);
  } else {
    Serial.println("read failed");
  }
  delay(50);
}
