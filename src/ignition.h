#pragma once

#include <Arduino.h>
#include "outputled.h"

class Ignition {
public:
    Ignition(uint8_t voltage_probe_pin,
             double voltage_probe_factor,
             OutputLed *killRelay,
             OutputLed *crankRelay,
             double crankMinVoltage,
             double minVoltage,
             double crankVoltageRatio,
             int onLED,
             int crankLED,
             int offLED,
             int errorLED
    );

    void handle(double killSwitchValue);
};
