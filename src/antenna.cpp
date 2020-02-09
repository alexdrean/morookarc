#include "antenna.h"

#define ANGLE_THRESHOLD 5.0

static inline double positiveModulo(double i, double n) {
    const double k = fmod(i, n);
    return k < 0 ? k + n : k;
}

static inline double diffAngle(double a, double b) {
    const double diff = positiveModulo(b - a, 360);
    return diff > 180 ? diff - 360 : diff;
}

static inline double clampAngle(double angle, double range) {
    if (angle > range) {
        return angle - ceil((angle - range) / 360.0) * 360.0;
    } else if (angle < range) {
        return angle + ceil((range - angle) / 360.0) * 360.0;
    } else {
        return angle;
    }
}

Antenna::Antenna(Servo *servo,
                 TinyGPSPlus *gps,
                 double angleOffset,
                 int servoMin,
                 int servoMax,
                 double turns,
                 double targetLat,
                 double targetLng
) {
    this->servo = servo;
    this->gps = gps;
    this->angleOffset = angleOffset;
    this->servoBase = (servoMin + servoMax) / 2.0;
    this->servoFactor = (servoMax - servoMin) / (turns * 360);
    this->range = turns * 180;
    this->targetLat = targetLat;
    this->targetLng = targetLng;
    this->compass = 0;
    this->heading = 0;
}

void Antenna::updateGPS() {
    if (gps->location.isUpdated())
        update();
}

void Antenna::setCompass(double headingDeg) {
    this->compass = headingDeg;
    update();
}

void Antenna::update() {
    double course = TinyGPSPlus::courseTo(gps->location.lat(), gps->location.lng(), targetLat, targetLng);
    double targetHeading = course - compass + angleOffset;
    double diff = diffAngle(heading, targetHeading);
    if (fabs(diff) < ANGLE_THRESHOLD) return;
    commit(clampAngle(heading + diff, range));
}

void Antenna::commit(double newHeading) {
    this->heading = newHeading;
    servo->writeMicroseconds((int) (servoBase + servoFactor * newHeading));
}
