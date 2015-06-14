
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "accel.h"
#include "gpiox.h"
#include "orchard.h"
#include "orchard-events.h"

#include "orchard-test.h"
#include "test-audit.h"

#define REG_INT_SYSMOD                0x0b
#define REG_INT_SYSMOD_SYSMOD1          (1 << 1)
#define REG_INT_SYSMOD_SYSMOD0          (1 << 0)
#define REG_INT_SYSMOD_SYSMOD           (1 << 0)
#define REG_INT_SYSMOD_STANDBY          (0 << 0)
#define REG_INT_SYSMOD_WAKE             (1 << 0)
#define REG_INT_SYSMOD_SLEEP            (2 << 0)

#define REG_INT_SRC                   0x0c
#define REG_INT_SRC_ASLP                (1 << 7)
#define REG_INT_SRC_FIFO                (1 << 6)
#define REG_INT_SRC_TRANS               (1 << 5)
#define REG_INT_SRC_LNDPRT              (1 << 4)
#define REG_INT_SRC_PULSE               (1 << 3)
#define REG_INT_SRC_FF_MT               (1 << 2)
#define REG_INT_SRC_DRDY                (1 << 0)

#define REG_WHO_AM_I                  0x0d

#define REG_XYZ_DATA_CFG              0x0e
#define REG_XYZ_DATA_CFG_HPF_OUT        (1 << 4)
#define REG_XYZ_DATA_CFG_FS1            (1 << 1)
#define REG_XYZ_DATA_CFG_FS0            (1 << 0)
#define REG_XYZ_DATA_CFG_FS_2G          (0 << 0)
#define REG_XYZ_DATA_CFG_FS_4G          (1 << 0)
#define REG_XYZ_DATA_CFG_FS_8G          (2 << 0)

#define REG_HP_FILTER_CUTOFF          0x0f
#define REG_HP_FILTER_CUTOFF_PULSE_HPF_BYP  (1 << 5)
#define REG_HP_FILTER_CUTOFF_PULSE_LPF_EN   (1 << 4)
#define REG_HP_FILTER_CUTOFF_SEL1           (1 << 1)
#define REG_HP_FILTER_CUTOFF_SEL0           (1 << 0)

#define REG_PL_STATUS                 0x10
#define REG_PL_STATUS_NEWLP             (1 << 7)
#define REG_PL_STATUS_LO                (1 << 6)
#define REG_PL_STATUS_LAPO1             (1 << 2)
#define REG_PL_STATUS_LAPO0             (1 << 1)
#define REG_PL_STATUS_LAPO_PORT_UP      (0 << 1)
#define REG_PL_STATUS_LAPO_PORT_DOWN    (1 << 1)
#define REG_PL_STATUS_LAPO_LAND_RIGHT   (2 << 1)
#define REG_PL_STATUS_LAPO_LAND_LEFT    (3 << 1)
#define REG_PL_STATUS_BAFRO             (1 << 0)

#define REG_PL_CFG                    0x11
#define REG_PL_CFG_DBCNTM               (1 << 7)
#define REG_PL_CFG_PL_EN                (1 << 6)

#define REG_PL_COUNT                  0x12

#define REG_PL_ZCOMP                  0x13
#define REG_PL_ZCOMP_BKFR1              (1 << 7)
#define REG_PL_ZCOMP_BKFR0              (1 << 6)
#define REG_PL_ZCOMP_ZLOCK2             (1 << 2)
#define REG_PL_ZCOMP_ZLOCK1             (1 << 1)
#define REG_PL_ZCOMP_ZLOCK0             (1 << 0)

#define REG_PL_THS_REG                0x14
#define REG_PL_THS_REG_PL_THS           (1 << 3)
#define REG_PL_THS_REG_HYS              (1 << 0)

#define REG_FF_MT_CFG                 0x15
#define REG_FF_MT_CFG_ELE               (1 << 7)
#define REG_FF_MT_CFG_OAE               (1 << 6)
#define REG_FF_MT_CFG_ZEFE              (1 << 5)
#define REG_FF_MT_CFG_YEFE              (1 << 4)
#define REG_FF_MT_CFG_XEFE              (1 << 3)

#define REG_FF_MT_SRC                 0x16
#define REG_FF_MT_SRC_EA                (1 << 7)
#define REG_FF_MT_SRC_ZHE               (1 << 5)
#define REG_FF_MT_SRC_ZHP               (1 << 4)
#define REG_FF_MT_SRC_YHE               (1 << 3)
#define REG_FF_MT_SRC_YHP               (1 << 2)
#define REG_FF_MT_SRC_XHE               (1 << 1)
#define REG_FF_MT_SRC_XHP               (1 << 0)

#define REG_FF_MT_THS                 0x17
#define REG_FF_MT_THS_DBCNTM            (1 << 7)

#define REG_FF_MT_COUNT               0x18

#define REG_TRANSIENT_CFG             0x1d
#define REG_TRANSIENT_CFG_ELE           (1 << 4)
#define REG_TRANSIENT_CFG_ZTEFE         (1 << 3)
#define REG_TRANSIENT_CFG_YTEFE         (1 << 2)
#define REG_TRANSIENT_CFG_XTEFE         (1 << 1)
#define REG_TRANSIENT_CFG_HFP_BYP       (1 << 0)

#define REG_TRANSIENT_SRC             0x1e
#define REG_TRANSIENT_SRC_EA            (1 << 6)
#define REG_TRANSIENT_SRC_ZTRANSE       (1 << 5)
#define REG_TRANSIENT_SRC_Z_TRANS_POL   (1 << 4)
#define REG_TRANSIENT_SRC_YTRANSE       (1 << 3)
#define REG_TRANSIENT_SRC_Y_TRANS_POL   (1 << 2)
#define REG_TRANSIENT_SRC_XTRANSE       (1 << 1)
#define REG_TRANSIENT_SRC_X_TRANS_POL   (1 << 0)

#define REG_TRANSIENT_THS             0x1f
#define REG_TRANSIENT_THS_DBCNTM        (1 << 7)
#define REG_TRANSIENT_THS_THS6          (1 << 6)
#define REG_TRANSIENT_THS_THS5          (1 << 5)
#define REG_TRANSIENT_THS_THS4          (1 << 4)
#define REG_TRANSIENT_THS_THS3          (1 << 3)
#define REG_TRANSIENT_THS_THS2          (1 << 2)
#define REG_TRANSIENT_THS_THS1          (1 << 1)
#define REG_TRANSIENT_THS_THS0          (1 << 0)

#define REG_TRANSIENT_COUNT           0x20

#define REG_PULSE_CFG                 0x21
#define REG_PULSE_CFG_DPA               (1 << 7)
#define REG_PULSE_CFG_ELE               (1 << 6)
#define REG_PULSE_CFG_ZDPEFE            (1 << 5)
#define REG_PULSE_CFG_ZSPEFE            (1 << 4)
#define REG_PULSE_CFG_YDPEFE            (1 << 3)
#define REG_PULSE_CFG_YSPEFE            (1 << 2)
#define REG_PULSE_CFG_XDPEFE            (1 << 1)
#define REG_PULSE_CFG_XSPEFE            (1 << 0)

#define REG_PULSE_SRC                 0x22
#define REG_PULSE_SRC_EA                (1 << 7)
#define REG_PULSE_SRC_AXZ               (1 << 6)
#define REG_PULSE_SRC_AXY               (1 << 5)
#define REG_PULSE_SRC_AXX               (1 << 4)
#define REG_PULSE_SRC_DPE               (1 << 3)
#define REG_PULSE_SRC_POLZ              (1 << 2)
#define REG_PULSE_SRC_POLY              (1 << 1)
#define REG_PULSE_SRC_POLX              (1 << 0)

#define REG_PULSE_THSX                0x23

#define REG_PULSE_THSY                0x24

#define REG_PULSE_THSZ                0x25

#define REG_PULSE_TMLT                0x26
#define REG_PULSE_TMLT_TMLT7            (1 << 7)
#define REG_PULSE_TMLT_TMLT6            (1 << 6)
#define REG_PULSE_TMLT_TMLT5            (1 << 5)
#define REG_PULSE_TMLT_TMLT4            (1 << 4)
#define REG_PULSE_TMLT_TMLT3            (1 << 3)
#define REG_PULSE_TMLT_TMLT2            (1 << 2)
#define REG_PULSE_TMLT_TMLT1            (1 << 1)
#define REG_PULSE_TMLT_TMLT0            (1 << 0)

#define REG_PULSE_LTCY                0x27
#define REG_PULSE_LTCY_LTCY7            (1 << 7)
#define REG_PULSE_LTCY_LTCY6            (1 << 6)
#define REG_PULSE_LTCY_LTCY5            (1 << 5)
#define REG_PULSE_LTCY_LTCY4            (1 << 4)
#define REG_PULSE_LTCY_LTCY3            (1 << 3)
#define REG_PULSE_LTCY_LTCY2            (1 << 2)
#define REG_PULSE_LTCY_LTCY1            (1 << 1)
#define REG_PULSE_LTCY_LTCY0            (1 << 0)

#define REG_PULSE_WIND                0x28
#define REG_PULSE_WIND_WIND7            (1 << 7)
#define REG_PULSE_WIND_WIND6            (1 << 6)
#define REG_PULSE_WIND_WIND5            (1 << 5)
#define REG_PULSE_WIND_WIND4            (1 << 4)
#define REG_PULSE_WIND_WIND3            (1 << 3)
#define REG_PULSE_WIND_WIND2            (1 << 2)
#define REG_PULSE_WIND_WIND1            (1 << 1)
#define REG_PULSE_WIND_WIND0            (1 << 0)

#define REG_ASLP_COUNT                0x29

#define REG_CTRL1                     0x2a
#define REG_CTRL1_ASLP_RATE1            (1 << 7)
#define REG_CTRL1_ASLP_RATE0            (1 << 6)
#define REG_CTRL1_DR2                   (1 << 5)
#define REG_CTRL1_DR1                   (1 << 4)
#define REG_CTRL1_DR0                   (1 << 3)
#define REG_CTRL1_LNOISE                (1 << 2)
#define REG_CTRL1_F_READ                (1 << 1)
#define REG_CTRL1_ACTIVE                (1 << 0)

#define REG_CTRL2                     0x2b
#define REG_CTRL2_ST                    (1 << 7)
#define REG_CTRL2_RST                   (1 << 6)
#define REG_CTRL2_SMODS1                (1 << 4)
#define REG_CTRL2_SMODS0                (1 << 3)
#define REG_CTRL2_SLPE                  (1 << 2)
#define REG_CTRL2_MODS1                 (1 << 1)
#define REG_CTRL2_MODS0                 (1 << 0)

#define REG_CTRL3                     0x2c
#define REG_CTRL3_WAKE_TRANS            (1 << 6)
#define REG_CTRL3_WAKE_LNDPRT           (1 << 5)
#define REG_CTRL3_WAKE_PULSE            (1 << 4)
#define REG_CTRL3_WAKE_FF_MT            (1 << 3)
#define REG_CTRL3_IPOL                  (1 << 1)
#define REG_CTRL3_PP_OD                 (1 << 0)

#define REG_CTRL4                     0x2d
#define REG_CTRL4_INT_EN_ASLP           (1 << 7)
#define REG_CTRL4_INT_EN_TRANS          (1 << 5)
#define REG_CTRL4_INT_EN_LNDPRT         (1 << 4)
#define REG_CTRL4_INT_EN_PULSE          (1 << 3)
#define REG_CTRL4_INT_EN_FF_MT          (1 << 2)
#define REG_CTRL4_INT_EN_DRDY           (1 << 0)

/* Set these bits to route an ISR to IRQ2 rather than IRQ1 */
#define REG_CTRL5                     0x2e
#define REG_CTRL5_INT_CFG_ASLP          (1 << 7)
#define REG_CTRL5_INT_CFG_TRANS         (1 << 5)
#define REG_CTRL5_INT_CFG_LNDPRT        (1 << 4)
#define REG_CTRL5_INT_CFG_PULSE         (1 << 3)
#define REG_CTRL5_INT_CFG_FF_MT         (1 << 2)
#define REG_CTRL5_INT_CFG_DRDY          (1 << 0)

static I2CDriver *driver;

event_source_t accel_x_axis_pulse;
event_source_t accel_y_axis_pulse;
event_source_t accel_z_axis_pulse;
event_source_t accel_freefall;
event_source_t accel_landscape_portrait;

static void accel_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, accelAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static uint8_t accel_get(uint8_t reg) {

  uint8_t val;

  i2cMasterTransmitTimeout(driver, accelAddr,
                           &reg, 1,
                           &val, 1,
                           TIME_INFINITE);
  return val;
}

static void accel_irq(void *port, int irq, int type) {

  uint8_t mask;

  (void)port;
  (void)irq;
  (void)type;

  i2cAcquireBus(driver);
  mask = accel_get(REG_INT_SRC);
  i2cReleaseBus(driver);

  if (mask & REG_INT_SRC_FF_MT) {
    i2cAcquireBus(driver);
    (void)accel_get(REG_FF_MT_SRC);
    i2cReleaseBus(driver);

    chEvtBroadcast(&accel_freefall);
  }

  if (mask & REG_INT_SRC_LNDPRT) {
    i2cAcquireBus(driver);
    (void)accel_get(REG_PL_STATUS);
    i2cReleaseBus(driver);

    chEvtBroadcast(&accel_landscape_portrait);
  }


  if (mask & REG_INT_SRC_PULSE) {
    uint8_t pulsemask;

    i2cAcquireBus(driver);
    pulsemask = accel_get(REG_PULSE_SRC);
    i2cReleaseBus(driver);

    if (pulsemask & REG_PULSE_SRC_AXX)
      chEvtBroadcast(&accel_x_axis_pulse);

    if (pulsemask & REG_PULSE_SRC_AXY)
      chEvtBroadcast(&accel_y_axis_pulse);

    if (pulsemask & REG_PULSE_SRC_AXZ)
      chEvtBroadcast(&accel_z_axis_pulse);
  }
}

void accelEnableFreefall(int sensitivity, int debounce) {

  i2cAcquireBus(driver);

  /* Put the accelerometer into "Standby" mode to program registers */
  while (accel_get(REG_CTRL1) & REG_CTRL1_ACTIVE)
    accel_set(REG_CTRL1, 0);
  
  /* Enable motion detect on all axes, freefall with no event latch */
  accel_set(REG_FF_MT_CFG, REG_FF_MT_CFG_OAE
                         | REG_FF_MT_CFG_ELE
                         | REG_FF_MT_CFG_XEFE
                         | REG_FF_MT_CFG_YEFE
                         | REG_FF_MT_CFG_ZEFE);

  /* Disable autosleep */
  accel_set(REG_CTRL2, 0);

  /* Enable portrait/landscape detection */
  accel_set(REG_PL_CFG, REG_PL_CFG_DBCNTM | REG_PL_CFG_PL_EN);

  /* Set dynamic range to +/- 2g */
  accel_set(REG_XYZ_DATA_CFG, REG_XYZ_DATA_CFG_FS_2G);

  /* Set motion to be really, really sensitive */
  accel_set(REG_FF_MT_THS, sensitivity);

  /* Set a debounce count for freefall */
  accel_set(REG_FF_MT_COUNT, debounce);

  /* Pick INT2 for the IRQs we want to enable */
  accel_set(REG_CTRL5, REG_CTRL5_INT_CFG_FF_MT | REG_CTRL5_INT_CFG_LNDPRT);

  /* Enable accelerometer IRQs */
  accel_set(REG_CTRL4, REG_CTRL4_INT_EN_FF_MT | REG_CTRL4_INT_EN_LNDPRT);

  /* Read the register to clear pending events */
  (void)accel_get(REG_FF_MT_SRC);
  (void)accel_get(REG_PL_STATUS);
  (void)accel_get(REG_PULSE_SRC);

  /* Re-enable the accelerometer */
  accel_set(REG_CTRL1, REG_CTRL1_ACTIVE);

  i2cReleaseBus(driver);
}

void accelDisableFreefall(void) {

  i2cAcquireBus(driver);

  /* Put the accelerometer into "Standby" mode */
  accel_set(REG_CTRL1, 0);

  /* Enable wake-from-sleep IRQs */
  accel_set(REG_CTRL3, 0);
  accel_set(REG_CTRL4, 0);
  accel_set(REG_CTRL1, REG_CTRL1_ACTIVE);
  i2cReleaseBus(driver);
}

void accelStop(void) {
  i2cAcquireBus(driver);
  // forces accelerometer into standby mode
  accel_set(REG_CTRL1, accel_get(REG_CTRL1) & ~REG_CTRL1_ACTIVE);
  i2cReleaseBus(driver);
}


void accelStart(I2CDriver *i2cp) {

  driver = i2cp;

  i2cAcquireBus(driver);

  /* Reset the chip */
  accel_set(REG_CTRL2, REG_CTRL2_RST);
  while (accel_get(REG_CTRL2) & REG_CTRL2_RST); /* Busy-wait for reset */

  /* Activate the chip */
  while (!(accel_get(REG_CTRL1) & REG_CTRL1_ACTIVE))
    accel_set(REG_CTRL1, REG_CTRL1_ACTIVE);

  i2cReleaseBus(driver);

  chEvtObjectInit(&accel_x_axis_pulse);
  chEvtObjectInit(&accel_y_axis_pulse);
  chEvtObjectInit(&accel_z_axis_pulse);
  chEvtObjectInit(&accel_freefall);
  chEvtObjectInit(&accel_landscape_portrait);

  gpioxRegisterHandler(GPIOX, 3, accel_irq);
  gpioxSetPadMode(GPIOX, 3, GPIOX_IN | GPIOX_IRQ_FALLING);

  // enable freefall by default
  accelEnableFreefall(0x18, 50);
  
}

msg_t accelPoll(struct accel_data *data) {
  uint8_t tx[1];
  uint8_t rx[7];

  tx[0] = 0;

  i2cAcquireBus(driver);
  i2cMasterTransmitTimeout(driver, accelAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);
  i2cReleaseBus(driver);

#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
  data->x  = ((rx[1] & 0xff)) << 4;
  data->x |= ((rx[2] >> 4) & 0x0f);
  
  data->y  = ((rx[3] & 0xff)) << 4;
  data->y |= ((rx[4] >> 4) & 0x0f);
#else
  // x/y swapped
  data->x  = ((rx[3] & 0xff)) << 4;
  data->x |= ((rx[4] >> 4) & 0x0f);
  
  data->y  = ((rx[1] & 0xff)) << 4;
  data->y |= ((rx[2] >> 4) & 0x0f);

  data->y = 4095 - data->y;  // y axis inverted
#endif
  
  data->z  = ((rx[5] & 0xff)) << 4;
  data->z |= ((rx[6] >> 4) & 0x0f);

  return MSG_OK;
}

OrchardTestResult test_accel(const char *my_name, OrchardTestType test_type) {
  (void) my_name;
  uint8_t ret;
  
  switch(test_type) {
  case orchardTestPoweron:
  case orchardTestTrivial:
    i2cAcquireBus(driver);
    ret =  accel_get(REG_WHO_AM_I);
    i2cReleaseBus(driver);
    if( ret != 0x2A ) {
      return orchardResultFail;
    } else {
      return orchardResultPass;
    }
    break;
  default:
    return orchardResultNoTest;
  }
  
  return orchardResultNoTest;
}
orchard_test("accel", test_accel);
