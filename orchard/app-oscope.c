#include "orchard-app.h"
#include "orchard-ui.h"

#include "analog.h"

#include <string.h>
#include <stdlib.h>

#include "fixmath.h"
#include "fix16_fft.h"

static int mode = 0;

static void agc(uint8_t  *sample) {
  uint8_t min, max;
  uint8_t scale = 1;
  int16_t temp;
  uint8_t i;

  // cheesy AGC to get something on the screen even if it's pretty quiet
  // e.g., "comfort noise"
  // note abstraction violation: we're going to hard-code this for a screen height of 64
  if( sample == NULL )
    return;

  min = 255; max = 0;
  for( i = 0; i < MIC_SAMPLE_DEPTH; i++ ) {
    if( sample[i] > max )
      max = sample[i];
    if( sample[i] < min )
      min = sample[i];
  }

  if( (max - min) < 128 )
    scale = 2;
  if( (max - min) < 64 )
    scale = 4;
  if( (max - min) < 32 )
    scale = 8;
  if( (max - min) < 16 )
    scale = 16;
  
  for( i = 0; i < MIC_SAMPLE_DEPTH; i++ ) {
    temp = sample[i];
    temp -= 128;
    temp *= scale;
    temp += 128;
    sample[i] = (uint8_t) temp;
  }
}

static void agc_fft(uint8_t  *sample) {
  uint8_t min, max;
  uint8_t scale = 1;
  int16_t temp;
  uint8_t i;

  // cheesy AGC to get something on the screen even if it's pretty quiet
  // e.g., "comfort noise"
  // note abstraction violation: we're going to hard-code this for a screen height of 64
  if( sample == NULL )
    return;

  min = 255; max = 0;
  for( i = 2; i < MIC_SAMPLE_DEPTH; i++ ) {
    if( sample[i] > max )
      max = sample[i];
    if( sample[i] < min )
      min = sample[i];
  }

  if( (max - min) < 128 )
    scale = 2;
  if( (max - min) < 64 )
    scale = 4;
  if( (max - min) < 32 )
    scale = 8;
  if( (max - min) < 16 )
    scale = 16;
  
  for( i = 0; i < MIC_SAMPLE_DEPTH; i++ ) {
    temp = sample[i];
    temp *= scale;
    sample[i] = (uint8_t) temp;
  }
}

static void redraw_ui(uint8_t *samples) {
  coord_t height;
  uint8_t i;
  uint8_t scale;
  fix16_t real[MIC_SAMPLE_DEPTH];
  fix16_t imag[MIC_SAMPLE_DEPTH];

  agc( samples );
  
  if ( mode ) {
    fix16_fft(samples, real, imag, MIC_SAMPLE_DEPTH);
    for( i = 0; i < MIC_SAMPLE_DEPTH; i++ ) {
      samples[i] = fix16_to_int( fix16_sqrt(fix16_sadd(fix16_mul(real[i],real[i]),
						       fix16_mul(imag[i],imag[i]))) );
    }
    
    agc_fft(samples);
  }
  
  orchardGfxStart();
  height = gdispGetHeight();
  scale = 256 / height;

  gdispClear(Black);

  for( i = 1; i < MIC_SAMPLE_DEPTH; i++ ) {
    if( samples != NULL ) {
      // below for dots, change starting index to i=0
      //      gdispDrawPixel((coord_t)i, (coord_t) (255 - samples[i]) / scale , White);
      gdispDrawLine((coord_t)i-1, (coord_t) (255 - samples[i-1]) / scale, 
		    (coord_t)i, (coord_t) (255 - samples[i]) / scale, 
		    White);
    } else
      gdispDrawPixel((coord_t)i, (coord_t) 32, White);
  }

  gdispFlush();
  orchardGfxEnd();
}

static uint32_t oscope_init(OrchardAppContext *context) {
  (void)context;

  return 0;
}

static void oscope_start(OrchardAppContext *context) {
  (void)context;

  redraw_ui(NULL);
  analogUpdateMic();
}

void oscope_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  uint8_t *samples;
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keyLeft) ) {
      orchardAppExit();
    } else  if ( (event->key.flags == keyDown) && (event->key.code == keySelect) ) {
      mode = !mode;
    }
  } else if( event->type == adcEvent) {
    if( event->adc.code == adcCodeMic ) {
      samples = analogReadMic();
      redraw_ui(samples);
    }
    analogUpdateMic();
  }
}

static void oscope_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Sound scope", oscope_init, oscope_start, oscope_event, oscope_exit);


