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
#define ANTENNA_CALIBRATION TRUE

OutputLed crankRelay(25, 30);
OutputLed killRelay(29, 20);
Ignition ignition(VOLTAGE_PROBE_PIN, VOLTAGE_PROBE_FACTOR, &killRelay, &crankRelay, 13, 22, 0.7, 2, 8, 4, 6);
byte rxLed[] = { 38, 42, 33 };
SBUS rx(Serial1, {172, 1811});
byte px4Led[] = { 40, 44, 46 };
SBUS px4(Serial2, {172, 1811});
byte piLed[] = { 36, 34, 32 };
SBUS pi(Serial, {172, 1811});
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

const int outputCount = sizeof(outputs) / sizeof(struct Output);

void setup() {
    for (int i = 0; i < 54; i++) {
        pinMode(i, OUTPUT);
    }
    analogReference(INTERNAL1V1);
    leftTrack.attach(LEFT_TRACK_PIN);
    rightTrack.attach(RIGHT_TRACK_PIN);
    dump.attach(DUMP_PIN);
    leftTrackLed.attach(LEFT_TRACK_LED_PIN);
    rightTrackLed.attach(RIGHT_TRACK_LED_PIN);
    dumpLed.attach(DUMP_LED_PIN);
    antennaServo.attach(ANTENNA_PIN);
    rx.begin();
    px4.begin();
    pi.begin(115200, SERIAL_8E1);
    Serial3.begin(115200); // GPS
}

void loop() {
    const double *channels = getChannels();
    handlePassthrough(channels);
    handleAntenna(pi.channels[COMPASS_CHANNEL - 1]);
    ignition.handle(channels[IGNITION_CHANNEL - 1]);
    handleGPS();
}

void handlePassthrough(const double *channels) {
    for (int i = 0; i < outputCount; i++) {
        const struct Output o = outputs[i];
        o.servo->writeMicroseconds(o.min + (o.max - o.min) * channels[o.channel - 1]);
    }
}

void handleAntenna(double compass) {
    antenna.setCompass(compass * 360);
}

void handleGPS() {
    if (Serial3.available()) {
        while (Serial3.available())
            gps << Serial3.read();
        antenna.updateGPS();
    }
}

const double *getChannels() {
    rx.read();
    px4.read();
    pi.read();
    unsigned long now = millis();
    bool rxgood = now - rx.lastUpdate < RX_TIMEOUT;
    bool pigood = now - pi.lastUpdate < RX_TIMEOUT;
    bool px4good = now - px4.lastUpdate < RX_TIMEOUT;
    Serial.print("rx = ");
    Serial.println(rxgood);
    Serial.print("pi = ");
    Serial.println(pigood);
    Serial.print("px = ");
    Serial.println(px4good);
    digitalWrite(rxLed[0], !rxgood);
    digitalWrite(rxLed[1], rxgood);
    digitalWrite(rxLed[2], 0);
    digitalWrite(piLed[0], !pigood);
    digitalWrite(piLed[1], pigood);
    digitalWrite(piLed[2], 0);
    digitalWrite(px4Led[0], !px4good);
    digitalWrite(px4Led[1], px4good);
    digitalWrite(px4Led[2], 0);
    if (rxgood && rx.channels[4] < 0.5) {
        Serial.println("using direct rx");
        digitalWrite(rxLed[2], 1);
        return rx.channels;
    } else if (pigood && pi.channels[4] < 0.5) {
        Serial.println("using pi");
        digitalWrite(piLed[2], 1);
        return pi.channels;
    } else if (px4good) {
        Serial.println("using px4");
        digitalWrite(px4Led[2], 1);
        return px4.channels;
    }
    Serial.println("using failsafe");
    return default_channels;
}
