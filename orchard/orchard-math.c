#include <stdint.h>
#include "orchard-math.h"
#include "led.h"
#include "fixmath.h"

static uint32_t rstate[2] = {0xbabeface, 0xfade1337};
static uint32_t key[2] = {0x243F6A88, 0x85A308D3}; // from pi

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

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
 
void btea(uint32_t *v, int n, uint32_t const key[4]) {
  uint32_t y, z, sum;
  unsigned rounds, e;
  int p;
  if (n > 1) {          /* Coding Part */
    rounds = 6 + 52/n;
    sum = 0;
    z = v[n-1];
    do {
      sum += DELTA;
      e = (sum >> 2) & 3;
      for (p=0; p<n-1; p++) {
	y = v[p+1]; 
	z = v[p] += MX;
      }
      y = v[0];
      z = v[n-1] += MX;
    } while (--rounds);
  } else if (n < -1) {  /* Decoding Part */
    n = -n;
    rounds = 6 + 52/n;
    sum = rounds*DELTA;
    y = v[0];
    do {
      e = (sum >> 2) & 3;
      for (p=n-1; p>0; p--) {
	z = v[p-1];
	y = v[p] -= MX;
      }
      z = v[n-1];
      y = v[0] -= MX;
      sum -= DELTA;
    } while (--rounds);
  }
}

void addEntropy(uint32_t value) {
  if( value & 1 ) {
    key[0] ^= value;
    key[1] ^= chVTGetSystemTime();
  } else {
    key[1] ^= value;
    key[0] ^= chVTGetSystemTime();
  }
  
  btea(rstate, 2, key);
}

int rand (void) {
  btea( rstate, 2, key );
  return rstate[0] ^ rstate[1];
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

// saturating add, returns a+b, stopping at limit. 
uint8_t satadd_8_limit(uint8_t a, uint8_t b, uint8_t limit) {
  uint16_t c = (uint16_t) a + (uint16_t) b;

  if (c > limit)
    return (uint8_t) limit;
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

int16_t map_16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max)
{
  fix16_t x16, in_min16, in_max16, out_min16, out_max16;
  fix16_t result16;

  x16 = fix16_from_int(x);
  in_min16 = fix16_from_int(in_min);
  in_max16 = fix16_from_int(in_max);
  out_min16 = fix16_from_int(out_min);
  out_max16 = fix16_from_int(out_max);

  result16 = fix16_div( fix16_sub(out_max16, out_min16), fix16_sub(in_max16, in_min16) );
  result16 = fix16_mul( result16, fix16_sub( x16, in_min16 ));
  result16 = fix16_add( result16, out_min16 );
  return fix16_to_int(result16);
  
  //return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
