#include "orchard-app.h"
#include "charger.h"
#include "gasgauge.h"
#include "orchard-test.h" // for cheezy UI prompts
#include "userconfig.h"

static uint8_t modeintent = 0;

static void redraw_ui(void) {
  coord_t width;
  coord_t height;
  font_t font;
  color_t text_color;
  color_t bg_color;

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  // draw title box
  gdispClear(Black);

  gdispDrawStringBox(0, height * 2, width, height,
                     "Select option...", font, White, justifyCenter);

  if( modeintent == 0 ) {
    text_color = Black;
    bg_color = White;
  } else {
    text_color = White;
    bg_color = Black;
  }
    
  gdispFillArea(0, height*4, width, height, bg_color);
  gdispDrawStringBox(0, height * 4, width, height,
		     "Standby (recommended)", font, text_color, justifyCenter);

  if( modeintent != 0 ) {
    text_color = Black;
    bg_color = White;
  } else {
    text_color = White;
    bg_color = Black;
  }
  gdispFillArea(0, height*5, width, height, bg_color);
  gdispDrawStringBox(0, height * 5, width, height,
                     "Disconnect battery", font, text_color, justifyCenter);
  
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

static void shipmode_start(OrchardAppContext *context) {

  (void)context;

  configFlush();  // flush all volatile state before shutting down
  redraw_ui();
}

void shipmode_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  (void)context;
  
  if (event->type == keyEvent) {
    if ((event->key.flags == keyDown) && (event->key.code == keySelect)) {
      if( modeintent == 0 ) {
	orchardTestPrompt("Standing by...", "hit reset to wake", 0);
	halt();
      } else {
	chThdYield();
	chThdSleepMilliseconds(300); // wait for previous touch state to drain
	if(orchardTestPrompt("Are you sure?", "Press key to confirm", 5) == orchardResultPass )
	  chargerShipMode();
	else
	  redraw_ui();
      }
    } else if( event->key.flags == keyDown ) {
      modeintent = !modeintent;
      redraw_ui();
    }
  }
}

orchard_app("power off", NULL, shipmode_start, shipmode_event, NULL);
