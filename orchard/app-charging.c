#include "orchard-app.h"
#include "orchard-ui.h"

#include "analog.h"

#include <string.h>
#include <stdlib.h>

static int32_t celcius;

static void redraw_ui(uint8_t firsttime) {
  char tmp[] = "Charger status";
  char celciusStr[32];
  
  coord_t width;
  coord_t height;
  font_t font;

  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     tmp, font, Black, justifyCenter);

  if( firsttime ) {
    chsnprintf(celciusStr, sizeof(celciusStr), "CPU Temp: measuring...");
  } else {
    chsnprintf(celciusStr, sizeof(celciusStr), "CPU Temp: %d.%dC", celcius / 1000, 
	       abs((celcius % 1000) / 100));
  }
  gdispDrawStringBox(0, height, width, height,
		     celciusStr, font, White, justifyLeft);

  gdispFlush();
}

static uint32_t charging_init(OrchardAppContext *context) {

  (void)context;

  return 0;
}

static void charging_start(OrchardAppContext *context) {
  
  analogUpdateTemperature(); 
  orchardAppTimer(context, 500 * 1000 * 1000, true); // update UI every 500 ms
  
  redraw_ui(1);
  // all this app does is launch a text entry box and store the name
}

void charging_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keyLeft) ) {
      orchardAppExit();
    }
  } else if (event->type == timerEvent) {
    analogUpdateTemperature(); // the actual value won't update probably until the UI is redrawn...
    redraw_ui(0);
  } else if( event->type == adcEvent) {
    if( event->adc.code == adcCodeTemp ) {
      celcius = analogReadTemperature();
    }
  }
}

static void charging_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Charging settings", charging_init, charging_start, charging_event, charging_exit);


