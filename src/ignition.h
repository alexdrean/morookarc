#pragma once

#include <Arduino.h>
#include "outputled.h"

enum IgnitionState {
    ERROR,
    OFF,
    CRANKING,
    ON,
    KILLING,
};

class Ignition {
public:
    Ignition(uint8_t voltageProbePin,
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
    );

    void handle(double killSwitchValue);

private:
    uint8_t voltageProbePin;
    double voltageProbeFactor;
    OutputLed *killRelay;
    OutputLed *crankRelay;
    double crankMinVoltage;
    double crankVoltageRatio;
    double minVoltage;
    uint8_t onLED;
    uint8_t crankLED;
    uint8_t offLED;
    uint8_t errorLED;

    double readVoltage();
    void commit();

    unsigned long crankActivationTimer;
    unsigned long killTimer;
    unsigned long crankTimeout;
    double crankLowVoltage;
    double crankHighVoltage;
    double lastKillSwitchValue;
    IgnitionState state;
};
