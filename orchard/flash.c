#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-app.h"

#include "flash.h"

uint32_t gCallBackCnt; /* global counter in callback(). */
pFLASHCOMMANDSEQUENCE g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)0xFFFFFFFF;

/* array to copy __Launch_Command func to RAM */
uint16_t __ram_func[LAUNCH_CMD_SIZE/2];
uint16_t __ram_for_callback[CALLBACK_SIZE/2]; /* length of this array depends on total size of the functions need to be copied to RAM*/

static void callback(void);
extern uint32_t RelocateFunction(uint32_t dest, uint32_t size, uint32_t src);

// Freescale's Flash Standard Software Driver Structure
FLASH_SSD_CONFIG flashSSDConfig =
{
    FTFx_REG_BASE,          /* FTFx control register base */
    P_FLASH_BASE,           /* base address of PFlash block */
    P_FLASH_SIZE,           /* size of PFlash block */
    FLEXNVM_BASE,           /* base address of DFlash block */
    0,                      /* size of DFlash block */
    EERAM_BASE,             /* base address of EERAM block */
    0,                      /* size of EEE block */
    DEBUGENABLE,            /* background debug mode enable bit */
    NULL_CALLBACK           /* pointer to callback function */
};

// offset is in bytes
// sectorCount is in sectors
int8_t flashErase(uint32_t offset, uint16_t sectorCount) {
  uint32_t destination;          // sector number of target
  uint32_t end;
  uint16_t number;               /* Number of longword or phrase to be program or verify*/
  uint32_t margin_read_level;    /* 0=normal, 1=user - margin read for reading 1's */
  uint32_t ret;
  int8_t retval = F_ERR_OK;
  
  destination = offset;
  end = destination + (uint32_t) sectorCount;

  if( destination < F_USER_SECTOR_START ) {
    chprintf(stream, "User sectors start at %d, aborting.\n\r", F_USER_SECTOR_START);
    retval = F_ERR_RANGE;
    return retval;
  }
  if( end > ((flashSSDConfig.PFlashBase + flashSSDConfig.PFlashSize) / FTFx_PSECTOR_SIZE) ) {
    chprintf(stream, "Too many sectors requested, end at %d but we only have %d sectors.\n\r",
	     end, (flashSSDConfig.PFlashBase + flashSSDConfig.PFlashSize) / FTFx_PSECTOR_SIZE);
    retval = F_ERR_RANGE;
    return retval;
  }
  
  chprintf(stream, "Erasing sectors %d - %d\n\r", destination, end - 1);
  
  while (destination < end) {
    chSysLock();
    ret = FlashEraseSector(&flashSSDConfig, destination * FTFx_PSECTOR_SIZE,
			   FTFx_PSECTOR_SIZE, g_FlashLaunchCommand);
    chSysUnlock();
    if (FTFx_OK != ret) {
      retval = F_ERR_LOWLEVEL;
      return retval;
    }

    /* Verify section for several sector of PFLASH */
    number = FTFx_PSECTOR_SIZE/FSL_FEATURE_FLASH_PFLASH_SECTION_CMD_ADDRESS_ALIGMENT;
    // margin level = 0 normal, 1 user, 2 factory
    // factory is most stringent and only good on first P/E cycle
    // user level will detect if we're approaching normal margin, e.g. running out of margin
    for(margin_read_level = 0; margin_read_level < 0x2; margin_read_level++) {
      chSysLock();
      ret = FlashVerifySection(&flashSSDConfig, destination * FTFx_PSECTOR_SIZE, number, margin_read_level, g_FlashLaunchCommand);
      chSysUnlock();
      if (FTFx_OK != ret) {
	chprintf( stream, "Erase verify failed (%d), margin read level: %d\n\r", ret, margin_read_level );
	retval = F_ERR_LOWLEVEL;
      }
    }

    chprintf(stream, "  Erased sector %d...\n\r", destination );
    destination++;
  }

  return retval;
}

void flashStart(void) {
  uint32_t ret;
  
  gCallBackCnt = 0;
  
  ret = FlashInit(&flashSSDConfig);
  if (FTFx_OK != ret)  {
    chprintf(stream, "Flash init failed\n\r");
  }

  // Set callbacks -- copy from FLASH to RAM (in case of programming sectors where code is located)
  flashSSDConfig.CallBack = (PCALLBACK)RelocateFunction((uint32_t)__ram_for_callback , CALLBACK_SIZE , (uint32_t)callback);
  g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)RelocateFunction((uint32_t)__ram_func , LAUNCH_CMD_SIZE ,(uint32_t)FlashCommandSequence);
  
}

uint32_t flashGetSecurity(void) {
  uint8_t  securityStatus; 
  uint32_t ret;

  securityStatus = 0x0;
  ret = FlashGetSecurityState(&flashSSDConfig, &securityStatus);

  return ret;
}

// src, dst are pointers to physical memory locations, not sectors
// count is in bytes
int8_t flashProgram(uint8_t *src, uint8_t *dest, uint32_t count) {
  int8_t retval = F_ERR_OK;
  uint8_t noterased = 0;
  uint32_t i;
  uint32_t ret;
  uint32_t failaddr;
  
  // check if dest, dest+count is in the user-designated area of FLASH
  if( ((uint32_t) dest < (F_USER_SECTOR_START * FTFx_PSECTOR_SIZE)) ||
      (((uint32_t) dest + count) > (flashSSDConfig.PFlashBase + flashSSDConfig.PFlashSize)) ) {
    return F_ERR_RANGE;
  }

  // check if number of bytes to write & destination is word-aligned
  if( ((count % 4) != 0) || (( ((uint32_t) dest) % 4) != 0) ) {
    return F_ERR_NOTALIGN;
  }

  // check that the destination bytes have been erased
  // we can't re-program over 0's, it will overstress the Flash (per user manual spec)
  for( i = 0; i < count; i++ ) {
    if( dest[i] != 0xFF )
      noterased = 1;
  }
  if( noterased )
    return F_ERR_NOTBLANK;

  chSysLock();
  ret = FlashProgram(&flashSSDConfig, (uint32_t) dest, count, src, g_FlashLaunchCommand);
  chSysUnlock();
  
  if (FTFx_OK != ret)
    return F_ERR_LOWLEVEL;

  // user margin is more strict than normal margin -- data can be read still if
  // this fails, but indicates flash is wearing out
  chSysLock();
  ret = FlashProgramCheck(&flashSSDConfig, (uint32_t) dest, count, src, &failaddr,
			  READ_USER_MARGIN, g_FlashLaunchCommand);
  chSysUnlock();
  
  if (FTFx_OK != ret) {
    chprintf( stream, "Failed programming verification at USER margin levels: worry a little bit. Failure address: %08x\n\r", failaddr );
    return F_ERR_U_MARGIN;
  }

  return retval;
}

static void callback(void)
{
  // empty for now
}
