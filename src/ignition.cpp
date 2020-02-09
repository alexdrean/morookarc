#include "ignition.h"

#define KILL_THRESHOLD 0.40
#define START_THRESHOLD 0.80
#define KILL_TIMER 3000
#define CRANK_ACTIVATION_TIMER 3000
#define CRANK_TIMEOUT 3000

Ignition::Ignition(uint8_t voltageProbePin,
                   double voltageProbeFactor,
                   OutputLed *killRelay,
                   OutputLed *crankRelay,
                   double crankMinVoltage,
                   double minVoltage,
                   double crankVoltageRatio,
                   uint8_t onLED,
                   uint8_t crankLED,
                   uint8_t offLED,
                   uint8_t errorLED
) {
    this->voltageProbePin = voltageProbePin;
    this->voltageProbeFactor = voltageProbeFactor;
    this->killRelay = killRelay;
    this->crankRelay = crankRelay;
    this->crankMinVoltage = crankMinVoltage;
    this->minVoltage = minVoltage;
    this->crankVoltageRatio = crankVoltageRatio;
    this->onLED = onLED;
    this->crankLED = crankLED;
    this->offLED = offLED;
    this->errorLED = errorLED;
    this->crankTimeout = 0;
    this->crankActivationTimer = 0;
    this->killTimer = 0;
    this->lastKillSwitchValue = 0.5;
    this->state = ERROR;
    this->crankLowVoltage = 0.0;
    this->crankHighVoltage = 0.0;
}

void Ignition::setup() {
    pinMode(voltageProbePin, INPUT);
    pinMode(onLED, OUTPUT);
    pinMode(offLED, OUTPUT);
    pinMode(crankLED, OUTPUT);
    pinMode(errorLED, OUTPUT);
}

double Ignition::readVoltage() {
    return analogRead(voltageProbePin) / 1024.0 / 1.1 * voltageProbeFactor;
}

void Ignition::handle(double killSwitchValue) {
    unsigned long now = millis();
    if (killSwitchValue < KILL_THRESHOLD) {
        if (state != KILLING) killTimer = now + KILL_TIMER;
        state = KILLING;
    } else if (now < killTimer) {
        state = KILLING;
    } else {
        double voltage = readVoltage();
        if (state == KILLING) state = OFF;
        if (state == CRANKING) {
            if (voltage < crankMinVoltage || now >= crankTimeout) state = ERROR;
            else if (voltage > crankLowVoltage + crankVoltageRatio * (crankHighVoltage - crankLowVoltage)) state = ON;
            else {
                if (voltage < crankLowVoltage) crankLowVoltage = voltage;
            }
        } else if (state == OFF) {
            if (voltage < minVoltage) state = ERROR;
            else if (killSwitchValue > START_THRESHOLD) {
                if (lastKillSwitchValue > START_THRESHOLD) {
                    if (now >= crankActivationTimer) {
                        state = CRANKING;
                        crankTimeout = now + CRANK_TIMEOUT;
                    }
                } else {
                    crankActivationTimer = now + CRANK_ACTIVATION_TIMER;
                    crankHighVoltage = voltage;
                    crankLowVoltage = voltage * 0.95;
                }
            }
        }
    }
    commit();
    this->lastKillSwitchValue = killSwitchValue;
}

void Ignition::commit() {
    killRelay->write(state == KILLING);
    crankRelay->write(state != CRANKING);
    digitalWrite(onLED, static_cast<uint8_t>(state == ON));
    digitalWrite(offLED, static_cast<uint8_t>(state == OFF || state == KILLING));
    digitalWrite(crankLED, static_cast<uint8_t>(state == CRANKING || state == KILLING));
    digitalWrite(errorLED, static_cast<uint8_t>(state == ERROR || state == KILLING));
}
