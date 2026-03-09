#include <Arduino.h>
#include <Wire.h>
#include "LoadCell.h"
#include "RPMSensor.h"

#define I2C_SDA 21 
#define I2C_SCL 22 
#define HX711_DT  13
#define HX711_SCK 18

float fatorCalibracao = -100561.65; 

LoadCell balanca(HX711_DT, HX711_SCK, fatorCalibracao);
RpmSensor sensorRPM;

unsigned long lastSerialTime = 0;
const int SEND_INTERVAL = 50; 

void setup() {
  Serial.begin(115200);
  delay(2000); // Essencial para dar tempo do monitor serial do Linux estabilizar
  
  Serial.println("--- CHECKPOINT 1: Serial Iniciada ---");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("--- CHECKPOINT 2: I2C Iniciado ---");

  Serial.println("Tentando iniciar balança (se travar aqui, cheque DT/SCK)...");
  balanca.begin(); 
  Serial.println("--- CHECKPOINT 3: Balança OK ---");

  sensorRPM.begin();
  Serial.println("--- CHECKPOINT 4: RPM OK ---");

  Serial.println("{\"status\": \"ready\", \"msg\": \"Sistema Totalmente Operacional\"}");
}

void loop() {
  balanca.update();
  sensorRPM.update();

  unsigned long now = millis();
  if (now - lastSerialTime > SEND_INTERVAL) {
    float rpm = sensorRPM.getRPM();
    float empuxo = balanca.getThrust();

    Serial.print("{\"rpm\": ");
    Serial.print(rpm, 0);
    Serial.print(", \"thrust\": ");
    Serial.print(empuxo, 4);
    Serial.println("}");

    lastSerialTime = now;
  }
}