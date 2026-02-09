#include "RpmSensor.h"

// Endereço e comando padrão do TF-Luna/Mini
#define LIDAR_ADDRESS 0x10
const unsigned char measureCmd[] = { 0x5A, 0x05, 0x00, 0x01, 0x60 };

RpmSensor::RpmSensor() {
  isObjectDetected = false;
  lastPassTime = 0;
  currentRPM = 0.0;
}

void RpmSensor::begin() {
  // O Wire.begin() deve ser chamado no main.cpp
}

void RpmSensor::update() {
  // 1. Envia comando de leitura
  Wire.beginTransmission(LIDAR_ADDRESS);
  Wire.write(measureCmd, 5);
  if (Wire.endTransmission(true) != 0) return; 

  // 2. Solicita dados
  Wire.requestFrom((int)LIDAR_ADDRESS, 9);
  
  unsigned long start = millis();
  while (Wire.available() < 9) {
    if (millis() - start > 20) return; 
  }

  // 3. Lê os bytes
  uint8_t data[9];
  for (int i = 0; i < 9; i++) data[i] = Wire.read();

  uint16_t distance = (data[3] << 8) | data[2];
  uint16_t strength = (data[5] << 8) | data[4];

  // 4. Lógica de detecção
  if (distance > 0 && distance <= 100 && strength > 100) {
    if (!isObjectDetected) {
      // Borda de subida (Hélice entrou)
      unsigned long now = micros();
      
      if (lastPassTime > 0) {
        unsigned long period = now - lastPassTime;
        
        // Filtro de ruído (ignora leituras absurdas > 30k RPM)
        if (period > 1000) { 
            float freqHz = 1000000.0 / period;
            
            // Calcula RPM: freq (Hz) * 60 (s/min) / 2 (Número de Pás)
            currentRPM = (freqHz * 60.0) / 2.0; 
        }
      }
      
      lastPassTime = now;
      isObjectDetected = true;
    }
  } else if (distance > 100 && isObjectDetected) {
    // Objeto saiu
    isObjectDetected = false;
  }

  // 5. Timeout: Zera se ficar 2s sem passar nada
  if (micros() - lastPassTime > 2000000) {
    currentRPM = 0;
  }
}

float RpmSensor::getRPM() {
  return currentRPM;
}