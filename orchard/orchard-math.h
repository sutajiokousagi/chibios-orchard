#ifndef __ORCHARD_MATH_H__
#define __ORCHARD_MATH_H__

#include <stdint.h>

unsigned int shift_lfsr(unsigned int v);
uint8_t satsub_8(uint8_t a, uint8_t b);
uint8_t satadd_8(uint8_t a, uint8_t b);
void addEntropy(uint32_t value);
int rand(void);
int16_t map_16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max);
int map(int x, int in_min, int in_max, int out_min, int out_max);
uint8_t satadd_8_limit(uint8_t a, uint8_t b, uint8_t limit);

#endif /* __ORCHARD_MATH_H__ */
