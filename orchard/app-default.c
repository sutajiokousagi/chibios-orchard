#include "orchard-app.h"
#include "led.h"
#include "genes.h"

#include "orchard-ui.h"
#include <string.h>

static uint8_t friend_index = 0;
static uint8_t numlines = 1;
static uint8_t friend_total = 0;

#define LED_UI_FONT  "fixed_5x8"
#define REFRACTORY_PERIOD 5000  // timeout for having sex
#define UI_LOCKOUT_TIME 6000  // timeout on friend list sorting/deletion after UI interaction

static uint32_t last_ui_time = 0;

static  const char title[] = "Have sex with";
static  const char item1[] = "Yes";
static  const char item2[] = "No";
static struct OrchardUiContext listUiContext;

static void initiate_sex(void) {
  
}

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

  friend_total = friendCount();
  // reset the index in case the total # of friends shrank on us while we weren't looking
  if( (friend_index >= friend_total) && (friend_total > 0) )
    friend_index = friend_total - 1;
  if( friend_total == 0 )
    friend_index = 0;

  orchardGfxStart();
  font = gdispOpenFont(LED_UI_FONT);
  width = gdispGetWidth();
  height = gdispGetHeight();
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight;

  gdispFillArea(0, header_height, width, height - header_height, Black);

  if( friend_total > 0 ) {
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
  } else {
      gdispDrawStringBox(0, header_height + 4 * fontheight, width, fontheight,
			 "no friends in range", font, text_color, justifyCenter);
      gdispDrawStringBox(0, header_height + 5 * fontheight, width, fontheight,
			 ":-(", font, text_color, justifyCenter);
  }
  
  gdispFlush();
  orchardGfxEnd();
}

static void confirm_sex(OrchardAppContext *context) {
  const char **friends;
  char partner[GENE_NAMELENGTH];
  const OrchardUi *listUi;

  if( friend_total == 0 )
    return;

  friendsLock();
  friends = friendsGet();
  if( friends[friend_index] == NULL ) {
    friendsUnlock();
    return;
  }
  strncpy(partner, friends[friend_index], GENE_NAMELENGTH);
  friendsUnlock();

  listUi = getUiByName("list");
  listUiContext.total = 2;  // two items, yes and no
  listUiContext.selected = 0;
  listUiContext.itemlist = (const char **) chHeapAlloc(NULL, sizeof(char *) * 3); // 3 lines incl header
  if( listUiContext.itemlist == NULL )
    return;
  listUiContext.itemlist[0] = title;
  listUiContext.itemlist[1] = item1;
  listUiContext.itemlist[2] = item2;

  if( listUi != NULL ) {
    context->instance->uicontext = &listUiContext;
    context->instance->ui = listUi;
  }
  listUi->start(context);
  
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
  
  //  effectsSetPattern(0);  // pick default pattern

}

void led_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  uint8_t shift;
  uint8_t selected = 0;
  
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
	  confirm_sex(context);
	}
	redraw_ui();
      }
    }
  } else if(event->type == radioEvent) {
    redraw_ui();
  } else if( event->type == uiEvent ) {
    last_ui_time = chVTGetSystemTime();
    
    chHeapFree(listUiContext.itemlist); // free the itemlist passed to the UI
    selected = (uint8_t) context->instance->ui_result;
    context->instance->ui = NULL;
    context->instance->uicontext = NULL;

    if(selected == 0) // 0 means we said yes based on list item order in the UI
      initiate_sex();
    
    redraw_ui();
  }
}

static void led_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Blinkies!", led_init, led_start, led_event, led_exit);


