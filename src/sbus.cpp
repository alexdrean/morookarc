#include "sbus.h"

#define push(value) { buffer[bufferIndex] = value; bufferIndex = (bufferIndex + 1) % 25; }
#define get(index) buffer[(bufferIndex + (index)) % 25]

SBUS::SBUS(HardwareSerial &serial, SBUS_Endpoints endpoints) { // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    this->serial = &serial;
    this->endpoints = endpoints;
    this->bufferIndex = 0;
    this->lastUpdate = 0; // fixme
    for (double & channel : channels)
        channel = 0.5;
    for (uint8_t & byte : buffer)
        byte = 0xFF;
}

void SBUS::begin(unsigned long baudrate, uint8_t config) {
    serial->begin(baudrate, config);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
bool SBUS::read() {
    if (!serial->available()) return false;
    while (serial->available())
        push(static_cast<uint8_t>(Serial.read()))
    if (get(0) != 0x0F || get(24) != 0x00) return false;
    double min = endpoints.min;
    double range = endpoints.max - endpoints.min;
    channels[0]  = (((get(0+1)    |get(1+1) <<8)                     & 0x07FF) - min) / range;
    channels[1]  = (((get(1+1)>>3 |get(2+1) <<5)                     & 0x07FF) - min) / range;
    channels[2]  = (((get(2+1)>>6 |get(3+1) <<2 |get(4+1)<<10)  	 & 0x07FF) - min) / range;
    channels[3]  = (((get(4+1)>>1 |get(5+1) <<7)                     & 0x07FF) - min) / range;
    channels[4]  = (((get(5+1)>>4 |get(6+1) <<4)                     & 0x07FF) - min) / range;
    channels[5]  = (((get(6+1)>>7 |get(7+1) <<1 |get(8+1)<<9)   	 & 0x07FF) - min) / range;
    channels[6]  = (((get(8+1)>>2 |get(9+1) <<6)                     & 0x07FF) - min) / range;
    channels[7]  = (((get(9+1)>>5 |get(10+1)<<3)                     & 0x07FF) - min) / range;
    channels[8]  = (((get(11+1)   |get(12+1)<<8)                     & 0x07FF) - min) / range;
    channels[9]  = (((get(12+1)>>3|get(13+1)<<5)                     & 0x07FF) - min) / range;
    channels[10] = (((get(13+1)>>6|get(14+1)<<2 |get(15+1)<<10) 	 & 0x07FF) - min) / range;
    channels[11] = (((get(15+1)>>1|get(16+1)<<7)                     & 0x07FF) - min) / range;
    channels[12] = (((get(16+1)>>4|get(17+1)<<4)                     & 0x07FF) - min) / range;
    channels[13] = (((get(17+1)>>7|get(18+1)<<1 |get(19+1)<<9)  	 & 0x07FF) - min) / range;
    channels[14] = (((get(19+1)>>2|get(20+1)<<6)                     & 0x07FF) - min) / range;
    channels[15] = (((get(20+1)>>5|get(21+1)<<3)                     & 0x07FF) - min) / range;
    return true;
}
#pragma clang diagnostic pop