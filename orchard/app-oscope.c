#include "orchard-app.h"
#include "orchard-ui.h"

#include "analog.h"

#include <string.h>
#include <stdlib.h>

static void redraw_ui(uint8_t *samples) {
  coord_t width;
  coord_t height;
  uint8_t i;

  width = gdispGetWidth();
  height = gdispGetHeight();

  gdispClear(Black);

  for( i = 0; i < MIC_SAMPLE_DEPTH; i++ ) {
    if( samples != NULL )
      gdispDrawPixel((coord_t)i, (coord_t) (255 - samples[i]) / (256 / height), White);
    else
      gdispDrawPixel((coord_t)i, (coord_t) 32, White);
  }

  gdispFlush();
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
    }
  } else if( event->type == adcEvent) {
    if( event->adc.code == adcCodeMic ) {
      samples = analogReadMic();
    }
    redraw_ui(samples);

    analogUpdateMic();
  }
}

static void oscope_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Oscilloscope", oscope_init, oscope_start, oscope_event, oscope_exit);


