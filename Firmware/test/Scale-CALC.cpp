#include "HX711.h"

#define DT  18
#define SCK 19

HX711 scale;

float pesoConhecido = 0.0306; // <<< peso REAL em kg (ex: 0.500, 1.000)

void setup() {
  Serial.begin(115200);
  scale.begin(DT, SCK);

  Serial.println("Remova qualquer peso da balança...");
  delay(3000);

  scale.tare(); // zera a balança
  Serial.println("Balança zerada!");

  Serial.println("Coloque o peso conhecido...");
  delay(5000);

  long leitura = scale.get_units(20); // média de 20 leituras

  float fatorScale = leitura / pesoConhecido;

  Serial.println("========== RESULTADO ==========");
  Serial.print("Leitura bruta: ");
  Serial.println(leitura);
  Serial.print("Peso conhecido (kg): ");
  Serial.println(pesoConhecido, 3);
  Serial.print("Fator de scale: ");
  Serial.println(fatorScale, 2);
  Serial.println("================================");
}

void loop() {
  // nada aqui
}
