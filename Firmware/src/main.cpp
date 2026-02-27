#include <Arduino.h>
#include <Wire.h>
#include "LoadCell.h"
#include "RpmSensor.h" // Inclui o novo módulo

// ======= Configurações I2C Gerais =====
#define I2C_SDA 21 
#define I2C_SCL 22 

// ======= Configurações Módulos =========
// LoadCell
#define HX711_DT  15
#define HX711_SCK 18
float fatorCalibracao = 94313.73; 

// RPM (Lidar)
#define LIDAR_ADDRESS 0x10

// --- Instanciação dos Objetos ---
LoadCell balanca(HX711_DT, HX711_SCK, fatorCalibracao);
RpmSensor sensorRPM;

// Variáveis de controle de envio
unsigned long lastSerialTime = 0;
const int SEND_INTERVAL = 50; 

void setup() {
  Serial.begin(115200);

  // Inicia I2C uma vez para todos os sensores
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicia os módulos
  balanca.begin();
  sensorRPM.begin();

  Serial.println("{\"status\": \"ready\", \"msg\": \"Sistema Modularizado Iniciado\"}");
}

void loop() {
  // 1. Atualiza os sensores (chamar sempre!)
  balanca.update();
  sensorRPM.update();

  // 2. Envio de dados via Serial (JSON)
  unsigned long now = millis();
  if (now - lastSerialTime > SEND_INTERVAL) {
    // Pega os valores processados "bonitinhos" de cada classe
    float rpm = sensorRPM.getRPM();
    float empuxo = balanca.getThrust();

    Serial.print("{\"rpm\": ");
    Serial.print(rpm, 0);
    Serial.print(", \"thrust\": ");
    Serial.print(empuxo, 4);
    Serial.println("}");

    lastSerialTime = now;
  }

  // Comandos via Serial (Ex: Tarar)
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'z' || cmd == 'Z') {
      balanca.tare();
    }
  }
}
