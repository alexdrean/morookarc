#include "outputled.h"

OutputLed::OutputLed(uint8_t pin, uint8_t led) {
    this->pin = pin;
    this->led = led;
}

void OutputLed::setup() {
    pinMode(pin, OUTPUT);
    pinMode(led, OUTPUT);
}

void OutputLed::write(bool value) {
    digitalWrite(pin, static_cast<uint8_t>(value));
    digitalWrite(led, static_cast<uint8_t>(value));
}

ServoLed::ServoLed(Servo *servo, Servo *led) {
    this->servo = servo;
    this->led = led;
}

void ServoLed::writeMicroseconds(int value) {
    servo->writeMicroseconds(value);
    led->write(static_cast<int>(map(value, 1000, 2000, 0, 180)));
}
