#include "ch.h"
#include "hal.h"

#include "orchard-app.h"
#include "led.h"
#include "genes.h"

#include "gasgauge.h"
#include "analog.h"
#include "orchard-ui.h"
#include <string.h>

#include "fixmath.h"
#include "fix16_fft.h"

static uint8_t friend_index = 0;
static uint8_t numlines = 1;
static uint8_t friend_total = 0;

#define LED_UI_FONT  "fixed_5x8"
#define REFRACTORY_PERIOD 5000  // timeout for having sex
#define UI_LOCKOUT_TIME 6000  // timeout on friend list sorting/deletion after UI interaction

#define OSCOPE_IDLE_TIME 30000  // 30 seconds of idle, and we switch to an oscilloscope mode UI...because it's pretty

static uint32_t last_ui_time = 0;
static uint32_t last_oscope_time = 0;

static char title[32];
static  const char item1[] = "Yes";
static  const char item2[] = "No";
static struct OrchardUiContext listUiContext;

static uint8_t mode = 0;  // used by the oscope routine
static uint8_t oscope_running = 0;
static uint8_t *samples;

static void initiate_sex(void) {

  // put up message indicate sex initiation
  // add completion bar for mutation rate + accelerometer hook

  // send radio message to request sex

  // wait until timeout for sex protocol to return

  // if protocol returns, create children

  // if protocol fails, indicate failure in UI
}

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

static void do_oscope(void) {
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

static void redraw_ui(void) {
  font_t font;
  coord_t width;
  coord_t fontheight, header_height;
  uint8_t i, starti, j;
  color_t text_color = White;
  color_t bg_color = Black;
  const char **friends;
  char tmp[24];

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
  if( oscope_running ) {
    // do oscope stuff
    do_oscope();
    return;
  }
  
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
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight;

  gdispClear(Black);

  // generate the title bar
  gdispFillArea(0, 0, width, header_height - 1, White);
  if( strncmp(effectsCurName(), "lg", 2) != 0 )
    chsnprintf(tmp, sizeof(tmp), "%s", effectsCurName());
  else {
    chsnprintf(tmp, sizeof(tmp), "%s", lightgeneName());
  }
  gdispDrawStringBox(0, 0, width, header_height,
                     tmp, font, Black, justifyLeft);

  if( friend_total > 0 )
    chsnprintf(tmp, sizeof(tmp), "%d/%d %d%%", friend_index + 1, friend_total,
	       ggStateofCharge());
  else 
    chsnprintf(tmp, sizeof(tmp), "%d%%", ggStateofCharge());
    
  gdispDrawStringBox(0, 0, width, header_height,
                     tmp, font, Black, justifyRight);
  

  // generate the friends list
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

      // 16 is based on the total space available given other UI constraints
      // it will cut the last 4 characters off of some names but improves
      // legibility of the "n of m" motif which is important for navigating lost
      // lists of items
      chsnprintf(tmp, 16, "%d", (int) friends[i][0]);
      gdispDrawStringBox(0, header_height + j * fontheight, width, fontheight,
			 tmp, font, text_color, justifyRight);
    
    }
    friendsUnlock();
  } else {
      gdispDrawStringBox(0, header_height + 3 * fontheight, width, fontheight,
			 "no friends in range", font, text_color, justifyCenter);
      gdispDrawStringBox(0, header_height + 4 * fontheight, width, fontheight,
			 ":-(", font, text_color, justifyCenter);
  }
  
  gdispCloseFont(font);
  
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

  chsnprintf(title, sizeof(title), "Sex with %s", &(partner[1]));
  
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
  last_oscope_time = chVTGetSystemTime();
  oscope_running = 0;
  orchardGfxStart();
  // determine # of lines total displayble on the screen based on the font
  font = gdispOpenFont(LED_UI_FONT);
  height = gdispGetHeight();
  fontheight = gdispGetFontMetric(font, fontHeight);
  numlines = (height / fontheight) - 1;   // one for the title bar
  gdispCloseFont(font);
  orchardGfxEnd();
  
  listEffects();

  redraw_ui();
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
      }
      else if ( event->key.code == keyRight ) {
	effectsNextPattern();
      }
      else if( event->key.code == keyCW ) {
	if( friend_total != 0 )
	  friend_index = (friend_index + 1) % friend_total;
	else
	  friend_index = 0;
	last_ui_time = chVTGetSystemTime();
	last_oscope_time = chVTGetSystemTime();
	oscope_running = 0;
      } else if( event->key.code == keyCCW) {
	if( friend_total != 0 ) {
	  if( friend_index == 0 )
	    friend_index = friend_total;
	  friend_index--;
	} else {
	  friend_index = 0;
	}
	last_ui_time = chVTGetSystemTime();
	last_oscope_time = chVTGetSystemTime();
	oscope_running = 0;
      } else if( event->key.code == keySelect ) {
	last_ui_time = chVTGetSystemTime();
	// oscope timer does not reset on select as it should swap between FFT and time domain mode
	if( oscope_running ) {
	  mode = !mode;
	}
	if( friend_total != 0 ) {
	  // trigger sex protocol
	  confirm_sex(context);
	}
      }
    }
  } else if(event->type == radioEvent) {
    // placeholder
  } else if( event->type == uiEvent ) {
    last_ui_time = chVTGetSystemTime();
    
    chHeapFree(listUiContext.itemlist); // free the itemlist passed to the UI
    selected = (uint8_t) context->instance->ui_result;
    context->instance->ui = NULL;
    context->instance->uicontext = NULL;

    if(selected == 0) // 0 means we said yes based on list item order in the UI
      initiate_sex();
  } else if( event->type == adcEvent) {
    if( oscope_running ) {
      if( event->adc.code == adcCodeMic ) {
	samples = analogReadMic();
	redraw_ui();
      }
      analogUpdateMic();
    }
  }
  
  // redraw UI on any event
  if( context->instance->ui == NULL ) {
    redraw_ui();

    // only kick off oscope if we're not in a UI mode...
    if( ((chVTGetSystemTime() - last_oscope_time) > OSCOPE_IDLE_TIME) && !oscope_running ) {
      analogUpdateMic(); // this kicks off the ADC sampling; once this returns, the UI will swap modes automagically
      oscope_running = 1;
    }
  }
}

static void led_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Blinkies!", led_init, led_start, led_event, led_exit);


