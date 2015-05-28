#define MIC_SAMPLE_DEPTH  128

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);
uint8_t *analogReadMic(void);

void analogStart(void);
