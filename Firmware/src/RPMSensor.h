#ifndef RPM_SENSOR_H
#define RPM_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

class RpmSensor {
public:
    RpmSensor();
    void begin();
    void update();
    float getRPM();

private:
    bool isObjectDetected;
    uint32_t lastPassTime;
    uint32_t lastSampleTime;
    float currentRPM;
};

#endif