#pragma once
#include <stdint.h>
#include <stddef.h>

#define SYNC_BYTE_1 0xAA
#define SYNC_BYTE_2 0x55

typedef struct __attribute__((packed)) {
    uint8_t sync1;
    uint8_t sync2;
    uint32_t uptime;      
    uint32_t id;
    uint32_t motion_count;
    uint8_t S_stat;
    uint8_t checksum;
} SpacecraftPacket;

inline uint8_t calculate_checksum(const uint8_t *data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
    }
    return crc;
}