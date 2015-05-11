#include "orchard-app.h"
#include "radio.h"

static void radio_message_received(uint8_t type, uint8_t src, uint8_t dst,
                                   uint8_t length, void *data) {

  (void)length;
  (void)type;
  chprintf(stream, "Received %s message from %02x: %s\r\n",
      (dst == RADIO_BROADCAST_ADDRESS) ? "broadcast" : "direct", src, data);
}

static uint32_t messenger_init(OrchardAppContext *context) {

  (void)context;
  radioSetHandler(radioDriver, 1, radio_message_received);
  return 0;
}

static void messenger_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Messenger", messenger_init, NULL, NULL, messenger_exit);
