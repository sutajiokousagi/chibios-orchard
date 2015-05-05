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
 * @file    KINETIS/spi_lld.c
 * @brief   KINETIS SPI subsystem low level driver source.
 *
 * @addtogroup SPI
 * @{
 */

#include "hal.h"

#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if !defined(KINETIS_SPI_USE_SPI0)
#define KINETIS_SPI_USE_SPI0                TRUE
#endif

#if !defined(KINETIS_SPI_USE_SPI1)
#define KINETIS_SPI_USE_SPI1                TRUE
#endif

#if !defined(KINETIS_SPI0_RX_DMA_IRQ_PRIORITY)
#define KINETIS_SPI0_RX_DMA_IRQ_PRIORITY    8
#endif

#if !defined(KINETIS_SPI0_RX_DMAMUX_CHANNEL)
#define KINETIS_SPI0_RX_DMAMUX_CHANNEL      0
#endif

#if !defined(KINETIS_SPI0_RX_DMA_CHANNEL)
#define KINETIS_SPI0_RX_DMA_CHANNEL         0
#endif

#if !defined(KINETIS_SPI0_TX_DMAMUX_CHANNEL)
#define KINETIS_SPI0_TX_DMAMUX_CHANNEL      1
#endif

#if !defined(KINETIS_SPI0_TX_DMA_CHANNEL)
#define KINETIS_SPI0_TX_DMA_CHANNEL         1
#endif

#define DMAMUX_SPI_RX_SOURCE    16
#define DMAMUX_SPI_TX_SOURCE    17

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief SPI0 driver identifier.*/
#if KINETIS_SPI_USE_SPI0 || defined(__DOXYGEN__)
SPIDriver SPID1;
#endif

/** @brief SPI1 driver identifier.*/
#if KINETIS_SPI_USE_SPI1 || defined(__DOXYGEN__)
SPIDriver SPID2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void spi_fill_buffer(SPIDriver *spip)
{
    if ((spip->txbuf) && (spip->txoffset < spip->count))
      spip->spi->DL = spip->txbuf[spip->txoffset];
    else
      spip->spi->DL = 0xff;
    spip->txoffset++;
}

static void spi_start_xfer(SPIDriver *spip, bool polling)
{

  (void)polling;

  osalDbgAssert(spip->state == SPI_ACTIVE, "Invalid SPI state");

  /* Reset the SPI Match Flag if it's set.  We don't use this feature. */
  if (spip->spi->S & SPIx_S_SPMF)
    spip->spi->S |= SPIx_S_SPMF;

  spip->txoffset = 0;
  spip->rxoffset = 0;
  spip->spi->C1 |= SPIx_C1_SPIE | SPIx_C1_SPTIE;

  spi_fill_buffer(spip);
}

static void spi_stop_xfer(SPIDriver *spip)
{

  spip->spi->C1 &= ~(SPIx_C1_SPIE | SPIx_C1_SPTIE);
  spip->state = SPI_READY;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

static void spi_handle_isr(SPIDriver *spip)
{

  osalDbgAssert(spip->state == SPI_ACTIVE, "Invalid SPI state");

  while ((spip->rxoffset < spip->count) && (spip->spi->S & SPIx_S_SPRF)) {
    if ((spip->rxbuf) && (spip->rxoffset < spip->count))
      spip->rxbuf[spip->rxoffset] = spip->spi->DL;
    else
      (void)spip->spi->DL;
    spip->rxoffset++;
  }

  if (spip->rxoffset >= spip->count) {
    spi_stop_xfer(spip);
    _spi_isr_code(spip);
  }
  else if (spip->txoffset < spip->count) {
    spi_fill_buffer(spip);
  }
}

#if KINETIS_SPI_USE_SPI0
OSAL_IRQ_HANDLER(KINETIS_SPI0_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  spi_handle_isr(&SPID1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if KINETIS_SPI_USE_SPI1
OSAL_IRQ_HANDLER(KINETIS_SPI1_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  spi_handle_isr(&SPID2);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SPI driver initialization.
 *
 * @notapi
 */
void spi_lld_init(void) {
#if KINETIS_SPI_USE_SPI0
  spiObjectInit(&SPID1);
#endif
#if KINETIS_SPI_USE_SPI1
  spiObjectInit(&SPID2);
#endif
}

/**
 * @brief   Configures and activates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_start(SPIDriver *spip) {

  /* If in stopped state then enables the SPI and DMA clocks.*/
  if (spip->state == SPI_STOP) {

#if KINETIS_SPI_USE_SPI0
    if (&SPID1 == spip) {

      /* Enable the clock for SPI0 */
      SIM->SCGC4 |= SIM_SCGC4_SPI0;

      spip->spi = SPI0;

      nvicEnableVector(SPI0_IRQn, KINETIS_SPI_SPI0_IRQ_PRIORITY);
    }
#endif

#if KINETIS_SPI_USE_SPI1
    if (&SPID2 == spip) {

      /* Enable the clock for SPI1 */
      SIM->SCGC4 |= SIM_SCGC4_SPI1;

      spip->spi = SPI1;

      nvicEnableVector(SPI1_IRQn, KINETIS_SPI_SPI1_IRQ_PRIORITY);
    }
#endif
  }

  /* Initialize the SPI peripheral default values.*/
  spip->spi->C1 = 0;
  spip->spi->C2 = 0;
  spip->spi->BR = 0;

  /* Enable SPI system, and run as a Master.*/
  spip->spi->C1 |= (SPIx_C1_SPE | SPIx_C1_MSTR);

  /* Dummy read of "S" */
  (void)spip->spi->S;
}

/**
 * @brief   Deactivates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_stop(SPIDriver *spip) {

  /* If in ready state then disables the SPI clock.*/
  if (spip->state == SPI_READY) {

    spip->spi->C1 &= ~(SPIx_C1_SPE | SPIx_C1_MSTR);

#if KINETIS_SPI_USE_SPI0
    if (&SPID1 == spip) {
      nvicDisableVector(SPI0_IRQn);

      /* Disable the clock for SPI0 */
      SIM->SCGC4 &= ~SIM_SCGC4_SPI0;
    }
#endif

#if KINETIS_SPI_USE_SPI1
    if (&SPID2 == spip) {
      nvicDisableVector(SPI1_IRQn);

      /* Disable the clock for SPI0 */
      SIM->SCGC4 &= ~SIM_SCGC4_SPI1;
    }
#endif
  }
}

/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_select(SPIDriver *spip) {

  palClearPad(spip->config->ssport, spip->config->sspad);
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_unselect(SPIDriver *spip) {

  palSetPad(spip->config->ssport, spip->config->sspad);
}

/**
 * @brief   Ignores data on the SPI bus.
 * @details This asynchronous function starts the transmission of a series of
 *          idle words on the SPI bus and ignores the received data.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be ignored
 *
 * @notapi
 */
void spi_lld_ignore(SPIDriver *spip, size_t n) {

  spip->count = n;
  spip->rxbuf = NULL;
  spip->txbuf = NULL;

  spi_start_xfer(spip, false);
}

/**
 * @brief   Exchanges data on the SPI bus.
 * @details This asynchronous function starts a simultaneous transmit/receive
 *          operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be exchanged
 * @param[in] txbuf     the pointer to the transmit buffer
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_exchange(SPIDriver *spip, size_t n,
                      const void *txbuf, void *rxbuf) {

  spip->count = n;
  spip->rxbuf = rxbuf;
  spip->txbuf = txbuf;

  spi_start_xfer(spip, false);
}

/**
 * @brief   Sends data over the SPI bus.
 * @details This asynchronous function starts a transmit operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf) {

  spip->count = n;
  spip->rxbuf = NULL;
  spip->txbuf = (void *)txbuf;

  spi_start_xfer(spip, false);
}

/**
 * @brief   Receives data from the SPI bus.
 * @details This asynchronous function starts a receive operation.
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below or
 *          equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf) {

  spip->count = n;
  spip->rxbuf = rxbuf;
  spip->txbuf = NULL;

  spi_start_xfer(spip, false);
}

/**
 * @brief   Exchanges one frame using a polled wait.
 * @details This synchronous function exchanges one frame using a polled
 *          synchronization method. This function is useful when exchanging
 *          small amount of data on high speed channels, usually in this
 *          situation is much more efficient just wait for completion using
 *          polling than suspending the thread waiting for an interrupt.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] frame     the data frame to send over the SPI bus
 * @return              The received data frame from the SPI bus.
 */
uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame) {

  spi_start_xfer(spip, true);

  spi_stop_xfer(spip);

  return frame;
}

#endif /* HAL_USE_SPI */

/** @} */
