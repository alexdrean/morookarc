#pragma once

#include <Arduino.h>
#include <Servo.h>
#include <TinyGPS++.h>

class Antenna {
public:
    Antenna(Servo *servo,
            TinyGPSPlus *gps,
            double angleOffset,
            int servoMin,
            int servoMax,
            double turns,
            double targetLat,
            double targetLng
    );
    void setCompass(double headingDeg);
    void updateGPS();
};