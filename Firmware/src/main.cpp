#include <Arduino.h>
#include <Wire.h>
#include "LoadCell.h"

// --- CONFIGURAÇÕES DE HARDWARE ---
#define I2C_SDA 21 
#define I2C_SCL 22 
#define HX711_DT  13
#define HX711_SCK 18

float fatorCalibracao = -100561.65; 

LoadCell balanca(HX711_DT, HX711_SCK, fatorCalibracao);

unsigned long lastSerialTime = 0;
const int SEND_INTERVAL = 50; 

void setup() {
  Serial.begin(115200);
  delay(2000); 
  
  Serial.println("--- CHECKPOINT 1: Serial Iniciada ---");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("--- CHECKPOINT 2: I2C Iniciado ---");

  Serial.println("Tentando iniciar balança...");
  balanca.begin(); 
  Serial.println("--- CHECKPOINT 3: Balança OK ---");

  Serial.println("{\"status\": \"ready\"}");
}

void loop() {
  balanca.update();

  unsigned long now = millis();
  if (now - lastSerialTime > SEND_INTERVAL) {
    
    float rawThrust = balanca.getThrust();
    float displayThrust = rawThrust * 1.0;

    Serial.print("{\"thrust\": ");
    Serial.print(displayThrust, 4);
    Serial.println("}");

    lastSerialTime = now;
  }
}
