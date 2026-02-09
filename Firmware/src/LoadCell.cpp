#include "LoadCell.h"

LoadCell::LoadCell(int dt, int sck, float factor) {
  pinDT = dt;
  pinSCK = sck;
  calibrationFactor = factor;
  
  // Inicializa variáveis do filtro
  indexFiltro = 0;
  soma = 0.0;
  bufferCheio = false;
  ultimaMedia = 0.0;
  
  for(int i=0; i<N; i++) buffer[i] = 0.0;
}

void LoadCell::begin() {
  hx.begin(pinDT, pinSCK);
  hx.set_scale(calibrationFactor);
  hx.tare(); 
}

void LoadCell::update() {
  // is_ready verifica se o hardware terminou a conversão (evita travar o ESP)
  if (hx.is_ready()) {
    float leitura = hx.get_units(1); // Lê 1 amostra bruta

    // Lógica do Filtro Circular
    soma -= buffer[indexFiltro];       
    buffer[indexFiltro] = leitura;     
    soma += leitura;                   

    indexFiltro = (indexFiltro + 1) % N; 
    if (indexFiltro == 0) bufferCheio = true;

    // Atualiza a média armazenada
    if (bufferCheio) {
      ultimaMedia = soma / N;
    } else {
      ultimaMedia = soma / indexFiltro;
    }
  }
}

float LoadCell::getThrust() {
  return ultimaMedia;
}

void LoadCell::tare() {
  hx.tare();
  // Reseta o filtro para evitar "arrasto" de valores antigos
  soma = 0;
  indexFiltro = 0;
  bufferCheio = false;
  for(int i=0; i<N; i++) buffer[i] = 0.0;
  ultimaMedia = 0.0;
}