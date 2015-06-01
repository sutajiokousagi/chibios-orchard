#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-events.h"
#include "analog.h"

#include "chbsem.h"

static adcsample_t mic_sample[MIC_SAMPLE_DEPTH];
static uint8_t mic_return[MIC_SAMPLE_DEPTH];
mutex_t adc_mutex;

#define ADC_GRPCELCIUS_NUM_CHANNELS   2
#define ADC_GRPCELCIUS_BUF_DEPTH      1
static int32_t celcius;
static adcsample_t celcius_samples[ADC_GRPCELCIUS_NUM_CHANNELS * ADC_GRPCELCIUS_BUF_DEPTH];

usbStat usb_status = usbStatNC;
#define ADC_GRPUSB_NUM_CHANNELS 2
#define ADC_GRPUSB_BUF_DEPTH 1
static adcsample_t usb_samples[ADC_GRPUSB_NUM_CHANNELS * ADC_GRPUSB_BUF_DEPTH];
static uint16_t usbn, usbp;

static void adc_temperature_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

  /*
   * The bandgap value represents the ADC reading for 1.0V
   */
  uint16_t sensor = buffer[0];
  uint16_t bandgap = buffer[1];

  /*
   * The v25 value is the voltage reading at 25C, it comes from the ADC
   * electricals table in the processor manual. V25 is in millivolts.
   */
  int32_t v25 = 716;

  /*
   * The m value is slope of the temperature sensor values, again from
   * the ADC electricals table in the processor manual.
   * M in microvolts per degree.
   */
  int32_t m = 1620;

  /*
   * Divide the temperature sensor reading by the bandgap to get
   * the voltage for the ambient temperature in millivolts.
   */
  int32_t vamb = (sensor * 1000) / bandgap;

  /*
   * This formula comes from the reference manual.
   * Temperature is in millidegrees C.
   */
  int32_t delta = (((vamb - v25) * 1000000) / m);
  celcius = 25000 - delta;

  chSysLockFromISR();
  chEvtBroadcastI(&celcius_rdy);
  chSysUnlockFromISR();
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 * Note: this comment above is from chibiOS sample code. I don't actually get
 *  what they mean by "8 samples of 1 channel" because that doesn't look like
 *  what's happening. 
 */
static const ADCConversionGroup adcgrpcelcius = {
  false,
  ADC_GRPCELCIUS_NUM_CHANNELS,
  adc_temperature_end_cb,
  NULL,
  ADC_TEMP_SENSOR | ADC_BANDGAP,
  /* CFG1 Regiser - ADCCLK = SYSCLK / 16, 16 bits per sample */
  ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
  ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
  ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),
  /* SC3 Register - Average 32 readings per sample */
  ADCx_SC3_AVGE |
  ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES)
};

void analogUpdateTemperature(void) {
  adcAcquireBus(&ADCD1);
  adcConvert(&ADCD1, &adcgrpcelcius, celcius_samples, ADC_GRPCELCIUS_BUF_DEPTH);
  adcReleaseBus(&ADCD1);
}

int32_t analogReadTemperature() {
  return celcius;
}


static void adc_mic_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

  uint8_t i;

  for( i = 0 ; i < n; i++ ) { 
    mic_return[i] = (uint8_t) buffer[i];
  }

  chSysLockFromISR();
  chEvtBroadcastI(&mic_rdy);
  chSysUnlockFromISR();
}


/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 */
static const ADCConversionGroup adcgrpmic = {
  false,
  1, // just one channel
  adc_mic_end_cb,
  NULL,
  ADC_DADP1,  // microphone input channel
  // SYCLK = 48MHz. ADCCLK = SYSCLK / 4 / 2 = 6 MHz
  ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_4) |
  ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
  ADCx_CFG1_MODE(ADCx_CFG1_MODE_8_OR_9_BITS),  // 8 bits per sample
  ADCx_SC3_AVGE |
  ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) // 32 sample average
  // this should give ~6.25kHz sampling rate
};

void analogUpdateMic(void) {
  adcAcquireBus(&ADCD1);
  adcConvert(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
  adcReleaseBus(&ADCD1);
}

uint8_t *analogReadMic() {
  return mic_return;
}


static void adc_usb_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

  usbn = (uint16_t) buffer[0];
  usbp = (uint16_t) buffer[1];
  
  // plugged into "live" USB port: USBN > 0xE000 && < 0xF800, USBP < 0x200
  // not connected: UNBN > 0xFF00, USBP > 0x200 (primarily USBN is the determining factor)
  // plugged into "CDP" port: USBN ~ USBP ~ FD00
  usb_status = usbStatNC;
  if( (usbn > 0xE000) && (usbn < 0xF800) && (usbp < 0x400) ) {
    usb_status = usbStat500;
  } else if( (usbn > 0xF000) && (usbp > 0xF000) ) {
    usb_status = usbStat1500;
  } else if( (usbn > 0xFE80) ) {
    usb_status = usbStatNC;
  }
  
  chSysLockFromISR();
  chEvtBroadcastI(&usbdet_rdy);
  chSysUnlockFromISR();
}


/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 */
static const ADCConversionGroup adcgrpusb = {
  false,
  ADC_GRPUSB_NUM_CHANNELS, 
  adc_usb_end_cb,
  NULL,
  ADC_AD9 | ADC_AD12,  // USB D- and USB D+, respectively
  ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
  ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
  ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),  // 16 bits
  ADCx_SC3_AVGE |
  ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) // 32 sample average
};

void analogUpdateUsbStatus(void) {
  adcAcquireBus(&ADCD1);
  adcConvert(&ADCD1, &adcgrpusb, usb_samples, ADC_GRPUSB_BUF_DEPTH);
  adcReleaseBus(&ADCD1);
}

usbStat analogReadUsbStatus(void) {
  return usb_status;
}

adcsample_t *analogReadUsbRaw(void) {
  return usb_samples;
}


void analogStart() {
  
}
