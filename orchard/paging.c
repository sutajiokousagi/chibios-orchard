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
  char seqstr[16];

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("ui2");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  
#if 0
  // draw title box
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
#endif
  
  gdispFillArea(0, 8, width, height, Black);
  // draw message
  gdispDrawStringBox(0, height * 2, width, height,
                     message, font, White, justifyCenter);


  switch(rand() & 0x3) {
  case 0:
    chsnprintf(seqstr, 16, "loves you!");
    break;
  case 1:
    chsnprintf(seqstr, 16, "says whassup!");
    break;
  case 2:
    chsnprintf(seqstr, 16, "says hello!");
    break;
  case 3:
    chsnprintf(seqstr, 16, "sends hugs!");
    break;
  }
  gdispDrawStringBox(0, height * 4, width, height,
		     seqstr, font, White, justifyCenter);
  
  gdispCloseFont(font);
  gdispFlush();
  orchardGfxEnd();
}

void radioPagePopup(void) {
  
  chThdSleepMilliseconds(100);  // wait 100ms before doing a redraw to flush event queues
  redraw_ui();
  chThdSleepMilliseconds(PAGE_DISPLAY_MS);
}

static void radio_message_received(uint8_t prot, uint8_t src, uint8_t dst,
                                   uint8_t length, const void *data) {

  (void)length;
  (void)prot;
  chprintf(stream, "Received %s message from %02x: %s\r\n",
      (dst == RADIO_BROADCAST_ADDRESS) ? "broadcast" : "direct", src, data);

  strncpy(message, data, MSG_MAXLEN);
  message[MSG_MAXLEN - 1] = '\0'; // force a null termination (by truncation) if there isn't one
  rxseq++;

  chEvtBroadcast(&radio_page);
}


void pagingStart(void) {
  radioSetHandler(radioDriver, radio_prot_paging, radio_message_received);
}
