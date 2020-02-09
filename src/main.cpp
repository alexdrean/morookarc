#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Arduino.h>
#include <Servo.h>
#include <TinyGPS++.h>
#include "sbus.h"
#include "ignition.h"
#include "outputled.h"
#include "antenna.h"

struct Output {
    ServoLed *servo;
    uint8_t channel;
    uint16_t min;
    uint16_t max;
};

const double *getChannels();

void handlePassthrough(const double *channels);

void handleAntenna(double compass);

void handleGPS();

Servo leftTrack;
Servo leftTrackLed;
Servo rightTrack;
Servo rightTrackLed;
Servo dump;
Servo dumpLed;
Servo antennaServo;

#define VOLTAGE_PROBE_PIN A14
#define VOLTAGE_PROBE_FACTOR 34.8
#define RX_TIMEOUT 100
#define LEFT_TRACK_PIN 41
#define LEFT_TRACK_LED_PIN 28
#define RIGHT_TRACK_PIN 43
#define RIGHT_TRACK_LED_PIN 26
#define DUMP_PIN 45
#define DUMP_LED_PIN 24
#define ANTENNA_PIN 21
#define COMPASS_CHANNEL 16
#define IGNITION_CHANNEL 1

OutputLed crankRelay(25, 30);
OutputLed killRelay(29, 20);
Ignition ignition(VOLTAGE_PROBE_PIN, VOLTAGE_PROBE_FACTOR, &killRelay, &crankRelay, 13, 22, 0.7, 2, 8, 4, 6);
byte rxLed[] = {38, 42, 33};
SBUS rx(Serial1, {172, 1811}); // NOLINT(cppcoreguidelines-interfaces-global-init)
byte px4Led[] = {40, 44, 46};
SBUS px4(Serial2, {172, 1811}); // NOLINT(cppcoreguidelines-interfaces-global-init)
byte piLed[] = {36, 34, 32};
SBUS pi(Serial, {172, 1811}); // NOLINT(cppcoreguidelines-interfaces-global-init)
ServoLed rightTrackOutput(&rightTrack, &rightTrackLed);
ServoLed leftTrackOutput(&leftTrack, &leftTrackLed);
ServoLed dumpOutput(&dump, &dumpLed);

TinyGPSPlus gps;
Antenna antenna(&antennaServo, &gps, 0, 1286, 1714, 2, 32.366737, -112.869810);

const double default_channels[16] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const struct Output outputs[] = {
        {.servo = &rightTrackOutput, .channel = 2, .min = 1000, .max = 2000},
        {.servo = &leftTrackOutput, .channel = 3, .min = 1000, .max = 2000},
        {.servo = &dumpOutput, .channel = 4, .min = 1000, .max = 2000}
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void setup() {
    analogReference(INTERNAL1V1);
    leftTrack.attach(LEFT_TRACK_PIN, 1000, 2000);
    rightTrack.attach(RIGHT_TRACK_PIN, 1000, 2000);
    dump.attach(DUMP_PIN, 1000, 2000);
    leftTrackLed.attach(LEFT_TRACK_LED_PIN);
    rightTrackLed.attach(RIGHT_TRACK_LED_PIN);
    dumpLed.attach(DUMP_LED_PIN);
    antennaServo.attach(ANTENNA_PIN, 1000, 2000);
    rx.begin();
    px4.begin();
    pi.begin(115200, SERIAL_8E1);
    Serial3.begin(115200); // GPS
    ignition.setup();
    crankRelay.setup();
    killRelay.setup();
    for (auto pin : piLed) pinMode(pin, OUTPUT);
    for (auto pin : px4Led) pinMode(pin, OUTPUT);
    for (auto pin : rxLed) pinMode(pin, OUTPUT);
}

#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void loop() {
    const double *channels = getChannels();
    handlePassthrough(channels);
    handleAntenna(pi.channels[COMPASS_CHANNEL - 1]);
    ignition.handle(channels[IGNITION_CHANNEL - 1]);
    handleGPS();
}

#pragma clang diagnostic pop

void handlePassthrough(const double *channels) {
    for (auto o : outputs) {
        o.servo->writeMicroseconds((int) (o.min + (o.max - o.min) * channels[o.channel - 1]));
    }
}

void handleAntenna(double compass) {
    antenna.setCompass(compass * 360);
}

void handleGPS() {
    if (Serial3.available()) {
        while (Serial3.available())
            gps << (char) Serial3.read();
        antenna.updateGPS();
    }
}

const double *getChannels() {
    rx.read();
    px4.read();
    pi.read();
    unsigned long now = millis();
    auto rxOk = static_cast<uint8_t>(now - rx.lastUpdate < RX_TIMEOUT);
    auto piOk = static_cast<uint8_t>(now - pi.lastUpdate < RX_TIMEOUT);
    auto px4Ok = static_cast<uint8_t>(now - px4.lastUpdate < RX_TIMEOUT);
    Serial.print("rx = ");
    Serial.println(rxOk);
    Serial.print("pi = ");
    Serial.println(piOk);
    Serial.print("px = ");
    Serial.println(px4Ok);
    digitalWrite(rxLed[0], static_cast<uint8_t>(!rxOk));
    digitalWrite(rxLed[1], rxOk);
    digitalWrite(rxLed[2], 0);
    digitalWrite(piLed[0], static_cast<uint8_t>(!piOk));
    digitalWrite(piLed[1], piOk);
    digitalWrite(piLed[2], 0);
    digitalWrite(px4Led[0], static_cast<uint8_t>(!px4Ok));
    digitalWrite(px4Led[1], px4Ok);
    digitalWrite(px4Led[2], 0);
    if (rxOk && rx.channels[4] < 0.5) {
        Serial.println("using direct rx");
        digitalWrite(rxLed[2], 1);
        return rx.channels;
    } else if (piOk && pi.channels[4] < 0.5) {
        Serial.println("using pi");
        digitalWrite(piLed[2], 1);
        return pi.channels;
    } else if (px4Ok) {
        Serial.println("using px4");
        digitalWrite(px4Led[2], 1);
        return px4.channels;
    }
    Serial.println("using failsafe");
    return default_channels;
}
