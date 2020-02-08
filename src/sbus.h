#pragma once

#include <Arduino.h>

struct SBUS_Endpoints {
    unsigned int min;
    unsigned int max;
};

class SBUS {
public:
    SBUS(HardwareSerial& serial, SBUS_Endpoints endpoints);
    void begin() {
        begin(100000);
    }
    void begin(unsigned long baudrate) {
        begin(baudrate, SERIAL_8E2);
    }
    void begin(unsigned long baudrate, uint8_t);
    bool read();

    unsigned long lastUpdate;
    double channels[16];
private:
    SBUS_Endpoints endpoints;
};
