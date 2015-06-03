#include "fixmath.h"
#include "orchard-app.h"

int rand(void);

#define FRAME_RATE 20 * 1000 /* 20 ms */
#define STAR_COUNT 32

struct star {
  fix16_t x;
  fix16_t y;
  fix16_t z;
  fix16_t s;
};

struct starfield_context {
  fix16_t  min;
  fix16_t  max;
  fix16_t  zscale;
  fix16_t  zmin;
  fix16_t  zmax;
  fix16_t  smin;
  fix16_t  smax;
  uint32_t midx;
  uint32_t midy;
  uint32_t width;
  uint32_t height;
  struct   star stars[STAR_COUNT];
};

static uint32_t sf_init(OrchardAppContext *ctx) {

  (void)ctx;
  return sizeof(struct starfield_context);
}

static fix16_t randrange(fix16_t min, fix16_t max) {

  fix16_t val = ((rand() << 7) ^ rand()) & 0xffff;
  fix16_t range = fix16_sub(max, min);
  return fix16_add(min, fix16_mul(val, range));
}

static void regen_star(struct starfield_context *sf, int i) {
  sf->stars[i].x = randrange(sf->min, sf->max);
  sf->stars[i].y = randrange(sf->min, sf->max);
  sf->stars[i].z = randrange(sf->zmin, sf->zmax);
  sf->stars[i].s = randrange(sf->smin, sf->smax);
}

static void sf_setup(OrchardAppContext *ctx) {

  struct starfield_context *sf = ctx->priv;
  int i;

  sf->min    = fix16_from_int(-250);
  sf->max    = fix16_from_int(250);
  sf->zmin   = fix16_from_int(0);
  sf->zmax   = fix16_from_int(4);
  sf->smin   = fix16_from_int(1);
  sf->smax   = fix16_from_int(3);
  sf->zscale = fix16_from_int(128);
  sf->midx   = gdispGetWidth() / 2;
  sf->midy   = gdispGetHeight() / 2;
  sf->width  = gdispGetWidth();
  sf->height = gdispGetHeight();

  for (i = 0; i < STAR_COUNT; i++)
    regen_star(sf, i);

  orchardAppTimer(ctx, FRAME_RATE, true);
}

static void sf_event(OrchardAppContext *ctx, const OrchardAppEvent *event) {

  struct starfield_context *sf = ctx->priv;

  if (event->type == timerEvent) {
    int i;
    uint32_t starx, stary;

    orchardGfxStart();
    gdispClear(Black);

    for (i = 0; i < STAR_COUNT; i++) {

      struct star *star = &sf->stars[i];

      /* Move the star */
      star->z = fix16_add(star->z, star->s);
	  
      if (fix16_to_int(star->z) >= 32)
	      regen_star(sf, i);

      /* Project 3D to 2D */
      fix16_t k = fix16_div(sf->zscale, star->z);
      starx = fix16_to_int(fix16_div(star->x, k)) + sf->midx;
      stary = fix16_to_int(fix16_div(star->y, k)) + sf->midy;

      if ((starx > sf->width) || (stary > sf->height))
	      regen_star(sf, i);
      else
        gdispDrawPixel(starx, stary, White);
    }

    gdispFlush();
    orchardGfxEnd();
  }
}

static void sf_exit(OrchardAppContext *ctx) {

  (void)ctx;
}

orchard_app("Starfield", sf_init, sf_setup, sf_event, sf_exit);
