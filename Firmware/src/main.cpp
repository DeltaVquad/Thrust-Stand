#include <Arduino.h>
#include <Wire.h>
#include "LoadCell.h"
#include "RPMSensor.h"

// --- CONFIGURAÇÕES DE HARDWARE ---
#define I2C_SDA 21 
#define I2C_SCL 22 
#define HX711_DT  13
#define HX711_SCK 18

// --- PARÂMETROS DE CONTROLE ---
// true  = Usa o valor medido pelo sensor RPM (Lidar/Óptico)
// false = Usa o valor calculado matematicamente via Empuxo
bool useSensorRPM = false; 

// Constante da hélice (Calibration Coefficient)
// T = k * RPM^2  ->  RPM = sqrt(T / k)
// Ajuste este valor testando: meça um RPM real e um Empuxo real, e faça k = Empuxo / (RPM^2)
const float PROP_K = 0.00000015; // Valor CHUTE inicial. CALIBRE ISSO!

float fatorCalibracao = -100561.65; 

LoadCell balanca(HX711_DT, HX711_SCK, fatorCalibracao);
RpmSensor sensorRPM;

unsigned long lastSerialTime = 0;
const int SEND_INTERVAL = 50; 

// --- FUNÇÃO DE CÁLCULO FÍSICO (OBSERVER) ---
float calculateRPMFromThrust(float rawThrust) {
  // 1. Aplica a correção de sinal solicitada
  float correctedThrust = rawThrust * -1.0;

  // 2. Proteção contra ruído negativo (raiz quadrada de negativo dá NaN)
  if (correctedThrust <= 0) {
    return 0.0;
  }

  // 3. Cálculo físico: RPM = sqrt(Thrust / k)
  // Se quiser um modelo linear simples (menos preciso), seria: RPM = correctedThrust * fatorLinear
  float estimatedRPM = sqrt(correctedThrust / PROP_K);
  
  return estimatedRPM;
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  
  Serial.println("--- CHECKPOINT 1: Serial Iniciada ---");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("--- CHECKPOINT 2: I2C Iniciado ---");

  Serial.println("Tentando iniciar balança...");
  balanca.begin(); 
  Serial.println("--- CHECKPOINT 3: Balança OK ---");

  sensorRPM.begin();
  Serial.println("--- CHECKPOINT 4: RPM OK ---");

  Serial.println("{\"status\": \"ready\", \"mode\": \"" + String(useSensorRPM ? "SENSOR" : "CALCULATED") + "\"}");
}

void loop() {
  // 1. Atualiza leituras brutas (Drivers)
  balanca.update();
  sensorRPM.update();

  unsigned long now = millis();
  if (now - lastSerialTime > SEND_INTERVAL) {
    
    // 2. Coleta dados crus
    float rawThrust = balanca.getThrust();
    float measuredRPM = sensorRPM.getRPM();

    // 3. Lógica de Seleção de Fonte (Data Fusion switch)
    float finalRPM = 0.0;

    if (useSensorRPM) {
      finalRPM = measuredRPM;
    } else {
      finalRPM = calculateRPMFromThrust(rawThrust);
    }
    
    // O empuxo enviado pro serial também já vai corrigido pelo -1 pra facilitar a plotagem?
    // Vou assumir que você quer ver o empuxo positivo no gráfico.
    float displayThrust = rawThrust * 1.0; 

    // 4. Telemetria JSON
    Serial.print("{\"rpm\": ");
    Serial.print(finalRPM, 0);
    Serial.print(", \"thrust\": ");
    Serial.print(displayThrust, 4); // Enviando já positivo
    Serial.print(", \"source\": \"");
    Serial.print(useSensorRPM ? "sens" : "calc");
    Serial.println("\"}");

    lastSerialTime = now;
  }
}