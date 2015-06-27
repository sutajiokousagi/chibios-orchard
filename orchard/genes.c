#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "genes.h"
#include "orchard-math.h"

#include "orchard-shell.h"

#include <string.h>
#include <stdlib.h>

static const char *first_names[16] =
  {"Happy",
   "Dusty",
   "Sassy",
   "Sexy",
   
   "Silly",
   "Curvy",
   "Nerdy",
   "Geeky",
   
   "OMG",
   "Fappy",
   "Trippy",
   "Lovely",

   "Furry",
   "WTF",
   "Spacy",
   "Lacy",
  };

static const char *middle_names[16] =
  {"Playa",
   "OMG",
   "Hot",
   "Dope",
   
   "Pink",
   "Balla",
   "Sweet",
   "Cool",
   
   "Cute",
   "Nice",
   "Fun",
   "Soft",

   "Short",
   "Tall",
   "Huge",
   "Red",
  };

static const char *last_names[8] =
  {"Virus",
   "Brain",
   "Raver",
   "Hippie",
   
   "Profit",
   "Relaxo",
   "Phage",
   "Blinky",
  };

void generateName(char *result) {
  uint32_t r = rand();
  uint8_t i = 0;

  i = strlen(strcpy(result, first_names[r & 0xF]));
  strcpy(&(result[i]), middle_names[(r >> 8) & 0xF]);
  i = strlen(result);
  strcpy(&(result[i]), last_names[(r >> 16) & 0x7]);
  i = strlen(result);

  osalDbgAssert( i < GENE_NAMELENGTH, "Name generated exceeds max length, revisit name database!\n\r" );
}


// for testing, mostly
void cmd_gename(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  char genName[GENE_NAMELENGTH];

  generateName(genName);
  chprintf(chp, "%s\n\r", genName);
}
orchard_command("gename", cmd_gename);

static void generate_gene(struct genome *individual) {
  char genName[GENE_NAMELENGTH];

  individual->cd_period = map((int16_t) rand() & 0xFF, 0, 255, 0, 6);
  individual->cd_rate = (uint8_t) rand() & 0xFF;
  individual->cd_dir = (uint8_t) rand() & 0xFF;
  individual->sat = (uint8_t) rand() & 0xFF;
  individual->hue = (uint8_t) rand() & 0xFF;
  individual->hue_ratedir = (uint8_t) rand() & 0xFF;
  individual->hue_bound = (uint8_t) rand() & 0xFF;
  individual->lin = (uint8_t) rand() & 0xFF;
  individual->strobe = (uint8_t) rand() & 0xFF;
  individual->accel = (uint8_t) rand() & 0xFF;
  individual->mic = (uint8_t) rand() & 0xFF;
  
  generateName(genName);
  strncpy(individual->name, genName, GENE_NAMELENGTH);
}

#if 0
void cmd_testmap(BaseSequentialStream *chp, int argc, char *argv[]) {
  int16_t i;

  for( i = 0; i < 16; i++ ) {
    chprintf( chp, "%d : %d\n\r", i, map(i, 0, 15, 0, 255) );
  }
  for( i = 0; i < 16; i++ ) {
    chprintf( chp, "%d : %d\n\r", i, map(i, 0, 15, 0, 5) );
  }
}
orchard_command("testmap", cmd_testmap);
#endif

void cmd_geneseq(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint8_t which;
  const struct genes *family;

  family = (const struct genes *) storageGetData(GENE_BLOCK);
  
  if( argc != 1 ) {
    chprintf(chp, "Usage: geneseq <individual>, where <individual> is 0-%d\n\r", GENE_FAMILYSIZE - 1);
    return;
  }
  which = (uint8_t) strtoul( argv[0], NULL, 0 );
  if( which >= GENE_FAMILYSIZE ) {
    chprintf(chp, "Usage: geneseq <individual>, where <individual> is 0-%d\n\r", GENE_FAMILYSIZE - 1);
    return;
  }

  if( family->signature != GENE_SIGNATURE ) {
    chprintf(chp, "Invalid genome signature\n\r" );
    return;
  }
  if( family->version != GENE_VERSION ) {
    chprintf(chp, "Invalid genome version\n\r" );
    return;
  }
  chprintf(chp, "Individual %s:\n\r", family->individual[which].name );
  chprintf(chp, " %3d cd_period\n\r", family->individual[which].cd_period );
  chprintf(chp, " %3d cd_rate\n\r", family->individual[which].cd_rate );
  chprintf(chp, " %3d cd_dir\n\r", family->individual[which].cd_dir );
  chprintf(chp, " %3d sat\n\r", family->individual[which].sat );
  chprintf(chp, " %3d hue\n\r", family->individual[which].hue );
  chprintf(chp, " %3d hue_ratedir\n\r", family->individual[which].hue_ratedir );
  chprintf(chp, " %3d hue_bound\n\r", family->individual[which].hue_bound );
  chprintf(chp, " %3d lin\n\r", family->individual[which].lin );
  chprintf(chp, " %3d strobe\n\r", family->individual[which].strobe );
  chprintf(chp, " %3d accel\n\r", family->individual[which].accel );
  chprintf(chp, " %3d mic\n\r", family->individual[which].mic );
}
orchard_command("geneseq", cmd_geneseq);

static void init_genes(uint32_t block) {
  struct genes family;
  char genName[GENE_NAMELENGTH];
  int i;

  family.signature = GENE_SIGNATURE;
  family.version = GENE_VERSION;
    
  generateName(genName);
  strncpy(family.name, genName, GENE_NAMELENGTH);

  for( i = 0; i < GENE_FAMILYSIZE; i++ ) {
    generate_gene(&family.individual[i]);
  }

  storagePatchData(block, (uint32_t *) &family, GENE_OFFSET, sizeof(struct genes));
}

void geneStart() {
  const struct genes *family;

  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if( family->signature != GENE_SIGNATURE ) {
    init_genes(GENE_BLOCK);
  } else if( family->version != GENE_VERSION ) {
    init_genes(GENE_BLOCK);
  }
}

