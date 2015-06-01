/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "orchard-app.h"  // has the mutex for gfx

#include "gfx.h"
#include "led.h"
#include "accel.h"
#include "captouch.h"

#define TILT_THRESH 100
#define TILT_RATE   128
#define BALL_SIZE  6

static int should_stop(void) {
  uint8_t bfr[1];
  return chnReadTimeout(serialDriver, bfr, sizeof(bfr), 1);
}

void cmd_oled(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;
  struct accel_data d, dref;
  coord_t x = 60, y = 28;
  coord_t xo, yo;
  uint8_t changed = 0;
  coord_t width, height;
  uint16_t val;

  osalMutexLock(&orchard_gfxMutex);
  width = gdispGetWidth();
  height = gdispGetHeight();
  osalMutexUnlock(&orchard_gfxMutex);

  accelPoll(&dref);  // seed accelerometer values
  dref.x = (dref.x + 2048) & 0xFFF;
  dref.y = (dref.y + 2048) & 0xFFF;

  // x values go up as you tilt to the right
  // y vaules go up as you tilt toward the bottom
  
  chprintf(chp, "\r\nPress any key to quit\r\n");
  while (!should_stop()) {
    val = captouchRead();
    if( val & 0x20 ) { // detect if center pad is hit
      accelPoll(&dref);
      dref.x = (dref.x + 2048) & 0xFFF;
      dref.y = (dref.y + 2048) & 0xFFF;
    }
    accelPoll(&d);
    d.x = (d.x + 2048) & 0xFFF;
    d.y = (d.y + 2048) & 0xFFF;

    xo = x; yo = y;
    if( d.x > (dref.x + TILT_THRESH) ) {
      x += (d.x - dref.x) / TILT_RATE;
      changed = 1;
    } else if( d.x < (dref.x - TILT_THRESH) ) {
      x -= (dref.x -d.x) / TILT_RATE;
      changed = 1;
    }
    if( x > (width - BALL_SIZE))
      x = width - BALL_SIZE;
    if( x < BALL_SIZE )
      x = BALL_SIZE;
    
    if( d.y > (dref.y + TILT_THRESH) ) {
      y += (d.y - dref.y) / TILT_RATE;
      changed = 1;
    } else if( d.y < (dref.y - TILT_THRESH) ) {
      y -= (dref.y -d.y) / TILT_RATE;
      changed = 1;
    }
    if( y > (height - BALL_SIZE))
      y = height - BALL_SIZE;
    if( y < BALL_SIZE )
      y = BALL_SIZE;
    
    if( changed ) {
      changed  = 0;
      osalMutexLock(&orchard_gfxMutex);
      gdispFillCircle(xo, yo, BALL_SIZE, Black);
      osalMutexUnlock(&orchard_gfxMutex);
    }
    
    osalMutexLock(&orchard_gfxMutex);
    gdispFillCircle(x, y, BALL_SIZE, White);
    gdispFlush();
    osalMutexUnlock(&orchard_gfxMutex);
  }
  chprintf(chp, "\r\n");

}

orchard_command("oled", cmd_oled);
