#ifndef __ORCHARD_RADIO_H__
#define __ORCHARD_RADIO_H__

struct _KRadioDevice;
typedef struct _KRadioDevice KRadioDevice;

#define RADIO_NETWORK_MAX_LENGTH 8

typedef struct _RadioPacket {
  uint8_t length;       /* Total length of packet, including length field */
  uint8_t dst;          /* Address of intended recipient */
  uint8_t src;          /* Address of device sending packet */
  uint8_t type;         /* Subsystem packet is intended for */
  uint8_t payload[0];   /* Actual contents of packet */
} RadioPacket;

extern KRadioDevice KRADIO1;

void radioStart(KRadioDevice *radio, SPIDriver *spip);
uint8_t radioRead(KRadioDevice *radio, uint8_t addr);
void radioWrite(KRadioDevice *radio, uint8_t addr, uint8_t val);
int radioDump(KRadioDevice *radio, uint8_t addr, void *bfr, int count);
int radioTemperature(KRadioDevice *radio);
void radioSetNetwork(KRadioDevice *radio, const uint8_t *id, uint8_t len);
void radioSend(KRadioDevice *radio, uint8_t dest, uint8_t type,
                                    size_t len, const void *payload);

void radioSetDefaultHandler(KRadioDevice *radio,
                            void (*handler)(uint8_t type,
                                            uint8_t src,
                                            uint8_t dst,
                                            uint8_t length,
                                            void *data));
void radioSetHandler(KRadioDevice *radio, uint8_t type,
                     void (*handler)(uint8_t type,
                                     uint8_t src,
                                     uint8_t dst,
                                     uint8_t length,
                                     void *data));

void radioInterrupt(EXTDriver *extp, expchannel_t channel);

#endif /* __ORCHARD_RADIO_H__ */
