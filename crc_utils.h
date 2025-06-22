#ifndef CRC_UTILS_H
#define CRC_UTILS_H

#include <stdint.h>

int crc_8(uint8_t message[], int length);
int crc_16(uint8_t message[], int length);

#endif // CRC_UTILS_H
