#include "orchard-app.h"
#include "led.h"

static uint8_t friend_index = 0;
static uint8_t numlines = 1;
static uint8_t friend_total = 0;

#define LED_UI_FONT  "fixed_5x8"
#define REFRACTORY_PERIOD 5000  // timeout for having sex
#define UI_LOCKOUT_TIME 6000  // timeout on friend list sorting/deletion after UI interaction

static uint32_t last_ui_time = 0;

static void redraw_ui(void) {
  font_t font;
  coord_t width, height;
  coord_t fontheight, header_height;
  uint8_t i, starti, j;
  color_t text_color = White;
  color_t bg_color = Black;
  const char **friends;
  char tmp[20];

  // theory: we just need to lockout sorting
  // in the case that a new friend is added, it gets put
  // on the bottom of the list -- so it won't affect our current
  // index state
  // in the case that an existing friend is pinged, that's also
  // fine as the record updates but doesn't change order
  // in the case that a friend record fades out, we would possibly
  // see the list shrink and the current index could become invalid.
  // however, since the list is sorted by ping count, and nearby friends
  // would have a strong ping count, we probably wouldn't see this as
  // a problem -- it would only be trouble if you're trying to have
  // sex with someone at the edge of reception, which would be unreliable
  // anyways and some difficulty in selecting the friend due to fading
  // names would give you a clue of the problem
  if( (chVTGetSystemTime() - last_ui_time) > UI_LOCKOUT_TIME )
    friendsSort();

  orchardGfxStart();
  font = gdispOpenFont(LED_UI_FONT);
  width = gdispGetWidth();
  height = gdispGetHeight();
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight;

  gdispFillArea(0, header_height, width, height - header_height, Black);
  
  friendsLock();
  friends = friendsGet();
  starti = friend_index - (friend_index % numlines);
  for( i = starti, j = 0; i < starti + numlines; i++, j++ ) {
    if( friends[i] == NULL )
      continue;
    if( i == friend_index ) {
      text_color = Black;
      bg_color = White;
    } else {
      text_color = White;
      bg_color = Black;
    }
    
    gdispFillArea(0, header_height + j * fontheight, width, fontheight, bg_color);
    gdispDrawStringBox(0, header_height + j * fontheight, width, fontheight,
		       &(friends[i][1]), font, text_color, justifyLeft);

    chsnprintf(tmp, sizeof(tmp), "%d", (int) friends[i][0]);
    gdispDrawStringBox(0, header_height + j * fontheight, width, fontheight,
		       tmp, font, text_color, justifyRight);
    
  }
  friendsUnlock();
  
  gdispFlush();
  orchardGfxEnd();
}

static uint32_t led_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void led_start(OrchardAppContext *context) {
  
  (void)context;
  font_t font;
  coord_t height;
  coord_t fontheight;

  last_ui_time = chVTGetSystemTime();
  orchardGfxStart();
  // determine # of lines total displayble on the screen based on the font
  font = gdispOpenFont(LED_UI_FONT);
  height = gdispGetHeight();
  fontheight = gdispGetFontMetric(font, fontHeight);
  numlines = (height / fontheight) - 1;   // one for the title bar
  gdispCloseFont(font);
  orchardGfxEnd();
  
  listEffects();
  
  effectsSetPattern(0);  // pick default pattern

}

void led_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  uint8_t shift;
  
  if (event->type == keyEvent) {
    if (event->key.flags == keyDown) {
      if ( event->key.code == keyLeft ) {
	shift = getShift();
	shift++;
	if (shift > 6)
	  shift = 0;
	setShift(shift);
	redraw_ui();
      }
      else if ( event->key.code == keyRight ) {
	effectsNextPattern();
	redraw_ui();
      }
      else if( event->key.code == keyCW ) {
	if( friend_total != 0 )
	  friend_index = friend_index + 1 % friend_total;
	else
	  friend_index = 0;
	redraw_ui();
	last_ui_time = chVTGetSystemTime();
      } else if( event->key.code == keyCCW) {
	if( friend_total != 0 ) {
	  if( friend_index == 0 )
	    friend_index = friend_total;
	  friend_index--;
	} else {
	  friend_index = 0;
	}
	redraw_ui();
	last_ui_time = chVTGetSystemTime();
      } else if( event->key.code == keySelect ) {
	last_ui_time = chVTGetSystemTime();
	if( friend_total != 0 ) {
	  // trigger sex protocol
	}
	redraw_ui();
      }
    }
  } else if(event->type == radioEvent) {
    redraw_ui();
  }
}

static void led_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Blinkies!", led_init, led_start, led_event, led_exit);


