#ifndef __BLE_SERVICE_H__
#define __BLE_SERVICE_H__

#include <stdint.h>

struct ble_service_message {
  uint8_t status_byte;
  uint8_t buffer[32];
};

struct ble_service {
  const uint8_t                     count;
  const struct ble_service_message *data;
};

extern const struct ble_service hid_keyboard;
extern const struct ble_service ble_broadcast;

#endif /* __BLE_SERVICE_H__ */
