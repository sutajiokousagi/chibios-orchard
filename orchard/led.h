#ifndef __LED_H__
#define __LED_H__

#include "hal.h"

enum pattern {
  patternCalm = 0,
  patternStrobe,
  patternRaindrop,
  patternRainbowdrop,
  patternWaveRainbow,
  patternTest,
  patternDirectedRainbow,
  patternLast
};

#define sign(x) (( x > 0 ) - ( x < 0 ))

typedef struct Color Color;
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

void ledStart(uint32_t leds, uint8_t *o_fb);

void effectsStart(void *_fb, int _count);
void effectsDraw(void);
void effectsSetPattern(enum pattern pattern);
enum pattern effectsGetPattern(void);
void bump(uint32_t amount);
void setShift(uint8_t s);
uint8_t getShift(void);
void effectsNextPattern(void);
void effectsPrevPattern(void);

#define EFFECTS_REDRAW_MS 35

void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
void ledSetColor(void *ptr, int x, Color c, uint8_t shift);
void ledSetRGBClipped(void *fb, uint32_t i,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t shift);
Color ledGetColor(void *ptr, int x);
void ledSetCount(uint32_t count);

void ledUpdate(uint8_t *fb, uint32_t len);
void ledStart(uint32_t leds, uint8_t *o_fb);

#endif /* __LED_H__ */
