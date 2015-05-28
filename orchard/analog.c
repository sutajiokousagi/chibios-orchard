#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-events.h"
#include "analog.h"

// double buffer, one buffer samples, the other reads out
static uint8_t micBuf0[MIC_SAMPLE_DEPTH];
static uint8_t micBuf1[MIC_SAMPLE_DEPTH];
static uint8_t micBufNum = 0;



#define ADC_GRPCELCIUS_NUM_CHANNELS   2
#define ADC_GRPCELCIUS_BUF_DEPTH      1
static int32_t celcius;
static adcsample_t celcius_samples[ADC_GRPCELCIUS_NUM_CHANNELS * ADC_GRPCELCIUS_BUF_DEPTH];

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

void adcUpdateTemperature(void) {
  adcConvert(&ADCD1, &adcgrpcelcius, celcius_samples, ADC_GRPCELCIUS_BUF_DEPTH);
}

int32_t adcReadTemperature() {
  return celcius;
}
