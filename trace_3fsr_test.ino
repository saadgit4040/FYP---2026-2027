// 3 FSR test on ESP32 (heel, 1st metatarsal, 5th metatarsal)
// Each FSR: 3.3V -> FSR -> node (GPIO) -> 10k to GND, optional 100nF from node to GND
//   heel  -> GPIO32
//   met1  -> GPIO33
//   met5  -> GPIO34
// Open Tools > Serial Plotter (115200) and press each sensor in turn.

#define PIN_HEEL  32
#define PIN_MET1  33
#define PIN_MET5  34
#define SAMPLES   8

int readAvg(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return sum / SAMPLES;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_HEEL, ADC_11db);
  analogSetPinAttenuation(PIN_MET1, ADC_11db);
  analogSetPinAttenuation(PIN_MET5, ADC_11db);
}

void loop() {
  int heel = readAvg(PIN_HEEL);
  int met1 = readAvg(PIN_MET1);
  int met5 = readAvg(PIN_MET5);

  Serial.printf("heel:%d met1:%d met5:%d\n", heel, met1, met5);
  delay(20);   // ~50 Hz
}
