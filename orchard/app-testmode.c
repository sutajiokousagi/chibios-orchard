#include "orchard-app.h"
#include "orchard-test.h"

#include "radio.h"

static uint32_t testmode_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void testmode_start(OrchardAppContext *context) {
  (void)context;
  
  orchardTestPrompt("Testing...", "", 0);
}

static void testmode_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  // dummy app that does nothing with events sent to it.
  // tests will create events and having any other app running could interfere with tests
}

static void testmode_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("~testmode", testmode_init, testmode_start, testmode_event, testmode_exit);


static uint32_t test_rxdat;
static void test_peer_handler(uint8_t type, uint8_t src, uint8_t dst,
			       uint8_t length, void *data) {

  (void)length;
  (void)type;

  test_rxdat = *((uint32_t *) data);
    
  chEvtBroadcast(&radio_app);
}

static uint32_t testpeer_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void testpeer_start(OrchardAppContext *context) {
  (void)context;
  
  radioSetHandler(radioDriver, RADIO_TYPE_DUT_TO_PEER, test_peer_handler);
  
  orchardTestPrompt("Now a test peer", "reboot to stop.", 0);
}

static void testpeer_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  (void)context;
  char datstr[16];
  
  if(event->type == radioEvent) {
    chprintf(stream, "received %08x, rebroadcasting...\n\r", test_rxdat);
    chsnprintf(datstr, sizeof(datstr), "%08x", test_rxdat );
    orchardTestPrompt("received:", datstr, 0);
    radioSend(radioDriver, RADIO_BROADCAST_ADDRESS, RADIO_TYPE_PEER_TO_DUT, 4, &test_rxdat);
  }
}

static void testpeer_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("~testpeer", testpeer_init, testpeer_start, testpeer_event, testpeer_exit);
