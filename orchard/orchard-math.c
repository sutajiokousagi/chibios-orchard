#include <stdint.h>
#include "orchard-math.h"
#include "led.h"

static unsigned int rstate = 0xfade1337;

unsigned int shift_lfsr(unsigned int v) {
  /*
    config          : galois
    length          : 16
    taps            : (16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 3, 2)
    shift-amount    : 1
    shift-direction : right
  */
  enum {
    length = 16,
    tap_00 = 16,
    tap_01 = 15,
    tap_02 = 14,
    tap_03 = 13,
    tap_04 = 12,
    tap_05 = 11,
    tap_06 = 10,
    tap_07 =  9,
    tap_08 =  8,
    tap_09 =  7,
    tap_10 =  6,
    tap_11 =  5,
    tap_12 =  3,
    tap_13 =  2
  };
  typedef unsigned int T;
  const T zero = (T)(0);
  const T lsb = zero + (T)(1);
  const T feedback = (
          (lsb << (tap_00 - 1)) ^
          (lsb << (tap_01 - 1)) ^
          (lsb << (tap_02 - 1)) ^
          (lsb << (tap_03 - 1)) ^
          (lsb << (tap_04 - 1)) ^
          (lsb << (tap_05 - 1)) ^
          (lsb << (tap_06 - 1)) ^
          (lsb << (tap_07 - 1)) ^
          (lsb << (tap_08 - 1)) ^
          (lsb << (tap_09 - 1)) ^
          (lsb << (tap_10 - 1)) ^
          (lsb << (tap_11 - 1)) ^
          (lsb << (tap_12 - 1)) ^
          (lsb << (tap_13 - 1))
          );
  v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
  return v;
}

int rand (void) {
  rstate = shift_lfsr(rstate);
  return rstate;
}

// saturating subtract. returns a-b, stopping at 0. assumes 8-bit types
uint8_t satsub_8(uint8_t a, uint8_t b) {
  if (a >= b)
    return (a - b);
  else
    return 0;
}

// saturating add, returns a+b, stopping at 255. assumes 8-bit types
uint8_t satadd_8(uint8_t a, uint8_t b) {
  uint16_t c = (uint16_t) a + (uint16_t) b;

  if (c > 255)
    return (uint8_t) 255;
  else
    return (uint8_t) (c & 0xFF);
}

// saturating subtract, acting on a whole RGB pixel
Color satsub_8p(Color c, uint8_t val) {
  Color rc;
  rc.r = satsub_8( c.r, val );
  rc.g = satsub_8( c.g, val );
  rc.b = satsub_8( c.b, val );

  return rc;
}

// saturating add, acting on a whole RGB pixel
Color satadd_8p( Color c, uint8_t val ) {
  Color rc;
  rc.r = satadd_8( c.r, val );
  rc.g = satadd_8( c.g, val );
  rc.b = satadd_8( c.b, val );

  return rc;
}

