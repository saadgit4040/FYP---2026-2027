// Single FSR test on ESP32
// Wiring: 3.3V -> FSR -> node (GPIO34) -> 10k resistor -> GND
//         optional: 100nF capacitor from the node to GND
// Open Serial Monitor (115200) or Tools > Serial Plotter and press the FSR.

#define FSR_PIN   34        // ADC1 pin (input-only, works fine alongside Wi-Fi/BLE)
#define R_FIXED   10000.0   // ohms, the resistor to GND
#define VCC       3.3
#define SAMPLES   16        // averaging to reduce noise

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);                  // 0..4095
  analogSetPinAttenuation(FSR_PIN, ADC_11db); // full 0..~3.3V range
  Serial.println("FSR test started");
}

void loop() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(FSR_PIN);
    delayMicroseconds(200);
  }
  float raw = sum / (float)SAMPLES;
  float volts = raw / 4095.0 * VCC;

  Serial.printf("raw=%4.0f  V=%.2f  ", raw, volts);
  if (volts < 0.02) {
    Serial.println("R_fsr=open (no press)");
  } else {
    float rFsr = R_FIXED * (VCC - volts) / volts;  // voltage divider maths
    Serial.printf("R_fsr=%.0f ohm\n", rFsr);
  }
  delay(50);
}
