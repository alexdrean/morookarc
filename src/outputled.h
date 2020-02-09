#pragma once

#include <Arduino.h>
#include <Servo.h>

class OutputLed {
public:
    OutputLed(uint8_t pin, uint8_t led);
    void setup();
    void write(bool value);

private:
    uint8_t pin;
    uint8_t led;
};

class ServoLed {
public:
    ServoLed(Servo *servo, Servo *led);
    void writeMicroseconds(int value);

private:
    Servo *servo;
    Servo *led;
};