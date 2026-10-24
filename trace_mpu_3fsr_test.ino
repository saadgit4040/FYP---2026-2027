// MPU6050 + 3 FSR on one ESP32 (this is your foot unit)
// MPU6050: VCC 3.3V, GND, SDA -> GPIO21, SCL -> GPIO22, AD0 -> GND
// FSRs (each: 3.3V -> FSR -> node -> 10k to GND): heel GPIO32, met1 GPIO33, met5 GPIO34
// Prints CSV at 50 Hz. Serial Monitor 115200. Copy the lines into a .csv file to inspect later.

#include <Wire.h>

#define MPU_ADDR 0x68
#define SDA_PIN  21
#define SCL_PIN  22
#define PIN_HEEL 32
#define PIN_MET1 33
#define PIN_MET5 34

#define ACC_LSB_PER_G    4096.0   // +/-8 g
#define GYRO_LSB_PER_DPS 32.8     // +/-1000 deg/s
#define PERIOD_US        20000    // 50 Hz

uint32_t nextTick = 0;

void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool mpuRead(float *a, float *g) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14) != 14) return false;
  int16_t v[7];
  for (int i = 0; i < 7; i++) {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    v[i] = (int16_t)((hi << 8) | lo);
  }
  for (int i = 0; i < 3; i++) {
    a[i] = v[i] / ACC_LSB_PER_G;
    g[i] = v[4 + i] / GYRO_LSB_PER_DPS;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Wire.beginTransmission(MPU_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("MPU6050 NOT FOUND - check wiring");
    while (true) delay(1000);
  }
  mpuWrite(0x6B, 0x00);   // wake
  mpuWrite(0x1C, 0x10);   // accel +/-8 g
  mpuWrite(0x1B, 0x10);   // gyro +/-1000 deg/s
  mpuWrite(0x1A, 0x03);   // low-pass ~44 Hz

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_HEEL, ADC_11db);
  analogSetPinAttenuation(PIN_MET1, ADC_11db);
  analogSetPinAttenuation(PIN_MET5, ADC_11db);

  Serial.println("t_ms,ax,ay,az,gx,gy,gz,heel,met1,met5");
  nextTick = micros();
}

void loop() {
  if ((int32_t)(micros() - nextTick) < 0) return;
  nextTick += PERIOD_US;

  float a[3], g[3];
  if (!mpuRead(a, g)) {
    Serial.println("mpu read failed");
    return;
  }
  int heel = analogRead(PIN_HEEL);
  int met1 = analogRead(PIN_MET1);
  int met5 = analogRead(PIN_MET5);

  Serial.printf("%lu,%.3f,%.3f,%.3f,%.1f,%.1f,%.1f,%d,%d,%d\n",
                (unsigned long)millis(), a[0], a[1], a[2], g[0], g[1], g[2],
                heel, met1, met5);
}
