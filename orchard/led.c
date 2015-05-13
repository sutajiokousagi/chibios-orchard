#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "led.h"
#include "orchard-math.h"
#include "fixmath.h"

#include <math.h>

extern void ledUpdate(uint8_t *fb, uint32_t len);

static void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
static void ledSetColor(void *ptr, int x, Color c, uint8_t shift);
static void ledSetRGBClipped(void *fb, uint32_t i,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
static Color ledGetColor(void *ptr, int x);
static void ledSetCount(uint32_t count);


// hardware configuration information
// max length is different from actual length because some
// pattens may want to support the option of user-added LED
// strips, whereas others will focus only on UI elements in the
// circle provided on the board itself
static struct {
  uint8_t       *fb; // effects frame buffer
  uint8_t       *final_fb;  // merged ui + effects frame buffer
  uint32_t      pixel_count;  // generated pixel length
  uint32_t      max_pixels;   // maximal generation length
  uint8_t       *ui_fb; // frame buffer for UI effects
  uint32_t      ui_pixels;  // number of LEDs on the PCB itself for UI use
} led_config;

// local effects state
struct effects_config {
  uint32_t count;
  uint32_t loop;
  enum pattern pattern;
};
static struct effects_config g_config;

static uint8_t shift = 4;  // start a little bit dimmer

static uint32_t bump_amount = 0;
static uint8_t bumped = 0;
static unsigned int bumptime = 0;
static unsigned long reftime = 0;
static unsigned long reftime_tau = 0;
static unsigned long offset = 0;
static unsigned int waverate = 10;
static unsigned int waveloop = 0;
static unsigned int patternChanged = 0;

static int wavesign = -1;

/**
 * @brief   Initialize Led Driver
 * @details Initialize the Led Driver based on parameters.
 *
 * @param[in] leds      length of the LED chain controlled by each pin
 * @param[out] o_fb     initialized frame buffer
 *
 */
void ledStart(uint32_t leds, uint8_t *o_fb, uint32_t ui_leds, uint8_t *o_ui_fb)
{
  unsigned int j;
  led_config.max_pixels = leds;
  led_config.pixel_count = leds;
  led_config.ui_pixels = ui_leds;

  led_config.fb = o_fb;
  led_config.ui_fb = o_ui_fb;

  led_config.final_fb = chHeapAlloc( NULL, sizeof(uint8_t) * led_config.max_pixels * 3 );
  
  for (j = 0; j < leds * 3; j++)
    led_config.fb[j] = 0x0;
  for (j = 0; j < ui_leds * 3; j++)
    led_config.ui_fb[j] = 0x0;

  chSysLock();
  ledUpdate(led_config.fb, led_config.max_pixels);
  chSysUnlock();
}

void uiLedGet(uint8_t index, Color *c) {
  if( index >= led_config.ui_pixels )
    index = led_config.ui_pixels - 1;
  
  c->g = led_config.ui_fb[index*3];
  c->r = led_config.ui_fb[index*3+1];
  c->b = led_config.ui_fb[index*3+2];
}

void uiLedSet(uint8_t index, Color c) {
  if( index >= led_config.ui_pixels )
    index = led_config.ui_pixels - 1;
  
  led_config.ui_fb[index*3] = c.g;
  led_config.ui_fb[index*3+1] = c.r;
  led_config.ui_fb[index*3+2] = c.b;
}

static void ledSetRGBClipped(void *fb, uint32_t i,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
  if (i >= led_config.pixel_count)
    return;
  ledSetRGB(fb, i, r, g, b, shift);
}

static void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift) {
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = g >> shift;
  buf[1] = r >> shift;
  buf[2] = b >> shift;
}

static void ledSetColor(void *ptr, int x, Color c, uint8_t shift) {
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = c.g >> shift;
  buf[1] = c.r >> shift;
  buf[2] = c.b >> shift;
}

static Color ledGetColor(void *ptr, int x) {
  Color c;
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);

  c.g = buf[0];
  c.r = buf[1];
  c.b = buf[2];
  
  return c;
}

static void ledSetCount(uint32_t count) {
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

#include "chprintf.h"
#include "orchard.h"
static void waveRainbowFB(void *fb, int count, int loop) {
  unsigned long curtime;
  int i;
  uint32_t c;
  int sign = 1;
  uint16_t colorrate = 1;
  
  curtime = chVTGetSystemTime() + offset;
  if ((curtime - reftime) > VU_T_PERIOD)
    reftime = curtime;

  if ((curtime - reftime_tau) > TAU) {
    reftime_tau = curtime;
    waverate -= 4;
    if (waverate < 10)
      waverate = 10;
    
    if (colorrate > 1)
      colorrate -= 1;
  }

  if (bumped) {
    bumped = 0;
    waverate += 20;
    colorrate += 1;
    if (waverate > 300)
      waverate = 300;
    if (colorrate > 10)
      colorrate = 10;
  }

  offset += waverate;
  if (offset > 0x80000000) {
    offset = 0;
    curtime = chVTGetSystemTime();
    reftime = curtime;
    reftime_tau = curtime;
  }

  waveloop += colorrate;
  if (waveloop == (256 * 5)) {
    waveloop = 0;
  }
  for (i = 0; i < count; i++) {
    fix16_t count_n = fix16_from_int(i * VU_X_PERIOD);
    fix16_t count_d = fix16_from_int(count - 1);
    fix16_t time_n = fix16_from_int(curtime - reftime);
    fix16_t time_d = fix16_from_int(VU_T_PERIOD);
    fix16_t ratios = fix16_add(
                fix16_div(count_n, count_d), fix16_div(time_n, time_d));

    /* 'ratios' now goes from 0 to 2, depending on where we are in the cycle */

    ratios = fix16_mul(ratios, fix16_from_int(2));
    ratios = fix16_mul(ratios, fix16_pi);
    fix16_t v = fix16_sin(ratios);

    /* Normalize, go from [-1, 1] to [0, 256] */
    v = fix16_mul(fix16_add(v, fix16_from_int(1)), fix16_from_int(127));

    c = fix16_to_int(v);

    /* Quick and dirty nonlinearity */
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
    fix16_t count_n = fix16_from_int(i * VU_X_PERIOD);
    fix16_t count_d = fix16_from_int(count - 1);
    fix16_t time_n = fix16_from_int((curtime - reftime) * wavesign);
    fix16_t time_d = fix16_from_int(VU_T_PERIOD);
    fix16_t ratios = fix16_add(
                fix16_div(count_n, count_d), fix16_div(time_n, time_d));

    /* 'ratios' now goes from 0 to 2, depending on where we are in the cycle */

    ratios = fix16_mul(ratios, fix16_from_int(2));
    ratios = fix16_mul(ratios, fix16_pi);
    fix16_t v = fix16_sin(ratios);

    /* Normalize, go from [-1, 1] to [0, 256] */
    v = fix16_mul(fix16_add(v, fix16_from_int(1)), fix16_from_int(127));

    c = fix16_to_int(v);
    
    /* Quick and dirty nonlinearity */
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
    //      shootPatternFB(led_config.fb, config->count, config->loop);
    if (config->pattern == patternCalm) {
      calmPatternFB(led_config.fb, config->count, config->loop);
      config->loop += 2; // make this one go faster
    } else if (config->pattern == patternTest)
      testPatternFB(led_config.fb, config->count, config->loop);
    else if (config->pattern == patternStrobe)
      strobePatternFB(led_config.fb, config->count, config->loop);
    else if( config->pattern == patternWaveRainbow )
      waveRainbowFB(led_config.fb, config->count, config->loop);
    else if( config->pattern == patternDirectedRainbow )
      directedRainbowFB(led_config.fb, config->count, config->loop);
    else if( config->pattern == patternRaindrop ) {
      raindropFB(led_config.fb, config->count, config->loop);
    } else if( config->pattern == patternRainbowdrop ) {
      rainbowDropFB(led_config.fb, config->count, config->loop);
    } else {
      testPatternFB(led_config.fb, config->count, config->loop);
    }

    return 0;
}

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

static void blendFbs() {
  uint8_t i;
  // UI FB + effects FB blend (just do a saturating add)
  for( i = 0; i < led_config.ui_pixels * 3; i ++ ) {
    led_config.final_fb[i] = satadd_8(led_config.fb[i], led_config.ui_fb[i]);
  }

  // copy over the remainder of the effects FB that extends beyond UI FB
  for( i = led_config.ui_pixels * 3; i < led_config.max_pixels * 3; i++ ) {
    led_config.final_fb[i] = led_config.fb[i];
  }
}

static THD_WORKING_AREA(waEffectsThread, 256);
static THD_FUNCTION(effects_thread, arg) {

  (void)arg;
  chRegSetThreadName("LED effects");

  while (1) {
    blendFbs();
    
    // transmit the actual framebuffer to the LED chain
    chSysLock();
    ledUpdate(led_config.final_fb, led_config.pixel_count);
    chSysUnlock();

    // wait until the next update cycle
    chThdYield();
    chThdSleepMilliseconds(EFFECTS_REDRAW_MS);

    // re-render the internal framebuffer animations
    draw_pattern(&g_config);
  }
  return;
}

void effectsStart(void) {

  g_config.count = led_config.pixel_count;
  g_config.loop = 0;
  g_config.pattern = patternWaveRainbow;

  draw_pattern(&g_config);
  chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread),
      NORMALPRIO - 6, effects_thread, &g_config);
}
