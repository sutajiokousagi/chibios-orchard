#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"

#include "flash.h"
#include "storage.h"

#include <string.h>
#include <stdlib.h>

// returns a pointer to the data section of the new sector
const uint32_t *init_sector(uint32_t sector, uint32_t block, uint32_t journalrev) {
  orfs_head header;
  orfs_head *rethead;
  int8_t ret;

  chprintf(stream, " init_sector: sector %d, block %d, journal %x\n\r", sector, block, journalrev);
  // initialize a sector to a blank state
  flashErase(sector, 1);

  header.signature = ORFS_SIG;
  header.version = ORFS_REV;
  header.block = block;
  header.journalrev = journalrev;
  header.firstData = 0xFFFFFFFF;  // don't set it
  
  ret = flashProgram((uint8_t *) &header, (uint8_t *) (sector * SECTOR_SIZE), sizeof(orfs_head));
  if( ret != F_ERR_OK ) {
    osalDbgAssert(FALSE, "Sector init failed on programming error\n\r");
    return NULL;
  }

  rethead = (orfs_head *) (sector * SECTOR_SIZE);
  return &(rethead->firstData);
}

// there should always be at least one empty sector
static uint32_t find_empty_sector(void) {
  uint32_t blockToSector[BLOCK_TOTAL];  // stores the sector that a block is located at
  uint32_t journalRevs[BLOCK_TOTAL];
  uint32_t i;
  orfs_head *header;

  memset( blockToSector, 0xFF, sizeof(blockToSector) );
  memset( journalRevs, 0xFF, sizeof(journalRevs) );
  
  // do a search through sector space for the first duplicated sector
  // we want the first duplicated sector number with the lower journal revision
  for( i = 0; i < SECTOR_COUNT; i++ ) {
    header = (orfs_head *) ((SECTOR_MIN + i) * SECTOR_SIZE);
    // first try a search for blank memory, if it's blank just return
    if( header->signature != ORFS_SIG ) {
      return i + SECTOR_MIN;
    } else {
      if( blockToSector[header->block] == 0xFFFFFFFF ) {
	blockToSector[header->block] = i; // record the last sector we saw
	journalRevs[header->block] = header->journalrev;
      } else {
	// this block has already been mapped to a sector.
	// either this is the newer version, or the older version. Find out.
	if( header->journalrev < journalRevs[header->block] ) {
	  // in this case, the current header is /newer/ than the one we saw
	  // (journal numbers go down from 0xFFFFFFFE)
	  // so return the old sector number
	  return blockToSector[header->block] + SECTOR_MIN;
	} else {
	  return i + SECTOR_MIN;
	}
      }
    }
  }

  // we should never get here...
  osalDbgAssert(FALSE, "Couldn't find an empty sector, ORFS corrupt?\n\r");
  return SECTOR_INVALID;
}

const void *storageGetData(uint32_t block) {
  uint32_t *foundData = NULL;
  uint32_t journalrev = JOURNAL_YOUNGEST;
  uint32_t i;
  orfs_head *header;
  // do a linear search for the sector holding our block
  for( i = 0; i < SECTOR_COUNT; i++ ) {
    header = (orfs_head *) ((SECTOR_MIN + i) * SECTOR_SIZE);
    if( header->signature == ORFS_SIG ) {
      if( (header->block == block) && (header->journalrev <= journalrev ) ) {
	foundData = &(header->firstData);
	journalrev = header->journalrev;
      }
    }
  }
  if( foundData != NULL ) {
    return foundData;
  } else {
    // we're dealing with virgin memory, just create a block out of thin air
    return init_sector(find_empty_sector(), block, JOURNAL_YOUNGEST);
  }
}

// return the journal entry number for a block
// if the block doesn't exist, create it
static uint32_t storage_get_journal(uint32_t block) {
  uint32_t *foundData = NULL;
  uint32_t journalrev = JOURNAL_YOUNGEST;
  uint32_t i;
  orfs_head *header;
  // do a linear search for the sector holding our block
  for( i = 0; i < SECTOR_COUNT; i++ ) {
    header = (orfs_head *) ((SECTOR_MIN + i) * SECTOR_SIZE);
    if( header->signature == ORFS_SIG ) {
      if( (header->block == block) && (header->journalrev <= journalrev ) ) {
	foundData = &(header->firstData);
	journalrev = header->journalrev;
      }
    }
  }
  if( foundData != NULL ) {
    return journalrev;
  } else {
    init_sector(find_empty_sector(), block, JOURNAL_YOUNGEST);    
    return JOURNAL_YOUNGEST;
  }
}

// return code based on F_ERR system
// you can patch a blank block, it will just allocate and initialize it
// offset and size are in bytes, and must be word-aligned
int8_t storagePatchData(uint32_t block, uint32_t *data, uint32_t offset, uint32_t size) {
  const uint32_t *destData;
  uint32_t *srcData;
  uint32_t i;
  uint8_t isblank = 1;
  uint8_t ret = F_ERR_OK;
  uint32_t destSector;
  uint32_t journalrev;
  
  if( (offset + size) > SECTOR_SIZE ) {
    // we're out of bounds, should we as a policy fail, or just truncate?
    osalDbgAssert(FALSE, "offset + size out of bounds\n\r");
    return F_ERR_RANGE;
  }
  if( block >= BLOCK_TOTAL ) {
    osalDbgAssert(FALSE, "block number is out of range\n\r");
    return F_ERR_RANGE;
  }

  osalDbgAssert( (offset % 4) == 0, "Offset isn't word-aligned.\n\r" );
  osalDbgAssert( (size % 4) == 0, "Size isn't word-aligned.\n\r" );
  
  // first, retrieve the block in question to be patched
  // if the block hasn't been allocated the following function will automatically do that too
  destData = storageGetData(block);
  osalDbgAssert(destData != NULL, "Couldn't find/allocate storage block to patch\n\r");

  for( i = 0; i < size / sizeof(uint32_t); i++ ) {
    if( destData[i + (offset / sizeof(uint32_t))] != 0xFFFFFFFF )
      isblank = 0;
  }
  if(isblank) {
    // we can patch in the new data
    ret = flashProgram((uint8_t *) data, (uint8_t *) ((uint32_t) destData + offset), size);
    osalDbgAssert(ret == F_ERR_OK, "Low level programming error in storagePatchData\n\r");
    return ret;
  } else {
    // if it's not blank:
    // allocate a new sector
    // decrement the journal number
    // program in the patched data
    journalrev = storage_get_journal(block);
    destSector = find_empty_sector();
    
    osalDbgAssert(destSector != SECTOR_INVALID, "ORFS general error, couldn't find the empty sector (there should always be exactly one)\n\r");

    if( journalrev == 0 ) {
      chprintf( stream, "Journaling overflow, we somehow went through 4 billion revisions...\n\r" );
      return F_ERR_JOURNAL_OVER;
      // TODO: a graceful way to handle this would be to simply reset journal to
      // youngest value, but make sure the older block is low-level erased so it doesn't
      // show up in the journaling sweep...
    }
    
    srcData = (uint32_t *) destData; // we now swap the meaning of source and destination:
    // we have to copy the old destination to the new destination with the patches
    destData = init_sector(destSector, block, journalrev - 1);

    // copy over the data up to the offset
    ret = flashProgram((uint8_t *) srcData, (uint8_t *) destData, offset);
    osalDbgAssert(ret == F_ERR_OK, "Low level programming error in storagePatchData\n\r");

    // patch in the new data
    ret = flashProgram((uint8_t *) data, (uint8_t *) ((uint32_t) destData + offset), size);
    osalDbgAssert(ret == F_ERR_OK, "Low level programming error in storagePatchData\n\r");

    // copy over data from the end of the patch block
    ret = flashProgram((uint8_t *) ((uint32_t) srcData + offset + size),
		       (uint8_t *) ((uint32_t) destData + offset + size),
		       SECTOR_SIZE - (offset + size) - (sizeof(orfs_head) - 4));
    osalDbgAssert(ret == F_ERR_OK, "Low level programming error in storagePatchData\n\r");
  }
    
  return F_ERR_OK;
}
