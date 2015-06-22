#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "genes.h"

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

void cmd_gename(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  char genName[GENE_NAMELENGTH];

  generateName(genName);
  chprintf(chp, "%s\n\r", genName);
}

orchard_command("gename", cmd_gename);


static void init_genes(uint32_t block) {
  struct genes family;
  char genName[GENE_NAMELENGTH];

  family.signature = GENE_SIGNATURE;
  family.version = GENE_VERSION;
    
  // TODO: pick a name based upon a name lookup table so it's a more intuitive name
  generateName(genName);
  strncpy(family.name, genName, GENE_NAMELENGTH);

  storagePatchData(block, (uint32_t *) &family, GENE_OFFSET, sizeof(struct genes));
}

void geneStart() {
  const struct genes *family;

  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if( family->signature != GENE_SIGNATURE ) {
    init_genes(GENE_BLOCK);
  }
}

