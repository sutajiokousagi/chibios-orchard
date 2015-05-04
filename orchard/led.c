#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "led.h"

#include <math.h>

struct effects_config {
  void *fb;
  uint32_t count;
  uint32_t loop;
  enum pattern pattern;
};

static struct {
  uint8_t       *fb; // LED frame buffer
  uint32_t      pixel_count;  // generated pixel length
  uint32_t      max_pixels;   // maximal generation length
} led_config;


uint8_t shift = 0;  // start a little bit dimmer

uint32_t bump_amount = 0;
uint8_t bumped = 0;
unsigned int bumptime = 0;
unsigned long reftime = 0;
unsigned long reftime_tau = 0;
unsigned long offset = 0;
unsigned int waverate = 10;
unsigned int waveloop = 0;
unsigned int patternChanged = 0;

int wavesign = -1;

unsigned int rstate = 0xfade1337;
unsigned int shift_lfsr(unsigned int v);

uint8_t sinLUT[256] = {
  0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,
  0x98,0x9b,0x9e,0xa2,0xa5,0xa7,0xaa,0xad,
  0xb0,0xb3,0xb6,0xb9,0xbc,0xbe,0xc1,0xc4,
  0xc6,0xc9,0xcb,0xce,0xd0,0xd3,0xd5,0xd7,
  0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,
  0xea,0xeb,0xed,0xee,0xf0,0xf1,0xf3,0xf4,
  0xf5,0xf6,0xf8,0xf9,0xfa,0xfa,0xfb,0xfc,
  0xfd,0xfd,0xfe,0xfe,0xfe,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xfe,0xfe,0xfe,0xfd,
  0xfd,0xfc,0xfb,0xfa,0xfa,0xf9,0xf8,0xf6,
  0xf5,0xf4,0xf3,0xf1,0xf0,0xee,0xed,0xeb,
  0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
  0xda,0xd7,0xd5,0xd3,0xd0,0xce,0xcb,0xc9,
  0xc6,0xc4,0xc1,0xbe,0xbc,0xb9,0xb6,0xb3,
  0xb0,0xad,0xaa,0xa7,0xa5,0xa2,0x9e,0x9b,
  0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
  0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,
  0x67,0x64,0x61,0x5d,0x5a,0x58,0x55,0x52,
  0x4f,0x4c,0x49,0x46,0x43,0x41,0x3e,0x3b,
  0x39,0x36,0x34,0x31,0x2f,0x2c,0x2a,0x28,
  0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,
  0x15,0x14,0x12,0x11,0xf,0xe,0xc,0xb,
  0xa,0x9,0x7,0x6,0x5,0x5,0x4,0x3,
  0x2,0x2,0x1,0x1,0x1,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,
  0x2,0x3,0x4,0x5,0x5,0x6,0x7,0x9,
  0xa,0xb,0xc,0xe,0xf,0x11,0x12,0x14,
  0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
  0x25,0x28,0x2a,0x2c,0x2f,0x31,0x34,0x36,
  0x39,0x3b,0x3e,0x41,0x43,0x46,0x49,0x4c,
  0x4f,0x52,0x55,0x58,0x5a,0x5d,0x61,0x64,
  0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c,
};


/**
 * @brief   Initialize Led Driver
 * @details Initialize the Led Driver based on parameters.
 *
 * @param[in] leds      length of the LED chain controlled by each pin
 * @param[out] o_fb     initialized frame buffer
 *
 */
void ledStart(uint32_t leds, uint8_t *o_fb)
{
  unsigned int j;
  led_config.max_pixels = leds;
  led_config.pixel_count = leds;

  led_config.fb = o_fb;
  for (j = 0; j < leds * 3; j++)
    led_config.fb[j] = 0x0;

  chSysLock();
  ledUpdate(led_config.fb, led_config.max_pixels);
  chSysUnlock();
}

void ledSetRGBClipped(void *fb, uint32_t i,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
  if (i >= led_config.pixel_count)
    return;
  ledSetRGB(fb, i, r, g, b, shift);
}

void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = g >> shift;
  buf[1] = r >> shift;
  buf[2] = b >> shift;
}

void ledSetColor(void *ptr, int x, Color c, uint8_t shift) {
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = c.g >> shift;
  buf[1] = c.r >> shift;
  buf[2] = c.b >> shift;
}

Color ledGetColor(void *ptr, int x) {
  Color c;
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);

  c.g = buf[0];
  c.r = buf[1];
  c.b = buf[2];
  
  return c;
}

void ledSetCount(uint32_t count) {
  if (count > led_config.max_pixels)
    return;
  led_config.pixel_count = count;
}



void setShift(uint8_t s) {
  shift = s;
}

uint8_t getShift(void) {
  return shift;
}

unsigned int shift_lfsr(unsigned int v)
{
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

unsigned int rand(void) {
  rstate = shift_lfsr(rstate);
  return rstate;
}


////////////////////
/// MATH ROUTINES
/// help with manipulating pixels 
////////////////////

// saturating subtract. returns a-b, stopping at 0. assumes 8-bit types
uint8_t satsub_8( uint8_t a, uint8_t b ) {
  if( a >= b )
    return (a - b);
  else
    return 0;
}

// saturating add, returns a+b, stopping at 255. assumes 8-bit types
uint8_t satadd_8( uint8_t a, uint8_t b ) {
  uint16_t c = (uint16_t) a + (uint16_t) b;

  if( c > 255 )
    return (uint8_t) 255;
  else
    return (uint8_t) (c & 0xFF);
}

// saturating subtract, acting on a whole RGB pixel
Color satsub_8p( Color c, uint8_t val ) {
  Color rc;
  rc.r = satsub_8( c.r, val );
  rc.g = satsub_8( c.g, val );
  rc.b = satsub_8( c.b, val );

  return( rc );
}

// saturating add, acting on a whole RGB pixel
Color satadd_8p( Color c, uint8_t val ) {
  Color rc;
  rc.r = satadd_8( c.r, val );
  rc.g = satadd_8( c.g, val );
  rc.b = satadd_8( c.b, val );

  return( rc );
}

// alpha blend, scale the input color based on a value from 0-255. 255 is full-scale, 0 is black-out.
// uses fixed-point math.
Color alphaPix( Color c, uint8_t alpha ) {
  Color rc;
  uint32_t r, g, b;

  r = c.r * alpha;
  g = c.g * alpha;
  b = c.b * alpha;

  rc.r = (r / 255) & 0xFF;
  rc.g = (g / 255) & 0xFF;
  rc.b = (b / 255) & 0xFF;

  return( rc );  
}



static Color Wheel(uint8_t wheelPos) {
  Color c;

  if (wheelPos < 85) {
    c.r = wheelPos * 3;
    c.g = 255 - wheelPos * 3;
    c.b = 0;
  }
  else if (wheelPos < 170) {
    wheelPos -= 85;
    c.r = 255 - wheelPos * 3;
    c.g = 0;
    c.b = wheelPos * 3;
  }
  else {
    wheelPos -= 170;
    c.r = 0;
    c.g = wheelPos * 3;
    c.b = 255 - wheelPos * 3;
  }
  return c;
}

static void strobePatternFB(void *fb, int count, int loop) {
  uint16_t i;
  uint8_t oldshift = shift;
  
  shift = 0;

  for( i = 0; i < count; i++ ) {
    if( (rand() % (unsigned int) count) < ((unsigned int) count / 3) )
      ledSetRGB(fb, i, 255, 255, 255, shift);
    else
      ledSetRGB(fb, i, 0, 0, 0, shift);
  }

  chThdSleepMilliseconds(30 + (rand() % 25));

  for( i = 0; i < count; i++ ) {
    ledSetRGB(fb, i, 0, 0, 0, shift);
  }

  chThdSleepMilliseconds(30 + (rand() % 25));
  
  shift = oldshift;
}

static void calmPatternFB(void *fb, int count, int loop) {
  int i;
  int count_mask;
  Color c;

  count_mask = count & 0xff;
  loop = loop % (256 * 5);
  for (i = 0; i < count; i++) {
    c = Wheel( (i * (256 / count_mask) + loop) & 0xFF );
    ledSetRGB(fb, i, c.r, c.g, c.b, shift);
  }
}

static void testPatternFB(void *fb, int count, int loop) {
  int i = 0;

#if 0
  while (i < count) {
    /* Black */
    ledSetRGB(fb, (i + loop) % count, 0, 0, 0, shift);
    if (++i >= count) break;

    /* Red */
    ledSetRGB(fb, (i + loop) % count, 255, 0, 0, shift);
    if (++i >= count) break;

    /* Yellow */
    ledSetRGB(fb, (i + loop) % count, 255, 255, 0, shift);
    if (++i >= count) break;

    /* Green */
    ledSetRGB(fb, (i + loop) % count, 0, 255, 0, shift);
    if (++i >= count) break;

    /* Cyan */
    ledSetRGB(fb, (i + loop) % count, 0, 255, 255, shift);
    if (++i >= count) break;

    /* Blue */
    ledSetRGB(fb, (i + loop) % count, 0, 0, 255, shift);
    if (++i >= count) break;

    /* Purple */
    ledSetRGB(fb, (i + loop) % count, 255, 0, 255, shift);
    if (++i >= count) break;

    /* White */
    ledSetRGB(fb, (i + loop) % count, 255, 255, 255, shift);
    if (++i >= count) break;
  }
#endif
#if 0
  while (i < count) {
    if (loop & 1) {
      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* White */
      ledSetRGB(fb, (i++ + loop) % count, 32, 32, 32, shift);
    }
    else {
      /* White */
      ledSetRGB(fb, (i++ + loop) % count, 32, 32, 32, shift);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0, shift);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0, shift);
    }
  }
#endif
  //  int threshold = (phageAdcGet() * count / 4096);
  int threshold = (20 * count / 4096);   ////////// NOTE NOTE BODGE
  for (i = 0; i < count; i++) {
    if (i > threshold)
      ledSetRGB(fb, i, 255, 0, 0, shift);
    else
      ledSetRGB(fb, i, 0, 0, 0, shift);
  }
}

static void shootPatternFB(void *fb, int count, int loop) {
  int i;

  //loop = (loop >> 3) % count;
  loop = loop % count;
  for (i = 0; i < count; i++) {
    if (loop == i)
      ledSetRGB(fb, i, 255, 255, 255, shift);
    else
      ledSetRGB(fb, i, 0, 0, 0, shift);
  }
}

#define VU_X_PERIOD 3   // number of waves across the entire band
#define VU_T_PERIOD 2500  // time to complete 2pi rotation, in integer milliseconds
#define TAU 1000

static void waveRainbowFB(void *fb, int count, int loop) {
  unsigned long curtime;
  int i;
  uint32_t c;
  int sign = 1;
  uint16_t colorrate = 1;
  
  curtime = chVTGetSystemTime() + offset;
  if( (curtime - reftime) > VU_T_PERIOD )
    reftime = curtime;

  if( (curtime - reftime_tau) > TAU ) {
    reftime_tau = curtime;
    waverate -= 4;
    if( waverate < 10 )
      waverate = 10;
    
    if( colorrate > 1 )
      colorrate -= 1;
  }

  if( bumped ) {
    bumped = 0;
    waverate += 20;
    colorrate += 1;
    if( waverate > 300 )
      waverate = 300;
    if( colorrate > 10 )
      colorrate = 10;
  }

  offset += waverate;
  if( offset > 0x80000000) {
    offset = 0;
    curtime = chVTGetSystemTime();
    reftime = curtime;
    reftime_tau = curtime;
  }

  waveloop += colorrate;
  if( waveloop == (256 * 5) ) {
    waveloop = 0;
  }
  for( i = 0; i < count; i++ ) {
    c = (uint8_t) (sinLUT[ (((i * VU_X_PERIOD * 255) / (count - 1))  +
			    ( (sign * 255 * (curtime - reftime)) / VU_T_PERIOD ) ) % 256]);
    
    c = c * c;
    c = (c >> 8) & 0xFF;
    ledSetColor(fb, i, alphaPix(Wheel(((i * 256 / count) + waveloop) & 255), (uint8_t) c), shift);
  }  
}

static void directedRainbowFB(void *fb, int count, int loop) {
  unsigned long curtime;
  uint32_t i;
  uint32_t c;
  uint32_t colorrate = 1;
  
  curtime = chVTGetSystemTime() + offset;
  if( (curtime - reftime) > VU_T_PERIOD )
    reftime = curtime;

  waverate = 80;
  colorrate = 1;

  if( bumped ) {
    bumped = 0;
    if( wavesign == 1 )
      wavesign = -1;
    else
      wavesign = 1;
  }

  offset += waverate;
  if( offset > 0x80000000) {
    offset = 0;
    curtime = chVTGetSystemTime();
    reftime = curtime;
    reftime_tau = curtime;
  }

  waveloop += colorrate;
  if( waveloop == (256 * 5) ) {
    waveloop = 0;
  }
  for( i = 0; i < (uint32_t) count; i++ ) {
    c = (uint8_t) (sinLUT[ ((((int)i * VU_X_PERIOD * 255) / ((int)count - 1))  +
			    ( (wavesign * 255 * ((int)curtime - (int)reftime)) / VU_T_PERIOD ) ) & 0xFF]);
    
    c = c * c;
    c = (c >> 8) & 0xFF;
    ledSetColor(fb, i, alphaPix(Wheel(((i * 256 / count) + waveloop) & 255), (uint8_t) c), shift);
  }  
}

static uint32_t asb_l(int i) {
  if (i > 0)
      return i;
  return -i;
}

#define DROP_INT 600
#define BUMP_TIMEOUT 2300
static void raindropFB(void *fb, int count, int loop) {
  unsigned long curtime;
  uint8_t oldshift = shift;
  uint8_t myshift;
  Color c;
  int i;
  
  if(patternChanged) {
    patternChanged = 0;
    for( i = 0; i < count; i++ ) {
      c.r = 0; c.g = 0; c.b = 0;
      ledSetColor(fb, i, c, shift);
    }
    bumptime = bumptime - BUMP_TIMEOUT;
  }

  myshift = shift;
  if( myshift > 3 )  // limit dimness as this is a sparse pattern
    myshift = 3;

  shift = 0;

  curtime = chVTGetSystemTime();
  if( ((curtime - reftime) > DROP_INT) && (curtime - bumptime > BUMP_TIMEOUT) ) {
    reftime = curtime;
    c.r = 255 >> myshift; c.g = 255 >> myshift; c.b = 255 >> myshift;
  } else {
    c.r = 0; c.g = 0; c.b = 0;
  }

  if( bumped ) {
    bumped = 0;
    c.r = 255 >> myshift; c.g = 255 >> myshift; c.b = 255 >> myshift;
  }

  ledSetColor(fb, 0, c, shift);

  for( i = count-2; i >= 0; i-- ) {
    c = ledGetColor(fb, i);
    ledSetColor(fb, i+1, c, shift);
  }

  shift = oldshift;
}

static void rainbowDropFB(void *fb, int count, int loop) {
  unsigned long curtime;
  uint8_t oldshift = shift;
  uint8_t myshift;
  Color c;
  Color c2;
  int i;
  
  if(patternChanged) {
    patternChanged = 0;
    for( i = 0; i < count; i++ ) {
      c.r = 0; c.g = 0; c.b = 0;
      ledSetColor(fb, i, c, shift);
    }
    bumptime = bumptime - BUMP_TIMEOUT;
  }
  c2.r = 0; c2.g = 0; c2.b = 0;

  myshift = shift;
  if( myshift > 3 )  // limit dimness as this is a sparse pattern
    myshift = 3;

  shift = 0;

  loop = loop % (256 * 5);

  curtime = chVTGetSystemTime();
  if( ((curtime - reftime) > DROP_INT) && (curtime - bumptime > BUMP_TIMEOUT) ) {
    c = Wheel(loop);
    c2 = Wheel(loop + 1);
    reftime = curtime;
  } else {
    c.r = 0; c.g = 0; c.b = 0;
    c2.r = 0; c2.g = 0; c2.b = 0;
  }

  if( bumped ) {
    bumped = 0;
    c = Wheel(loop);
    c2 = Wheel(loop + 1);
  }

  ledSetColor(fb, 0, c, shift);

  for( i = count-2; i >= 0; i-- ) {
    c2 = ledGetColor(fb, i);
    ledSetColor(fb, i+1, c2, shift);
  }

  shift = oldshift;
}


static void larsonScannerFB(void *fb, int count, int loop) {
  int i;
  int dir;

  loop %= (count * 2);

  if (loop >= count)
    dir = 1;
  else
    dir = 0;

  loop %= count;

  for (i = 0; i < count; i++) {
    uint32_t x = i;

    if (dir)
      x = count - i - 1;

    /* LED going out */
    if (asb_l(i - loop) == 2)
      ledSetRGBClipped(fb, x, 1, 0, 0, shift);
    else if (asb_l(i - loop) == 1)
      ledSetRGBClipped(fb, x, 20, 0, 0, shift);
    else if (asb_l(i - loop) == 0)
      ledSetRGBClipped(fb, x, 255, 0, 0, shift);
    else
      ledSetRGBClipped(fb, x, 0, 0, 0, shift);
  }

}

#define BUMP_DEBOUNCE 300 // 300ms debounce to next bump

void bump(uint32_t amount) {
  bump_amount = amount;
  if( chVTGetSystemTime() - bumptime > BUMP_DEBOUNCE ) {
    bumptime = chVTGetSystemTime();
    bumped = 1;
  }
}

static int draw_pattern(struct effects_config *config) {
    config->loop++;

    if( bump_amount != 0 ) {
      config->loop += bump_amount;
      bump_amount = 0;
    }

    //    if (config->pattern == patternShoot)
    //      shootPatternFB(config->fb, config->count, config->loop);
    if (config->pattern == patternCalm) {
      calmPatternFB(config->fb, config->count, config->loop);
      config->loop += 2; // make this one go faster
    } else if (config->pattern == patternTest)
      testPatternFB(config->fb, config->count, config->loop);
    else if (config->pattern == patternStrobe)
      strobePatternFB(config->fb, config->count, config->loop);
    else if( config->pattern == patternWaveRainbow )
      waveRainbowFB(config->fb, config->count, config->loop);
    else if( config->pattern == patternDirectedRainbow )
      directedRainbowFB(config->fb, config->count, config->loop);
    else if( config->pattern == patternRaindrop ) {
      raindropFB(config->fb, config->count, config->loop);
    } else if( config->pattern == patternRainbowdrop ) {
      rainbowDropFB(config->fb, config->count, config->loop);
    } else {
      testPatternFB(config->fb, config->count, config->loop);
    }

    return 0;
}

static struct effects_config g_config;
void effectsSetPattern(enum pattern pattern) {
  g_config.pattern = pattern;
}

enum pattern effectsGetPattern(void) {
  return g_config.pattern;
}

void effectsNextPattern(void) {
  g_config.pattern = g_config.pattern + 1;
  g_config.pattern = g_config.pattern % patternLast;
  patternChanged = 1;
}

void effectsPrevPattern(void) {
  if( g_config.pattern == 0 ) {
    g_config.pattern = patternLast - 1;
  } else {
    g_config.pattern = g_config.pattern - 1;
  }
  patternChanged = 1;
}


static THD_WORKING_AREA(waEffectsThread, 256);
static msg_t effects_thread(void *arg) {

  chRegSetThreadName("effects");

  while (1) {
    chSysLock();
    ledUpdate(led_config.fb, led_config.pixel_count);
    chSysUnlock();
    chThdYield();
    chThdSleepMilliseconds(EFFECTS_REDRAW_MS);
    effectsDraw();
  }
  return MSG_OK;
}

void effectsDraw(void) {
  draw_pattern(&g_config);
}

void effectsStart(void) {

  g_config.fb = led_config.fb;
  g_config.count = led_config.pixel_count;
  g_config.loop = 0;
  g_config.pattern = patternWaveRainbow;

  draw_pattern(&g_config);
  chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread),
      NORMALPRIO - 6, effects_thread, &g_config);
}
