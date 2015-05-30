/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#include "board.h"
#include "SSD_FTFx.h"
/* size of array to copy__Launch_Command function to.*/
/* It should be at least equal to actual size of __Launch_Command func */
/* User can change this value based on RAM size availability and actual size of __Launch_Command function */
#define LAUNCH_CMD_SIZE         0x100

/* Size of function used for callback.  Change this depending on the size of your function */
#define CALLBACK_SIZE           0x80

#define BUFFER_SIZE_BYTE        0x80

#define FTFx_REG_BASE           0x40020000
#define P_FLASH_BASE            0x00000000

/* Program Flash block information */
#define P_FLASH_SIZE            (FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE * FSL_FEATURE_FLASH_PFLASH_BLOCK_COUNT)
#define P_BLOCK_NUM             FSL_FEATURE_FLASH_PFLASH_BLOCK_COUNT
#define P_SECTOR_SIZE           FSL_FEATURE_FLASH_PFLASH_BLOCK_SECTOR_SIZE
/* Data Flash block information */
#define FLEXNVM_BASE            FSL_FEATURE_FLASH_FLEX_NVM_START_ADDRESS
#define FLEXNVM_SECTOR_SIZE     FSL_FEATURE_FLASH_FLEX_NVM_BLOCK_SECTOR_SIZE
#define FLEXNVM_BLOCK_SIZE      FSL_FEATURE_FLASH_FLEX_NVM_BLOCK_SIZE
#define FLEXNVM_BLOCK_NUM       FSL_FEATURE_FLASH_FLEX_NVM_BLOCK_COUNT

/* Flex Ram block information */
#define EERAM_BASE              FSL_FEATURE_FLASH_FLEX_RAM_START_ADDRESS
#define EERAM_SIZE              FSL_FEATURE_FLASH_FLEX_RAM_SIZE

/* Destination to program security key back to flash location */
#if (FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE == 8)
    #define SECURITY_LOCATION         0x408
#else /* FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE == 4 */
    #define SECURITY_LOCATION         0x40C
#endif

#define BACKDOOR_KEY_LOCATION     0x400

/* Program flash IFR map*/
#if (FSL_FEATURE_FLASH_IS_FTFE == 1)
    #define PFLASH_IFR                0x3C0
#else /* FSL_FEATURE_FLASH_IS_FTFL == 1 or FSL_FEATURE_FLASH_IS_FTFA = =1 */
    #define PFLASH_IFR                0xC0
#endif

#if (FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP == 1)
    #if (FSL_FEATURE_FLASH_IS_FTFE == 1)
        #define SWAP_STATUS_BIT (REG_READ(FTFx_REG_BASE + FTFx_SSD_FCNFG_OFFSET) & FTFE_FCNFG_SWAP_MASK)
    #endif
    #if (FSL_FEATURE_FLASH_IS_FTFL == 1)
        #define SWAP_STATUS_BIT (REG_READ(FTFx_REG_BASE + FTFx_SSD_FCNFG_OFFSET) & FTFL_FCNFG_SWAP_MASK)
    #endif
    #if (FSL_FEATURE_FLASH_IS_FTFA == 1)
        #define SWAP_STATUS_BIT (REG_READ(FTFx_REG_BASE + FTFx_SSD_FCNFG_OFFSET) & FTFA_FCNFG_SWAP_MASK)
    #endif
#endif

/* Has flash cache control in MCM module */
#if (FSL_FEATURE_FLASH_HAS_MCM_FLASH_CACHE_CONTROLS == 1)
    #define CACHE_DISABLE             MCM_BWR_PLACR_DFCS(MCM_BASE_PTR, 1);
/* Has flash cache control in FMC module */
#elif (FSL_FEATURE_FLASH_HAS_FMC_FLASH_CACHE_CONTROLS == 1)
    #if defined(FMC_PFB1CR) && defined(FMC_PFB1CR_B1SEBE_MASK)
        #define CACHE_DISABLE     FMC_PFB0CR &= ~(FMC_PFB0CR_B0SEBE_MASK | FMC_PFB0CR_B0IPE_MASK | FMC_PFB0CR_B0DPE_MASK | FMC_PFB0CR_B0ICE_MASK | FMC_PFB0CR_B0DCE_MASK);\
                                  FMC_PFB1CR &= ~(FMC_PFB1CR_B1SEBE_MASK | FMC_PFB1CR_B1IPE_MASK | FMC_PFB1CR_B1DPE_MASK | FMC_PFB1CR_B1ICE_MASK | FMC_PFB1CR_B1DCE_MASK);
    #elif defined(FMC_PFB23CR)
        #define CACHE_DISABLE     FMC_PFB01CR &= ~(FMC_PFB01CR_B0IPE_MASK | FMC_PFB01CR_B0DPE_MASK | FMC_PFB01CR_B0ICE_MASK | FMC_PFB01CR_B0DCE_MASK);\
                                  FMC_PFB23CR &= ~(FMC_PFB23CR_B1IPE_MASK | FMC_PFB23CR_B1DPE_MASK | FMC_PFB23CR_B1ICE_MASK | FMC_PFB23CR_B1DCE_MASK);
    #else
        #define CACHE_DISABLE     FMC_PFB0CR &= ~(FMC_PFB0CR_B0SEBE_MASK | FMC_PFB0CR_B0IPE_MASK | FMC_PFB0CR_B0DPE_MASK | FMC_PFB0CR_B0ICE_MASK | FMC_PFB0CR_B0DCE_MASK);
    #endif
#else
/* No cache in the device */
    #define CACHE_DISABLE
#endif

/***************************************************************************************/
/***************************************************************************************/

/* Other defines */
#define DEBUGENABLE               0x00

#define READ_NORMAL_MARGIN        0x00
#define READ_USER_MARGIN          0x01
#define READ_FACTORY_MARGIN       0x02

#define ONE_KB                  1024                        //0x400:  10 zeros
#define TWO_KB                  (2*ONE_KB)
#define THREE_KB                (3*ONE_KB)
#define FOUR_KB                 (4*ONE_KB)
#define FIVE_KB                 (5*ONE_KB)
#define SIX_KB                  (6*ONE_KB)
#define SEVEN_KB                (7*ONE_KB)
#define EIGHT_KB                (8*ONE_KB)
#define NINE_KB                 (9*ONE_KB)
#define TEN_KB                  (10*ONE_KB)
#define ONE_MB                  (ONE_KB*ONE_KB)             //0x100000:     20 zeros
#define ONE_GB                  (ONE_KB*ONE_KB*ONE_KB)      //0x40000000:   30 zeros

#if defined(SWAP_M)
/****************************************************************************/
/* Use an address towards the end of P-Flash block for the swap indicator.  */
/* The swap indicator is managed by the swap system, but is a location in */
/* PFlash and it gets modified (erased & programmed) throughout the swap */
/* process, so it cannot share space with normal application code */
/* Here, we are using the second-to-last sector, since the Lower/Upper */
/* data blocks use the last sector are used to hold our test data to help */
/* identify each block after the swap */
/****************************************************************************/
    #define PSWAP_INDICATOR_ADDR      (P_FLASH_SIZE/2 - 2*(FTFx_PSECTOR_SIZE))
/* The Lower & Upper Data sectors are used to program test data into, to */
/* help identify each block - for debug purposes.  */
    #define PSWAP_LOWERDATA_ADDR      (PSWAP_INDICATOR_ADDR + FTFx_PSECTOR_SIZE + 0x100)
    #define PSWAP_UPPERDATA_ADDR      (PSWAP_INDICATOR_ADDR + FTFx_PSECTOR_SIZE + P_FLASH_SIZE/2 + 0x100)
/************************************************************************************************/
#endif /* End of #if defined(SWAP_M) */

/************************************************************/
/* prototypes                                               */
/************************************************************/
void flashStart(void);
uint32_t flashGetSecurity(void);
int8_t flashErase(uint32_t offset, uint16_t sectorCount);
int8_t flashProgram(uint8_t *src, uint8_t *dst, uint32_t count);

// define limit for user-accessible sector
// we assume we have flash from the user sector all the way to the end of Flash
// __storage_start__ comes from the linker
#define F_USER_SECTOR_START ((uint32_t)__storage_start__ / (uint32_t)FTFx_PSECTOR_SIZE) 

// chibios error codes -- they make more sense to me than Freescale's set of error codes
#define F_ERR_OK  0
#define F_ERR_NOTBLANK  -1   // attempting to program a programmed section
#define F_ERR_NOTALIGN  -2   // attempting to program a block of data that's not 4-byte aligned
#define F_ERR_LOWLEVEL  -3   // low level error (e.g. margin fail, etc.)
#define F_ERR_RANGE     -4   // attempting to manipulate a block outside the user range
#define F_ERR_U_MARGIN  -6   // failed verification at user margin levels -- worry a little
#define F_ERR_N_MARGIN  -7   // failed verification at normal margin levels -- worry a lot
#define F_ERR_F_MARGIN  -8   // failed verification at factory margin levels -- don't worry too much
#define F_ERR_GENERAL   -9   // general error code
#define F_ERR_JOURNAL_OVER   -10   // journalingy overflow -- we don't handle it
#endif /* _FLASH_H_ */
