#include "orchard-app.h"
#include "radio.h"

#include "genes.h"
#include "storage.h"
#include <string.h>

static uint32_t last_page;
static uint8_t pagecount = 0;
static uint8_t cooldown_active = 0;

#define SPAM_LIMIT 5    // max # messages to page before cooldown kicks in
#define SPAM_DECAY 2500 // time to decay spam timer

static void redraw_ui(void) {
  coord_t width;
  coord_t height;
  font_t font;
  char title[] = "Paging mode";
  char message[20];

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  // draw title box
  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     title, font, Black, justifyCenter);

  gdispDrawStringBox(0, height * 2, width, height,
                     "Announce yourself", font, White, justifyCenter);
  
  gdispDrawStringBox(0, height * 3, width, height,
                     "to everyone nearby!", font, White, justifyCenter);
  
  // draw cooldown info
  if( !cooldown_active ) {
    chsnprintf(message, sizeof(message), "%d of %d used", pagecount, SPAM_LIMIT);
    gdispDrawStringBox(0, height * 5, width, height,
		       message, font, White, justifyCenter);
  } else {
    chsnprintf(message, sizeof(message), "cooldown active %d", pagecount);
    gdispDrawStringBox(0, height * 5, width, height,
		       message, font, White, justifyCenter);
  }
  
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

static uint32_t messenger_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void messenger_start(OrchardAppContext *context) {

  (void)context;
  last_page = chVTGetSystemTime();
  redraw_ui();
}

void messenger_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  const struct genes *family;

  (void)context;
  
  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if( (chVTGetSystemTime() - last_page) > SPAM_DECAY ) {
    if( pagecount > 0 ) {
      if( cooldown_active )
	pagecount--;
      last_page = chVTGetSystemTime();
    }
  }

  if( cooldown_active ) {
    if( pagecount == 0 )
      cooldown_active = 0;
  }
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keySelect) ) {
      // send a page
      if( (pagecount < SPAM_LIMIT) && !cooldown_active ) {
	radioAcquire(radioDriver);
	radioSend(radioDriver, RADIO_BROADCAST_ADDRESS, radio_prot_paging,
		  strlen(family->name) + 1, family->name);
	radioRelease(radioDriver);
      }
      pagecount++;
      
      if( pagecount >= SPAM_LIMIT ) {
	cooldown_active = 1;
	pagecount = SPAM_LIMIT;
      }

      last_page = chVTGetSystemTime();
    }
  }
  
  redraw_ui();
}

static void messenger_exit(OrchardAppContext *context) {
  (void)context;
}

orchard_app("Announce Yourself",
    messenger_init, messenger_start, messenger_event, messenger_exit);
