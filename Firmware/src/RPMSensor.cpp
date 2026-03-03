#include "RPMSensor.h"

#define LIDAR_ADDRESS 0x10
#define MAX_DISTANCE 100        // mm
#define MIN_STRENGTH 80
#define SAMPLE_INTERVAL 10000UL // 10ms = 100Hz

#define MIN_PERIOD_US 1000      // Limite físico (~30k RPM com 2 pás)
#define TIMEOUT_US 2000000UL    // 2s

const unsigned char measureCmd[] = { 0x5A, 0x05, 0x00, 0x01, 0x60 };

RpmSensor::RpmSensor() {
  isObjectDetected = false;
  lastPassTime = 0;
  currentRPM = 0.0;
  lastSampleTime = 0;
}

void RpmSensor::begin() {
    // Nada por enquanto
}

void RpmSensor::update() {

  unsigned long nowMicros = micros();

  // 1️⃣ Limita taxa de leitura
  if (nowMicros - lastSampleTime < SAMPLE_INTERVAL)
    return;

  lastSampleTime = nowMicros;

  // 2️⃣ Solicita dados
  Wire.beginTransmission(LIDAR_ADDRESS);
  Wire.write(measureCmd, 5);
  if (Wire.endTransmission(false) != 0) return;

  Wire.requestFrom(LIDAR_ADDRESS, 9);
  if (Wire.available() < 9) return;

  uint8_t data[9];
  for (int i = 0; i < 9; i++)
    data[i] = Wire.read();

  uint16_t distance = (data[3] << 8) | data[2];
  uint16_t strength = (data[5] << 8) | data[4];

  bool objectNow = (distance > 0 && distance <= MAX_DISTANCE && strength > MIN_STRENGTH);

  // 3️⃣ Borda de subida
  if (objectNow && !isObjectDetected) {

    if (lastPassTime > 0) {

      unsigned long period = nowMicros - lastPassTime;

      // Filtro físico
      if (period > MIN_PERIOD_US) {

        // RPM direto (2 pás)
        float rpm = (60000000.0f / period) / 2.0f;

        // Filtro exponencial simples
        currentRPM = currentRPM * 0.7f + rpm * 0.3f;
      }
    }

    lastPassTime = nowMicros;
    isObjectDetected = true;
  }

  // 4️⃣ Histerese de saída
  if (!objectNow)
    isObjectDetected = false;

  // 5️⃣ Timeout
  if (nowMicros - lastPassTime > TIMEOUT_US)
    currentRPM = 0;
}

float RpmSensor::getRPM() {
  return currentRPM;
}