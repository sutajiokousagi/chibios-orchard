/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    KL1x/kinetis_registry.h
 * @brief   KL1x capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _KINETIS_REGISTRY_H_
#define _KINETIS_REGISTRY_H_

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    KL1x capabilities
 * @{
 */

/* EXT attributes.*/
#define KINETIS_PORTA_IRQ_VECTOR    VectorB8
#define KINETIS_PORTD_IRQ_VECTOR    VectorBC

/* ADC attributes.*/
#define KINETIS_HAS_ADC0            TRUE
#define KINETIS_ADC0_IRC_VECTOR     Vector7C

/* I2C attributes.*/
#define KINETIS_I2C0_IRQ_VECTOR     Vector60
#define KINETIS_I2C1_IRQ_VECTOR     Vector64

/* UART attributes.*/
#define KINETIS_UART0_IRQ_VECTOR    Vector70
#define KINETIS_UART1_IRQ_VECTOR    Vector74
#define KINETIS_UART2_IRQ_VECTOR    Vector78

/* SPI attributes.*/
#define KINETIS_SPI0_IRQ_VECTOR     Vector68
#define KINETIS_SPI1_IRQ_VECTOR     Vector6C

/* RTC attributes.*/
#define KINETIS_RTCA_IRQ_VECTOR     Vector90
#define KINETIS_RTCS_IRQ_VECTOR     Vector94

/* DAC attributes.*/
#define KINETIS_DAC0_IRQ_VECTOR     VectorA4

/** @} */

#endif /* _KINETIS_REGISTRY_H_ */

/** @} */
