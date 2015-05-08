/************************************************************************************
* This is a template header file.
*
*
* (c) Copyright 2012, Freescale Semiconductor, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
************************************************************************************/


 

#define	RADIO_Fifo	       	   	0x00	/*	FIFO	read/write	access							0x00		*/
#define	RADIO_OpMode	       	0x01	/*	Operating	modes	of	the	transceiver					0x04		*/
#define	RADIO_DataModul		0x02	/*	Data	operation	mode	and	Modulation	settings				0x00		*/
#define	RADIO_BitrateMsb	    0x03	/*	Bit	Rate	setting,	Most	Significant	Bits				0x1A		*/
#define	RADIO_BitrateLsb	    0x04	/*	Bit	Rate	setting,	Least	Significant	Bits				0x0B		*/
#define	RADIO_FdevMsb	      	0x05	/*	Frequency	Deviation	setting,	Most	Significant	Bits				0x00		*/
#define	RADIO_FdevLsb	     	0x06	/*	Frequency	Deviation	setting,	Least	Significant	Bits				0x52		*/
#define	RADIO_FrfMsb	        0x07	/*	RF	Carrier	Frequency,	Most	Significant	Bits				0xE4		*/
#define	RADIO_FrfMid	        0x08	/*	RF	Carrier	Frequency,	Intermediate	Bits					0xC0		*/
#define	RADIO_FrfLsb	        0x09	/*	RF	Carrier	Frequency,	Least	Significant	Bits				0x00		*/
#define	RADIO_Osc1	          	0x0A	/*	RC	Oscillators	Settings							0x41		*/
#define	RADIO_AfcCtrl	     	0x0B	/*	AFC	control	in	low	modulation	index	situations			0x00		*/
#define	RADIO_LowBat	        0x0C	/*	Low	Battery	Indicator	Settings						0x02		*/
#define	RADIO_Listen1	      	0x0D	/*	Listen	Mode	settings							0x92		*/
#define	RADIO_Listen2	      	0x0E	/*	Listen	Mode	Idle	duration						0xF5		*/
#define	RADIO_Listen3	      	0x0F	/*	Listen	Mode	Rx	duration						0x20		*/
#define	RADIO_Version	      	0x10	/*	Semtech	ID	relating	the	silicon	revision				0x22		*/
#define	RADIO_PaLevel	      	0x11	/*	PA	selection	and	Output	Power	control				0x9F		*/
#define	RADIO_PaRamp	        0x12	/*	Control	of	the	PA	ramp	time	in	FSK	mode	0x09		*/
#define	RADIO_Ocp	          	0x13	/*	Over	Current	Protection	control						0x1A		*/
#define	RADIO_Reserved14	    0x14	/*	-									0x40		*/
#define	RADIO_Reserved15	    0x15	/*	-									0xB0		*/
#define	RADIO_Reserved16	    0x16	/*	-									0x7B		*/
#define	RADIO_Reserved17	    0x17	/*	-									0x9B		*/
#define	RADIO_Lna	          	0x18	/*	LNA	settings								0x08	0x88	*/
#define	RADIO_RxBw	          	0x19	/*	Channel	Filter	BW	Control						0x86	0x55	*/
#define	RADIO_AfcBw	        0x1A	/*	Channel	Filter	BW	control	during	the	AFC	routine		0x8A	0x8B	*/
#define	RADIO_OokPeak	      	0x1B	/*	OOK	demodulator	selection	and	control	in	peak	mode		0x40		*/
#define	RADIO_OokAvg	        0x1C	/*	Average	threshold	control	of	the	OOK	demodulator			0x80		*/
#define	RADIO_OokFix	        0x1D	/*	Fixed	threshold	control	of	the	OOK	demodulator			0x06		*/
#define	RADIO_AfcFei	        0x1E	/*	AFC	and	FEI	control	and	status				0x10		*/
#define	RADIO_AfcMsb	        0x1F	/*	MSB	of	the	frequency	correction	of	the	AFC		0x00		*/
#define	RADIO_AfcLsb	        0x20	/*	LSB	of	the	frequency	correction	of	the	AFC		0x00		*/
#define	RADIO_FeiMsb	        0x21	/*	MSB	of	the	calculated	frequency	error				0x00		*/
#define	RADIO_FeiLsb	        0x22	/*	LSB	of	the	calculated	frequency	error				0x00		*/
#define	RADIO_RssiConfig	    0x23	/*	RSSI-related	settings								0x02		*/
#define	RADIO_RssiValue	    0x24	/*	RSSI	value	in	dBm						0xFF		*/
#define	RADIO_DioMapping1	  	0x25	/*	Mapping	of	pins	DIO0	to	DIO3				0x00		*/
#define	RADIO_DioMapping2	  	0x26	/*	Mapping	of	pins	DIO4	and	DIO5,	ClkOut	frequency		0x05	0x07	*/
#define	RADIO_IrqFlags1	    0x27	/*	Status	register:	PLL	Lock	state,	Timeout,	RSSI	>	Threshold...	0x80		*/
#define	RADIO_IrqFlags2	    0x28	/*	Status	register:	FIFO	handling	flags,	Low	Battery	detection...		0x00		*/
#define	RADIO_RssiThresh	    0x29	/*	RSSI	Threshold	control							0xFF	0xE4	*/
#define	RADIO_RxTimeout1	    0x2A	/*	Timeout duration	between	Rx	request	and	RSSI	detection		0x00		*/
#define	RADIO_RxTimeout2	    0x2B	/*	Timeout	duration	between	RSSI	detection	and	PayloadReady			0x00		*/
#define	RADIO_PreambleMsb	  	0x2C	/*	Preamble	length,	MSB							0x00		*/
#define	RADIO_PreambleLsb	  	0x2D	/*	Preamble	length,	LSB							0x03		*/

#define	RADIO_SyncConfig	    0x2E	/*	Sync	Word	Recognition	control						0x98		*/
#define	RADIO_SyncConfig_SyncOn                   (1 << 7)
#define	RADIO_SyncConfig_FifoFillCondition        (1 << 6)
#define	RADIO_SyncConfig_SyncSize_Mask            (7 << 3)
#define	RADIO_SyncConfig_SyncSize_Shift           3
#define	RADIO_SyncConfig_SyncTol_Mask             (7 << 0)
#define	RADIO_SyncConfig_SyncTol_Shift            0

#define	RADIO_SyncValue1	    0x2F	/*	Sync	Word	bytes,	1	through	8				0x00	0x01	*/
#define	RADIO_SyncValue2	    0x30	/*										0x00		*/
#define	RADIO_SyncValue3	    0x31	/*										0x00		*/
#define	RADIO_SyncValue4	    0x32	/*										0x00		*/
#define	RADIO_SyncValue5	    0x33	/*										0x00		*/
#define	RADIO_SyncValue6	    0x34	/*										0x00		*/
#define	RADIO_SyncValue7	    0x35	/*										0x00		*/
#define	RADIO_SyncValue8	    0x36	/*										0x00		*/

#define	RADIO_PacketConfig1	0x37	/*	Packet	mode	settings							0x10		*/
#define	RADIO_PayloadLength	0x38	/*	Payload	length	setting							0x40		*/
#define	RADIO_NodeAddress               0x39	/*	Node	address								0x00		*/
#define	RADIO_BroadcastAddress          0x3A	/*	Broadcast	address								0x00		*/
#define	RADIO_AutoModes	    0x3B	/*	Auto	modes	settings							0x00		*/
#define	RADIO_FifoThresh	    0x3C	/*	Fifo	threshold,	Tx	start	condition					0x0F	0x8F	*/
#define	RADIO_PacketConfig2	0x3D	/*	Packet	mode	settings							0x02		*/
#define	RADIO_AesKey1	      	0x3E	/*	16	bytes	of	the	cypher	key				0x00		*/
#define	RADIO_AesKey2	      	0x3F	/*										0x00		*/
#define	RADIO_AesKey3	      	0x40	/*										0x00		*/
#define	RADIO_AesKey4	      	0x41	/*										0x00		*/
#define	RADIO_AesKey5	      	0x42	/*										0x00		*/
#define	RADIO_AesKey6	      	0x43	/*										0x00		*/
#define	RADIO_AesKey7	      	0x44	/*										0x00		*/
#define	RADIO_AesKey8	      	0x45	/*										0x00		*/
#define	RADIO_AesKey9	      	0x46	/*										0x00		*/
#define	RADIO_AesKey10	      	0x47	/*										0x00		*/
#define	RADIO_AesKey11	      	0x48	/*										0x00		*/
#define	RADIO_AesKey12	      	0x49	/*										0x00		*/
#define	RADIO_AesKey13	      	0x4A	/*										0x00		*/
#define	RADIO_AesKey14	      	0x4B	/*										0x00		*/
#define	RADIO_AesKey15	      	0x4C	/*										0x00		*/
#define	RADIO_AesKey16	      	0x4D	/*										0x00		*/
#define	RADIO_Temp1	        0x4E	/*	Temperature	Sensor	control							0x01		*/
#define	RADIO_Temp2	        0x4F	/*	Temperature	readout								0x00		*/
#define	RADIO_TestLna	      	0x58	/*	Sensitivity	boost								0x1B		*/
#define	RADIO_TestAfc	      	0x71	/*	AFC	offset	for	low	modulation	index	AFC			0x00		*/
#define RADIO_Calib          	0x57  /*  Used in calibration procedure for V2a chip version*/
#define RADIO_TestPLL_BW            0x5F    /* Added, PLL Bandwidth setting   */ /*JAPAN CFG*/
#define RADIO_TestDagc       	0x6F  /*  Fading margin improvement. Added for extended PHY. */

/* RegDioMapping1 (0x25) */
/* |7  6|5  4|3  2|1  0| */
/* |DIO0|DIO1|DIO2|DIO3| */

/* RegDioMapping2 (0x26) */
/* |7  6|5  4|3 |2    0| */
/* |DIO4|DIO5|0 |ClkOut| */

#define ClkOutFxOsc_Div1        (0x00)
#define ClkOutFxOsc_Div2        (0x01)
#define ClkOutFxOsc_Div4        (0x02)
#define ClkOutFxOsc_Div8        (0x03)
#define ClkOutFxOsc_Div16       (0x04)
#define ClkOutFxOsc_Div32       (0x05)
#define ClkOutRC                (0x06)       
#define ClkOutOff               (0x07)

#define DIO0_RxCrkOk          (0x00 << 6)
#define DIO0_RxPayloadReady   (0x01 << 6)
#define DIO0_RxSyncAddress    (0x02 << 6)
#define DIO0_RxRssi           (0x03 << 6)

#define DIO1_RxFifoLevel      (0x00 << 4)
#define DIO1_RxFifoFull       (0x01 << 4)
#define DIO1_RxFifoNotEmpty   (0x02 << 4)
#define DIO1_RxTimeout        (0x03 << 4)

#define DIO2_RxFifoNotEmpty   (0x00 << 2)
#define DIO2_RxData           (0x01 << 2)
#define DIO2_RxLowBat         (0x02 << 2)
#define DIO2_RxAutoMode       (0x03 << 2)

#define DIO3_RxFifoFull       (0x00 << 0)
#define DIO3_RxRssi           (0x01 << 0)
#define DIO3_RxSyncAddres     (0x02 << 0)
#define DIO3_RxPllLock        (0x03 << 0)

#define DIO4_RxTimeout        (0x00 << 6)
#define DIO4_RxRssi           (0x01 << 6)
#define DIO4_RxRxReady        (0x02 << 6)
#define DIO4_RxPllLock        (0x03 << 6)

#define DIO5_RxClkOut         (0x00 << 4)
#define DIO5_RxData           (0x01 << 4)
#define DIO5_RxLowBat         (0x02 << 4)
#define DIO5_RxModeReady      (0x03 << 4)


#define DIO0_TxPacketSent     (0x00 << 6)
#define DIO0_TxTxReady        (0x01 << 6)
#define DIO0_TxLowBat         (0x02 << 6)
#define DIO0_TxPllLock        (0x03 << 6)

#define DIO1_TxFifoLevel      (0x00 << 4)
#define DIO1_TxFifoFull       (0x01 << 4)
#define DIO1_TxFifoNotEmpty   (0x02 << 4)
#define DIO1_TxTimeout        (0x03 << 4)

#define DIO2_TxFifoNotEmpty   (0x00 << 2)
#define DIO2_TxData           (0x01 << 2)
#define DIO2_TxLowBat         (0x02 << 2)
#define DIO2_TxAutoMode       (0x03 << 2)

#define DIO3_TxFifoFull       (0x00 << 0)
#define DIO3_TxTxReady        (0x01 << 0)
#define DIO3_TxLowBat         (0x02 << 0)
#define DIO3_TxPllLock        (0x03 << 0)

#define DIO4_TxModeReady      (0x00 << 6)
#define DIO4_TxTxReady        (0x01 << 6)
#define DIO4_TxLowBat         (0x02 << 6)
#define DIO4_TxPllLock        (0x03 << 6)

#define DIO5_TxClkOut         (0x00 << 4)
#define DIO5_TxData           (0x01 << 4)
#define DIO5_TxLowBat         (0x02 << 4)
#define DIO5_TxModeReady      (0x03 << 4)

/* OpMode */
#define OpMode_Sequencer_Off  (0x01 << 7)
#define OpMode_Sequencer_On   (0x00 << 7)

#define OpMode_Listen_Off     (0x00 << 6)
#define OpMode_Listen_On      (0x01 << 6)
#define OpMode_Listen_Abort   (0x01 << 5)

#define OpMode_Sleep          (0x00 << 2)
#define OpMode_StandBy        (0x01 << 2)
#define OpMode_FreqSynt       (0x02 << 2)
#define OpMode_Transmitter    (0x03 << 2)
#define OpMode_Receiver       (0x04 << 2)

/* DataModul */
#define DataModul_DataMode_Mask                               (3 << 5)
#define DataModul_DataMode_Packet                             (0 << 5)
#define DataModul_DataMode_Continous                          (2 << 5)
#define DataModul_DataMode_ContinousNoBitSync                 (3 << 5)
#define DataModul_Modulation_Mask                             (3 << 3)
#define DataModul_Modulation_Fsk                              (0 << 3)
#define DataModul_Modulation_Ook                              (1 << 3)
#define DataModul_ModulationShaping_Mask                      (3 << 0)
#define DataModul_ModulationShaping_NoShaping                 (0 << 0)
#define DataModul_ModulationShaping_BT_1                      (1 << 0)
#define DataModul_ModulationShaping_BT_05                     (2 << 0)
#define DataModul_ModulationShaping_BT_03                     (3 << 0)

/*Calculations*/
/******CMA comments*********
FRF calculation
* Frf = Fstep × Frf(23;0)

Frf(23;0)=Frf/Fstep
****************************
Fsteps corresponding values:
57.2204  // 30.0 MHz   CLKOUT
61.0352  // 32.0 MHz   CLKOUT
****************************/

/*NAM default 915MHz*/
#define FrfMsb_915                               0xE4  // Default
#define FrfMid_915                               0xC0  // Default
#define FrfLsb_915                               0x00  // Default

/*EMEA default 868MHz*/
#define FrfMsb_868                               0xD8  // Default
#define FrfMid_868                               0xFF  // Default
#define FrfLsb_868                               0xF5  // Default

/*India default 865MHz*/
#define FrfMsb_865                               0xD8  // Default
#define FrfMid_865                               0x3F  // Default
#define FrfLsb_865                               0xF5  // Default

/*China default 470MHz*/
#define FrfMsb_470                               0x75  // Default
#define FrfMid_470                               0x7F  // Default
#define FrfLsb_470                               0xFA  // Default

/*India default 434MHz*/
#define FrfMsb_434                               0x6C  // Default
#define FrfMid_434                               0x7F  // Default
#define FrfLsb_434                               0xFA  // Default

/*Japan default 920MHz*/ /*30MHz CLK, Fstep=57.2204*/                           /*920.6 MHz*/
#define FrfMsb_920                               0xF5  // Default
#define FrfMid_920                               0x7E  // Default
#define FrfLsb_920                               0x4B  // Default

#define BitrateMsb_4800   0x1A
#define BitrateLsb_4800   0x0B

#define BitrateMsb_38400  0x03
#define BitrateLsb_38400  0x41

#define BitrateMsb_10000  0x0C
#define BitrateLsb_10000  0x80

#define BitrateMsb_20000  0x06
#define BitrateLsb_20000  0x40

#define BitrateMsb_40000  0x03
#define BitrateLsb_40000  0x20

#define BitrateMsb_50000  0x02
#define BitrateLsb_50000  0x80

#define BitrateMsb_100000 0x01
#define BitrateLsb_100000 0x40

#define BitrateMsb_150000 0x00
#define BitrateLsb_150000 0xD5

#define BitrateMsb_200000 0x00
#define BitrateLsb_200000 0xA0

#define Bitrate_4800	  0x1A0B
#define Bitrate_38400     0x0341
#define Bitrate_50000     0x0280
#define Bitrate_100000    0x0140
#define Bitrate_150000    0x00D5
#define Bitrate_200000    0x00A0


/* Fdev */
// 4.8 Kbps, modulation index 1
#define FdevMsb_2400   0x00
#define FdevLsb_2400   0x27
// 10 Kbps, modulation index 0.5
#define FdevMsb_2500   0x00
#define FdevLsb_2500   0x29
// 20 Kbps, modulation index 0.5
#define FdevMsb_5000   0x00
#define FdevLsb_5000   0x52
// 40 Kbps, modulation index 0.5
#define FdevMsb_10000  0x00
#define FdevLsb_10000  0xA4
// 50 Kbps, modulation index 1
#define FdevMsb_25000  0x01
#define FdevLsb_25000  0x9A
// 150 Kbps, modulation index 0.5
#define FdevMsb_37500  0x02
#define FdevLsb_37500  0x68
// 100 Kbps, modulation index 1
#define FdevMsb_50000  0x03
#define FdevLsb_50000  0x33
// 200 Kbps, modulation index 0.5 !!!!!
#define FdevMsb_50049  0x03
#define FdevLsb_50049  0x34
// 200 Kbps, modulation index 1
#define FdevMsb_100000 0x06
#define FdevLsb_100000 0x66

#define FdevMsb_19000  0x01
#define FdevLsb_19000  0x37

#define Fdev_5000      0x0052
#define Fdev_19000     0x0137
#define Fdev_25000     0x019A
#define Fdev_50000     0x0333
#define Fdev_50049     0x0334
#define Fdev_100000    0x0666
#define Fdev_170000    0x0AE1
#define Fdev_180000    0x0B85



/* Frf */

// 450-470 MHz
// Channel spacing               12.5 KHz
// GL = GH = Channel Spacing / 2 6.25 KHz
// Total number of channels      1599

/* Channel bandwidth control */

#define DccFreq_0   (0x00 << 5)
#define DccFreq_1   (0x01 << 5)
#define DccFreq_2   (0x02 << 5)
#define DccFreq_3   (0x03 << 5)
#define DccFreq_4   (0x04 << 5)
#define DccFreq_5   (0x05 << 5)
#define DccFreq_6   (0x06 << 5)
#define DccFreq_7   (0x07 << 5)

#define RxBwMant_0  (0x00 << 3)
#define RxBwMant_1  (0x01 << 3)
#define RxBwMant_2  (0x02 << 3)

#define RxBwExp_0   (0x00 << 0)
#define RxBwExp_1   (0x01 << 0)
#define RxBwExp_2   (0x02 << 0)
#define RxBwExp_3   (0x03 << 0)
#define RxBwExp_4   (0x04 << 0)
#define RxBwExp_5   (0x05 << 0)
#define RxBwExp_6   (0x06 << 0)
#define RxBwExp_7   (0x07 << 0)

#define RxBw_2600   ( (RxBwExp_7) | (RxBwMant_2) )
#define RxBw_3100   ( (RxBwExp_7) | (RxBwMant_1) )
#define RxBw_3900   ( (RxBwExp_7) | (RxBwMant_0) )

#define RxBw_5200   ( (RxBwExp_6) | (RxBwMant_2) )
#define RxBw_6300   ( (RxBwExp_6) | (RxBwMant_1) )
#define RxBw_7800   ( (RxBwExp_6) | (RxBwMant_0) )

#define RxBw_10400  ( (RxBwExp_5) | (RxBwMant_2) )
#define RxBw_12500  ( (RxBwExp_5) | (RxBwMant_1) )
#define RxBw_15600  ( (RxBwExp_5) | (RxBwMant_0) )

#define RxBw_20800  ( (RxBwExp_4) | (RxBwMant_2) )
#define RxBw_25000  ( (RxBwExp_4) | (RxBwMant_1) )
#define RxBw_31300  ( (RxBwExp_4) | (RxBwMant_0) )

#define RxBw_41700  ( (RxBwExp_3) | (RxBwMant_2) )
#define RxBw_50000  ( (RxBwExp_3) | (RxBwMant_1) )
#define RxBw_62500  ( (RxBwExp_3) | (RxBwMant_0) )

#define RxBw_83300  ( (RxBwExp_2) | (RxBwMant_2) )
#define RxBw_100000 ( (RxBwExp_2) | (RxBwMant_1) )
#define RxBw_125000 ( (RxBwExp_2) | (RxBwMant_0) )

#define RxBw_166700 ( (RxBwExp_1) | (RxBwMant_2) )
#define RxBw_200000 ( (RxBwExp_1) | (RxBwMant_1) )
#define RxBw_250000 ( (RxBwExp_1) | (RxBwMant_0) )

#define RxBw_333300 ( (RxBwExp_0) | (RxBwMant_2) )
#define RxBw_400000 ( (RxBwExp_0) | (RxBwMant_1) )
#define RxBw_500000 ( (RxBwExp_0) | (RxBwMant_0) )


/* AfcCtrl */

#define AfcCtrl_AfcLowBeta_On   (0x01 << 5)
#define AfcCtrl_AfcLowBeta_Off  (0x00)

/* PaLevel */

#define PaLevel_Pa0_On      (0x01 << 7)
#define PaLevel_Pa0_Off     (0x00)

#define PaLevel_Pa1_On      (0x01 << 6)
#define PaLevel_Pa1_Off     (0x00)

#define PaLevel_Pa2_On      (0x01 << 5)
#define PaLevel_Pa2_Off     (0x00)

#define PaLevel_Pa1_Pa2_On      (0x60)
#define PaLevel_Pa1_Pa2_Off     (0x00)

#define PA0_On              (0x01 << 0)
#define PA1_On              (0x01 << 1)
#define PA2_On              (0x01 << 2)


/* PaRamp */

#define PaRamp_3400 (0x00)
#define PaRamp_2000 (0x01)
#define PaRamp_1000 (0x02)
#define PaRamp_500  (0x03)
#define PaRamp_250  (0x04)
#define PaRamp_125  (0x05)
#define PaRamp_100  (0x06)
#define PaRamp_62   (0x07)
#define PaRamp_50   (0x08)
#define PaRamp_40   (0x09)
#define PaRamp_31   (0x0A)
#define PaRamp_25   (0x0B)
#define PaRamp_20   (0x0C)
#define PaRamp_15   (0x0D)
#define PaRamp_12   (0x0E)
#define PaRamp_10   (0x0F)

/* RegOcp */

#define Ocp_Ocp_On  (0x01 << 4)
#define Ocp_Ocp_Off (0x00)

/* RegLna */

#define Lna_LnaZin_200  (0x01 << 7)
#define Lna_LnaZin_50 (0x00)

#define Lna_LnaGain_Agc          (0x00)
#define Lna_LnaGain_MaxGain      (0x01)
#define Lna_LnaGain_MaxGain_6    (0x02)
#define Lna_LnaGain_MaxGain_12   (0x04)
#define Lna_LnaGain_MaxGain_24   (0x05)
#define Lna_LnaGain_MaxGain_36   (0x06)
#define Lna_LnaGain_MaxGain_48   (0x07)

/* RegAfcFei */

#define AfcFei_FeiDone           (0x01 << 6) //read only
#define AfcFei_FeiStart          (0x01 << 5)

#define AfcFei_AfcDone           (0x01 << 4) //read only
#define AfcFei_AfcAutoClear_Off  (0x00)
#define AfcFei_AfcAutoClear_On   (0x01 << 3)
#define AfcFei_AfcAuto_Off       (0x00)
#define AfcFei_AfcAuto_On        (0x01 << 2)
#define AfcFei_AfcClear          (0x01 << 1)
#define AfcFei_AfcStart          (0x01 << 0)


/* RegRssi */

#define Rssi_RssiDone  (0x01 << 1) //read only
#define Rssi_RssiStart (0x01 << 0)

/* RegSyncConfig */

#define SyncConfig_Sync_On                    (0x01 << 7)
#define SyncConfig_Sync_Off                   (0x00)

#define SyncConfig_FifioFill_ifSyncAddres     (0x00)
#define SyncConfig_FifioFill_ifFifoFillisSet  (0x01 << 6)

#define SyncConfig_SyncSize_1                 (0x00 << 3)
#define SyncConfig_SyncSize_2                 (0x01 << 3)
#define SyncConfig_SyncSize_3                 (0x02 << 3)
#define SyncConfig_SyncSize_4                 (0x03 << 3)
#define SyncConfig_SyncSize_5                 (0x04 << 3)
#define SyncConfig_SyncSize_6                 (0x05 << 3)
#define SyncConfig_SyncSize_7                 (0x06 << 3)
#define SyncConfig_SyncSize_8                 (0x07 << 3)

/* RegPacketConfig1 */

#define PacketConfig1_PacketFormat_Fixed_Length             (0 << 7)
#define PacketConfig1_PacketFormat_Variable_Length          (1 << 7)

#define PacketConfig1_DcFree_Mask                           (3 << 5)
#define PacketConfig1_DcFree_Shift                          5
#define PacketConfig1_DcFree_None                           (0 << 5)
#define PacketConfig1_DcFree_Manchester                     (1 << 5)
#define PacketConfig1_DcFree_Whitening                      (2 << 5)

#define PacketConfig1_Crc_Off                               (0 << 4)
#define PacketConfig1_Crc_On                                (1 << 4)

#define PacketConfig1_CrcAutoClear_On                       (0 << 3)
#define PacketConfig1_CrcAutoClear_Off                      (1 << 3)

#define PacketConfig1_AddresFiltering_Off                   (0 << 1)
#define PacketConfig1_AddresFiltering_Node                  (1 << 1)
#define PacketConfig1_AddresFiltering_Node_Or_Broadcast     (2 << 1)

/* RegPacketConfig2 */

#define PacketConfig2_Aes_Off                               (0 << 0)
#define PacketConfig2_Aes_On                                (1 << 0)

#define PacketConfig2_AutoRxRestart_Off                     (0 << 1)
#define PacketConfig2_AutoRxRestart_On                      (1 << 1)

#define PacketConfig2_RxRestart                             (1 << 2)

/* Reg_IrqFlags1 */
#define IrqFlags1_ModeReady                                 (1 << 7)
#define IrqFlags1_RxReady                                   (1 << 6)
#define IrqFlags1_TxReady                                   (1 << 5)
#define IrqFlags1_PllLock                                   (1 << 4)
#define IrqFlags1_Rssi                                      (1 << 3)
#define IrqFlags1_Timeout                                   (1 << 2)
#define IrqFlags1_AutoMode                                  (1 << 1)
#define IrqFlags1_SyncAddressMatch                          (1 << 0)

#define IrqFlags2_FifoFull                                  (1 << 7)
#define IrqFlags2_FifoNotEmpty                              (1 << 6)
#define IrqFlags2_FifoLevel                                 (1 << 5)
#define IrqFlags2_FifoOverrun                               (1 << 4)
#define IrqFlags2_PacketSent                                (1 << 3)
#define IrqFlags2_PayloadReady                              (1 << 2)
#define IrqFlags2_CrcOk                                     (1 << 1)
#define IrqFlags2_LowBat                                    (1 << 0)







/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Level 1 block comment
*************************************************************************************
************************************************************************************/

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// Level 2 block comment
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

/* Level 3 block comment */




// Delimiters

/***********************************************************************************/

//-----------------------------------------------------------------------------------


