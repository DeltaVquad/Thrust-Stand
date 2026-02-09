#ifndef RPMSENSOR_H
#define RPMSENSOR_H

#include <Arduino.h>
#include <Wire.h>

class RpmSensor {
  private:
    bool isObjectDetected;
    unsigned long lastPassTime;
    float currentRPM;

  public:
    RpmSensor(); // Construtor simplificado
    void begin();
    void update();
    float getRPM();
};

#endif