#pragma once
#include <stdint.h>
#include <stddef.h>

#define Light_sensor 35
#define DHT_pin 13
#define Relay_one 26
#define Relay_two 25
#define Button_light 27
#define Button_warm 14
#define Manual_mode 0
#define DHTTYPE DHT11
#define TEMP_THRESHOLD_LOW  23.5f
#define TEMP_THRESHOLD_HIGH 24.5f
#define solar 34
// ==========================================
// CHECK SUM
// ==========================================
#define SYNC_BYTE_1 0xAA
#define SYNC_BYTE_2 0x55
#define BLUE 2

const uint16_t dhtInterval = 2000; 
const uint16_t dreb_delay = 200;
const uint16_t solarInterval = 2000;



typedef struct __attribute__((packed)) {
    uint8_t  sync1;
    uint8_t  sync2;
    uint8_t hum;
    uint16_t temp;
    uint8_t relay1;
    uint8_t relay2;
    uint8_t light_sensor_state;
    uint8_t manual;
    uint8_t check_sum;
} TelemetryPacket;



enum SystemState {
    STATE_BOOT,//start and diagnostic
    STATE_AUTO, // auto mode
    STATE_MANUAL,// manual mode
    STATE_EMERGENCY // emergency incedents
};

enum SolarState {
    SOLAR_STOWED,    // Сложены в обтекателе
    SOLAR_DEPLOYING, // Идёт раскрытие приводов
    SOLAR_DEPLOYED   // Полностью зафиксированы
};


inline uint8_t calculate_checksum(const uint8_t *data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
    }
    return crc;
}