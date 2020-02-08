#pragma once

#include <Arduino.h>
#include <Servo.h>

class OutputLed {
public:
    OutputLed(uint8_t pin, uint8_t led);
    void write(uint8_t value);
};

class ServoLed {
public:
    ServoLed(Servo *servo, Servo *led);
    void writeMicroseconds(int value);
};