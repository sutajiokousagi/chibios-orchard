#define MIC_SAMPLE_DEPTH  128

typedef enum usbStat {
  usbStatNC = 0,
  usbStat500,   // if we see any USB, let's assume 500 mA charging level
  usbStat1500,  // if we can detect a CDP, let's use it
  usbStatUnknown,
} usbStat;

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);
uint8_t *analogReadMic(void);

void analogUpdateUsbStatus(void);
usbStat analogReadUsbStatus(void);
adcsample_t *analogReadUsbRaw(void);

void analogStart(void);
