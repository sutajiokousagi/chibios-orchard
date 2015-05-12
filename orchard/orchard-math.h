#ifndef __ORCHARD_MATH_H__
#define __ORCHARD_MATH_H__

#include <stdint.h>

unsigned int shift_lfsr(unsigned int v);
uint8_t satsub_8(uint8_t a, uint8_t b);
uint8_t satadd_8(uint8_t a, uint8_t b);

#endif /* __ORCHARD_MATH_H__ */
