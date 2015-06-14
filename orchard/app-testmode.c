#include "orchard-app.h"
#include "orchard-test.h"

#include "radio.h"

static void testmode_start(OrchardAppContext *context) {
  (void)context;
  
  orchardTestPrompt("Testing...", "", 0);
}

orchard_app("~testmode", NULL, testmode_start, NULL, NULL);


static uint32_t test_rxdat;
static void test_peer_handler(uint8_t type, uint8_t src, uint8_t dst,
                              uint8_t length, void *data) {

  (void)length;
  (void)type;
  (void)src;
  (void)dst;

  test_rxdat = *((uint32_t *) data);

  chEvtBroadcast(&radio_app);
}

static void testpeer_start(OrchardAppContext *context) {
  (void)context;
  
  radioSetHandler(radioDriver, RADIO_TYPE_DUT_TO_PEER, test_peer_handler);
  
  orchardTestPrompt("Now a test peer", "reboot to stop.", 0);
}

static void testpeer_event(OrchardAppContext *context,
                           const OrchardAppEvent *event) {
  (void)context;
  char datstr[16];
  
  if(event->type == radioEvent) {
    chprintf(stream, "received %08x, rebroadcasting...\n\r", test_rxdat);
    chsnprintf(datstr, sizeof(datstr), "%08x", test_rxdat);
    orchardTestPrompt("received:", datstr, 0);
    radioSend(radioDriver, RADIO_BROADCAST_ADDRESS, RADIO_TYPE_PEER_TO_DUT,
              4, &test_rxdat);
  }
}

orchard_app("~testpeer", NULL, testpeer_start, testpeer_event, NULL);
