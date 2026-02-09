#ifndef LOADCELL_H
#define LOADCELL_H

#include <Arduino.h>
#include "HX711.h"

class LoadCell {
  private:
    HX711 hx;
    int pinDT;
    int pinSCK;
    float calibrationFactor;
    
    // Configurações do Filtro de Média Móvel (Circular)
    static const int N = 10; // Tamanho do buffer (ajuste se quiser mais suavidade)
    float buffer[N];
    int indexFiltro;
    float soma;
    bool bufferCheio;
    float ultimaMedia; 

  public:
    LoadCell(int dt, int sck, float factor);
    void begin();
    void update();      // Executa a leitura e o filtro (chamar no loop rápido)
    float getThrust();  // Retorna o valor limpo (chamar quando for enviar)
    void tare();        // Zera a balança
};

#endif