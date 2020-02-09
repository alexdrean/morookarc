#pragma once

#include <Arduino.h>

struct SBUS_Endpoints {
    double min;
    double max;
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
    void begin(unsigned long baudrate, uint8_t config);
    bool read();

    unsigned long lastUpdate;
    double channels[16];
private:
    uint8_t buffer[25];
    int bufferIndex;
    HardwareSerial *serial;
    SBUS_Endpoints endpoints;
};
