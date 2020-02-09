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

private:
    void update();
    void commit(double newHeading);

    TinyGPSPlus *gps;
    Servo *servo;
    double heading;
    double targetLat;
    double targetLng;
    double angleOffset;
    double servoBase;
    double servoFactor;
    double range;
    double compass;
};
