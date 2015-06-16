#include "orchard-app.h"
#include "orchard-events.h"
#include "orchard-ui.h"
#include <string.h>

static const char entry_list[] = "abcdefghijklmnopqrstuvwxyz 0123456789.,!?;:-)(=\\_/<>~|@#$%^&*{}[]";
static uint8_t entry_selection = 0;
static char entry[TEXTENTRY_MAXLEN + 1];  // +1 for the null terminator
static uint8_t entry_position = 0;

// Note: text entry UI assumes you will draw your own header graphic, using 5x8 font
static void textentry_redraw(void) {
  font_t font;
  coord_t width, height;
  coord_t header_height;
  coord_t fontheight, fontwidth;
  uint8_t listLen;
  uint8_t entriesPerWidth, entriesPerHeight;
  uint8_t leftmargin;
  uint8_t i, j, drawindex;
  color_t draw_color = White;
  char str[2];

  str[0] = '\0'; str[1] = '\0';
  
  orchardGfxStart();
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetHeight();
  
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight + 1;
  fontheight = fontheight + 1; // 5x8 font metric does not include adjacent space
  fontwidth = gdispGetFontMetric(font, fontMaxWidth) + 1;

  listLen = (uint8_t) strlen(entry_list);

  entriesPerWidth = width / fontwidth;
  entriesPerHeight = height / fontheight;
  // center up the rendering with remainder
  leftmargin = (width - (entriesPerWidth * fontwidth)) / 2;

  // blank the window region we "own" (all but the header height)
  gdispFillArea( 0, header_height, width, height - header_height, Black );
  
  // draw cursor
  gdispFillArea( entry_position * fontwidth,
		 header_height,
		 fontwidth, fontheight,
		 White );

  // draw current text entry progress
  i = 0;
  while( entry[i] != '\0' ) {
    str[0] = entry[i];
    gdispDrawStringBox(i * fontwidth,
		       header_height,
		       fontwidth, fontheight,
		       str, font, White, justifyCenter);
    i++;
  }

  header_height += fontheight + 3;
  // draw entry array
  drawindex = 0;
  for( j = 0; j < entriesPerHeight; j++ ) {
    for( i = 0; i < entriesPerWidth; i++ ) {
      draw_color = White;
      str[0] = entry_list[drawindex];
      
      if( drawindex == entry_selection ) {
	gdispFillArea(leftmargin + i * fontwidth,
		      header_height + j * fontheight,
		      fontwidth, fontheight,
		      White);
	draw_color = Black;
      }
      
      gdispDrawStringBox(leftmargin + i * fontwidth,
			 header_height + j * fontheight,
			 fontwidth, fontheight,
			 str, font, draw_color, justifyCenter);
      drawindex++;
      if( drawindex == listLen)
	break;
    }
    if( drawindex == listLen)
      break;
  }
  
  gdispFlush();
  orchardGfxEnd();
}

static void textentry_start(OrchardAppContext *context) {

  (void)context;

  entry_selection = 0;
  memset(&entry, 0, TEXTENTRY_MAXLEN + 1);
  entry_position = 0;
  
  textentry_redraw();
}

static void textentry_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  //// note to self -- once the text entry is complete, the UI widget should
  //// send an event back to the instance app indicating UI routine is complete
  
  if (event->type == keyEvent) {
    if (event->key.flags == keyDown) {
      if( event->key.code == keyCW ) {
	entry_selection++;
	if( entry_selection >= sizeof(entry_list) )
	  entry_selection--;
	
      } else if( event->key.code == keyCCW) {
	if( entry_selection > 0 )
	  entry_selection--;
	
      } else if( event->key.code == keySelect ) {
	if( entry_position < TEXTENTRY_MAXLEN )
	  entry[entry_position] = entry_list[entry_selection];
	entry_position++;
	if( entry_position > TEXTENTRY_MAXLEN )
	  entry_position--;
	
      } else if( event->key.code == keyLeft ) {
	if( entry_position > 0 )
	  entry_position--;
	entry[entry_position] = '\0';
	
      } else if( event->key.code == keyRight ) {
	context->instance->ui_result = (uint32_t) entry;
	chEvtBroadcast(&ui_completed);
      }
    }
  }

  textentry_redraw();
}

static void textentry_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_ui("textentry", textentry_start, textentry_event, textentry_exit);

