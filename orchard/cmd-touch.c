
#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "captouch.h"

#include <stdlib.h>

static int should_stop(void) {
  uint8_t bfr[1];
  return chnReadTimeout(serialDriver, bfr, sizeof(bfr), 1);
}

static void cmd_touch(BaseSequentialStream *buf, int argc, char **argv) {

  uint16_t val;

  (void)argc;
  (void)argv;

  chprintf(buf, "Captouch value: \n\r");
  while( !should_stop() ) {
    val = captouchRead();
    chprintf(buf, "0x%04x\r", val);
  }
  chprintf(buf, "\r\n");
  
}

orchard_command("touch", cmd_touch);

static void cmd_touchraw(BaseSequentialStream *buf, int argc, char **argv) {
  (void)argc;
  (void)argv;

  uint8_t ret;
  uint16_t val;
  uint8_t i, j;

  chprintf(buf, "Captouch raw values: \n\r");
  while( !should_stop() ) {
    for( i = 0x4, j = 0x1e; i < 0x1b; i+= 2, j++ ) {
      val = 0;
      ret = captouchGet(i);
      val = (uint16_t) ret;
      ret = captouchGet(i+1);
      val |= (ret << 8);
      chprintf(buf, "%04d ", val);
      val = ((uint16_t) captouchGet(j)) << 2;
      chprintf(buf, "%03d | ", val);
    }
    chprintf(buf, "\r" );
  }
  chprintf(buf, "\r\n");
}
orchard_command("tchraw", cmd_touchraw);

static void cmd_touchbaseline(BaseSequentialStream *buf, int argc, char **argv) {
  (void) argc;
  (void) argv;
  (void) buf;

  captouchFastBaseline();
}
orchard_command("tchbase", cmd_touchbaseline);

static void cmd_tdbg(BaseSequentialStream *buf, int argc, char **argv) {
  (void)buf;
  uint8_t adr;
  uint8_t dat;

  if(argc == 1 ) {
    adr = (uint8_t ) strtoul(argv[0], NULL, 16);
    captouchPrint(adr);
  } else if( argc == 2 ) {
    adr = (uint8_t ) strtoul(argv[0], NULL, 16);
    dat = (uint8_t ) strtoul(argv[1], NULL, 16);
    captouchSet(adr, dat);
  } else {
    captouchDebug();
  }
}
orchard_command("td", cmd_tdbg);


static void cmd_tcal(BaseSequentialStream *buf, int argc, char **argv) {
  (void)buf;
  (void)argc;
  (void)argv;

  captouchRecal();
}
orchard_command("tcal", cmd_tcal);
