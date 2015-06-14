#include "orchard-app.h"
#include "radio.h"

#include "led.h"
#include "paging.h"
#include "genes.h"
#include "storage.h"
#include <string.h>

static char message[MSG_MAXLEN];
static uint32_t rxseq = 0;

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

#if 0
  // draw txseq
  chsnprintf(seqstr, 16, "%d", txseq);
  gdispDrawStringBox(0, height * 2, width, height,
                     seqstr, font, White, justifyCenter);
#endif
  // draw rxseq
  chsnprintf(seqstr, 16, "%d", rxseq);
  gdispDrawStringBox(0, height * 3, width, height,
                     seqstr, font, White, justifyCenter);
  
  // draw message
  gdispDrawStringBox(0, height * 4, width, height,
                     message, font, White, justifyCenter);
  
  gdispFlush();
  orchardGfxEnd();
}

void radioPagePopup(void) {
  
  redraw_ui();
  chThdSleepMilliseconds(PAGE_DISPLAY_MS);

}

static void radio_message_received(uint8_t type, uint8_t src, uint8_t dst,
                                   uint8_t length, const void *data) {

  (void)length;
  (void)type;
  chprintf(stream, "Received %s message from %02x: %s\r\n",
      (dst == RADIO_BROADCAST_ADDRESS) ? "broadcast" : "direct", src, data);

  strncpy(message, data, MSG_MAXLEN);
  message[MSG_MAXLEN - 1] = '\0'; // force a null termination (by truncation) if there isn't one
  rxseq++;

  chEvtBroadcast(&radio_page);
}


void pagingStart(void) {
  radioSetHandler(radioDriver, 1, radio_message_received);
}
