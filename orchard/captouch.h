#ifndef __CAPTOUCH_H__
#define __CAPTOUCH_H__

/*
    MPR121.h
	April 8, 2010
	by: Jim Lindblom
*/

// MPR121 Register Defines
#define ELE_OORL 0x02
#define ELE_OORH 0x03

#define ELE_TCHL 0x00
#define ELE_TCHH 0x01

#define MHD_R	0x2B
#define NHD_R	0x2C
#define	NCL_R 	0x2D
#define	FDL_R	0x2E
#define	MHD_F	0x2F
#define	NHD_F	0x30
#define	NCL_F	0x31
#define	FDL_F	0x32
#define	ELE0_T	0x41
#define	ELE0_R	0x42
#define	ELE1_T	0x43
#define	ELE1_R	0x44
#define	ELE2_T	0x45
#define	ELE2_R	0x46
#define	ELE3_T	0x47
#define	ELE3_R	0x48
#define	ELE4_T	0x49
#define	ELE4_R	0x4A
#define	ELE5_T	0x4B
#define	ELE5_R	0x4C
#define	ELE6_T	0x4D
#define	ELE6_R	0x4E
#define	ELE7_T	0x4F
#define	ELE7_R	0x50
#define	ELE8_T	0x51
#define	ELE8_R	0x52
#define	ELE9_T	0x53
#define	ELE9_R	0x54
#define	ELE10_T	0x55
#define	ELE10_R	0x56
#define	ELE11_T	0x57
#define	ELE11_R	0x58
#define TCH_DBNC 0x5B
#define	FIL_CFG	0x5D
#define	ELE_CFG	0x5E
#define GPIO_CTRL0	0x73
#define	GPIO_CTRL1	0x74
#define GPIO_DATA	0x75
#define	GPIO_DIR	0x76
#define	GPIO_EN		0x77
#define	GPIO_SET	0x78
#define	GPIO_CLEAR	0x79
#define	GPIO_TOGGLE	0x7A
#define	ATO_CFG_CTL0	0x7B
#define	ATO_CFG_CTL1	0x7C
#define	ATO_CFG_USL	0x7D
#define	ATO_CFG_LSL	0x7E
#define	ATO_CFG_TGT	0x7F

// Global Constants
#if KEY_LAYOUT == LAYOUT_BM
// direct touches to the PCB have a much larger signature over baseline
// so we can use this to our advantage to filter ambient noise
#define TOU_THRESH      0x18 
#define REL_THRESH      0x1C
#else
// touches through the PC case are fairly small; but the case also filters
// out most ambient noise so we can be more sensitive
#define TOU_THRESH	0x08
#define	REL_THRESH	0x0C
#endif

void captouchStart(I2CDriver *i2cp);
void captouchStop(void);
uint16_t captouchRead(void);

void captouchDebug(void);
void captouchPrint(uint8_t reg);
void captouchSet(uint8_t adr, uint8_t dat);
uint8_t captouchGet(uint8_t reg);
void captouchRecal(void);
void captouchFastBaseline(void);
void captouchCalibrate(void);

extern event_source_t captouch_changed;

#endif /* __CAPTOUCH_H__ */
