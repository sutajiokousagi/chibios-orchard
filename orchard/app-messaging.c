#include "orchard-app.h"
#include "radio.h"

#include "genes.h"
#include "storage.h"
#include <string.h>

#define MSG_MAXLEN 16
static char message[MSG_MAXLEN];
static uint32_t rxseq = 0;
static uint32_t txseq = 0;

static void redraw_ui(void) {
  coord_t width;
  coord_t height;
  font_t font;
  char title[] = "Messenger test";
  char seqstr[16];

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

  // draw txseq
  chsnprintf(seqstr, 16, "%d", txseq);
  gdispDrawStringBox(0, height * 2, width, height,
                     seqstr, font, White, justifyCenter);
  
  // draw rxseq
  chsnprintf(seqstr, 16, "%d", rxseq);
  gdispDrawStringBox(0, height * 3, width, height,
                     seqstr, font, White, justifyCenter);
  
  // draw message
  gdispDrawStringBox(0, height * 4, width, height,
                     message, font, White, justifyCenter);
  
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

static uint32_t messenger_init(OrchardAppContext *context) {
  (void)context;
  return 0;
}

static void messenger_start(OrchardAppContext *context) {
  chsnprintf(message, MSG_MAXLEN, "%s", "nothing yet");
  redraw_ui();
}

void messenger_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  (void)context;
  const struct genes *family;
  
  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keySelect) ) {
      // send a message
      txseq++;
      radioSend(radioDriver, RADIO_BROADCAST_ADDRESS, 1, strlen(family->name) + 1, family->name);
    }
  }
  redraw_ui();
  
}

static void messenger_exit(OrchardAppContext *context) {
  (void)context;
}

orchard_app("Messenger test", messenger_init, messenger_start, messenger_event, messenger_exit);
