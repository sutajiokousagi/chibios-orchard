/*
 * Copyright (c) 2015 Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2015 Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This program was originally contributed by community member "Fleck" as
 * the winning entry in the December 2014 demo competition.
 *
 * Some minor changes have been made by the uGFX maintainers.
 */

#include "gfx.h"
#include "stdlib.h"
#include "string.h"
#include "orchard-app.h"
#include "fixmath.h"

#define ROTATE_AMOUNT 15

#define BOARD_TOP_X 15
#define BOARD_TOP_Y 15
#define BOARD_BOTTOM_X (gdispGetWidth() - 15)
#define BOARD_BOTTOM_Y (gdispGetHeight() - 15)

static const int ship[] = {
   4,  0,
  -4, -3,
  -3,  0,
  -4,  3,
};

static const int jet[] = {
  -4, -2,
  -7,  0,
  -4,  2,
};

struct pew_context {
  int             x;
  int             y;
  int             rotation;
  fix16_t         x_vel;
  fix16_t         y_vel;
  fix16_t         rotation_f;
  int             max_x;
  int             min_x;
  int             max_y;
  int             min_y;
  int             width;
  int             height;
  unsigned long   score;
  unsigned long   game_speed;
  bool            engines;
  font_t          font16;
  font_t          font12;
};

static int rotx(int x, int y, fix16_t fr) {
  fix16_t fx = fix16_from_int(x);
  fix16_t fy = fix16_from_int(y);

  return fix16_to_int(fix16_sub(
        fix16_mul(fx, fix16_cos(fr)),
        fix16_mul(fy, fix16_sin(fr))));
}

static int roty(int x, int y, fix16_t fr) {
  fix16_t fx = fix16_from_int(x);
  fix16_t fy = fix16_from_int(y);

  return fix16_to_int(fix16_add(
        fix16_mul(fx, fix16_sin(fr)),
        fix16_mul(fy, fix16_cos(fr))));
}

static void pew_draw_ship(struct pew_context *pew) {
  int px;
  int py;
  int cx;
  int cy;
  unsigned int i;

  // Calculate the actual draw point by rotating and then translating.
  px = rotx(ship[6], ship[7], pew->rotation_f) + pew->x;
  py = roty(ship[6], ship[7], pew->rotation_f) + pew->y;
         
  // Loop through the other points---note that this therefore begins at point 1, not 0!
  for (i = 0; i < ARRAY_SIZE(ship); i += 2) {
    cx = rotx(ship[i], ship[i + 1], pew->rotation_f) + pew->x;
    cy = roty(ship[i], ship[i + 1], pew->rotation_f) + pew->y;

    gdispDrawLine(px + 5, py + 5, cx + 5, cy + 5, White);

    px = cx;
    py = cy;
  }

  if (pew->engines) {
    // Calculate the actual draw point by rotating and then translating.
    px = rotx(jet[4], jet[5], pew->rotation_f) + pew->x;
    py = roty(jet[4], jet[5], pew->rotation_f) + pew->y;
           
    // Loop through the other points---note that this therefore begins at point 1, not 0!
    for (i = 0; i < ARRAY_SIZE(jet); i += 2) {
      cx = rotx(jet[i], jet[i + 1], pew->rotation_f) + pew->x;
      cy = roty(jet[i], jet[i + 1], pew->rotation_f) + pew->y;

      gdispDrawLine(px + 5, py + 5, cx + 5, cy + 5, White);

      px = cx;
      py = cy;
    }
  }
}

static void pew_redraw(struct pew_context *pew){

  gdispClear(Black);
  gdispDrawBox(pew->min_x,
               pew->min_y,
               pew->max_x,
               pew->max_y,
               White);
  pew_draw_ship(pew);
  gdispDrawString(20,
                  (pew->min_y + pew->max_y) / 2,
                  "PEW",
                  pew->font16,
                  White);
  gdispFlush();
}

static void pew_tick(struct pew_context *pew) {

  fix16_t stepsize = fix16_div(fix16_from_int(20), fix16_from_int(1000));

  if (pew->engines) {
    fix16_t scale = fix16_from_int(4);
    pew->x_vel = 
      fix16_add(pew->x_vel, fix16_mul(fix16_cos(pew->rotation_f), scale));
    pew->y_vel = 
      fix16_add(pew->y_vel, fix16_mul(fix16_sin(pew->rotation_f), scale));
  }

  pew->x += fix16_to_int(fix16_mul(pew->x_vel, stepsize));
  pew->y += fix16_to_int(fix16_mul(pew->y_vel, stepsize));

  while (pew->x > pew->max_x)
    pew->x -= pew->width;

  while (pew->x < pew->min_x)
    pew->x += pew->width;

  while (pew->y > pew->max_y)
    pew->y -= pew->height;

  while (pew->y < pew->min_y)
    pew->y += pew->height;

  pew_redraw(pew);
}

static uint32_t pew_init(OrchardAppContext *context) {
  (void)context;
  return sizeof(struct pew_context);
}

static void pew_start(OrchardAppContext *context) {
  struct pew_context *pew = context->priv;

  pew->font16 = gdispOpenFont("DejaVuSans16");
  pew->font12 = gdispOpenFont("DejaVuSans12");

  pew->min_x = BOARD_TOP_X;
  pew->max_x = BOARD_BOTTOM_X;
  pew->min_y = BOARD_TOP_Y;
  pew->max_y = BOARD_BOTTOM_Y;

  pew->width = pew->max_x - pew->min_x;
  pew->height = pew->max_y - pew->min_y;

  pew->x = (pew->width / 2) + pew->min_x;
  pew->y = (pew->height / 2) + pew->min_y;

  pew->game_speed = 20 * 1000;
  pew->engines = false;
  pew->x_vel = 0;
  pew->y_vel = 0;

  pew_redraw(pew);

  orchardAppTimer(context, pew->game_speed, true);
}

static void pew_exit(OrchardAppContext *context) {
  struct pew_context *pew = context->priv;

  gdispCloseFont(pew->font16);
  gdispCloseFont(pew->font12);
}

static void pew_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  struct pew_context *pew = context->priv;

  if (event->type == timerEvent) {
    pew_tick(pew);
  }
  else if (event->type == keyEvent) {
    if (event->key.code == keyCW) {
      pew->rotation += ROTATE_AMOUNT;
      while (pew->rotation > 360)
        pew->rotation -= 360;
      pew->rotation_f = fix16_deg_to_rad(fix16_from_int(pew->rotation));
    }
    else if (event->key.code == keyCCW) {
      pew->rotation -= ROTATE_AMOUNT;
      while (pew->rotation < 0)
        pew->rotation += 360;
      pew->rotation_f = fix16_deg_to_rad(fix16_from_int(pew->rotation));
    }

    else if (event->key.code == keyLeft) {
      if (event->key.flags == keyDown)
        pew->engines = true;
      else
        pew->engines = false;
    }
  }

}

orchard_app("Pew", pew_init, pew_start, pew_event, pew_exit);
