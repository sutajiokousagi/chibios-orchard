/*
 * Orchard Factory Test
 *
 * The factory test requires that the serial port and SWD pins be wired up.
 * Additionally, it is recommended that a USB power connection be made, so
 * as to check the charger.
 *
 * The Device Under Test (DUT) should be loaded into the test jig before the
 * test is run.  The board will be reflashed.  If OpenOCD is having trouble
 * attaching, it will erase flash.  Therefore, if you start the test prior
 * to loading the device into the jig, it will be completely erased.
 *
 * This test program will flash the DUT and will then run all tests.  If
 * you need to run individual tests, or flash the board separately, arguments
 * may be passed to this program.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

const char *openocd_default_args[] = {
  "openocd",
  "-c", "interface sysfsgpio",
  "-c", "transport select swd",
  "-c", "source [find target/klx.cfg]",
  "-c", "klx.cpu configure -rtos ChibiOS",
};

struct factory_flasher {
  int   openocd_sock; /* TCL socket */
  int   openocd_pid;  /* Process PID */
};

struct factory_config {
  const char *openocd_config;
  const char *serial_path;
  int         button_gpio;
  int         swclk_gpio;
  int         swdio_gpio;
};

static int openocd_run(struct factory_config *cfg) {
  char const *openocd_args[ARRAY_SIZE(openocd_default_args) + 7];
  char swdio_str[128];
  char swclk_str[128];
  unsigned int i;

  snprintf(swdio_str, sizeof(swdio_str) - 1, "sysfsgpio_swdio_num %d",
      cfg->swdio_gpio);
  snprintf(swclk_str, sizeof(swclk_str) - 1, "sysfsgpio_swclk_num %d",
      cfg->swclk_gpio);
  for (i = 0; i < ARRAY_SIZE(openocd_default_args); i++)
    openocd_args[i] = openocd_default_args[i];

  if (cfg->swdio_gpio >= 0) {
    openocd_args[i++] = "-c";
    openocd_args[i++] = swdio_str;
  }

  if (cfg->swclk_gpio >= 0) {
    openocd_args[i++] = "-c";
    openocd_args[i++] = swclk_str;
  }

  if (cfg->openocd_config) {
    openocd_args[i++] = "-f";
    openocd_args[i++] = cfg->openocd_config;
  }

  openocd_args[i++] = NULL;

  for (i = 0; openocd_args[i]; i++)
    printf("arg[%d]: %s\n", i, openocd_args[i]);

  return -1;
}

void print_help(const char *name) {
  printf("Usage:\n");
  printf("    %s\n", name);
  printf(" -c --config   Config file to use with OpenOCD\n");
  printf(" -s --serial   Serial port to use for monitoring\n");
  printf(" -k --swclk    GPIO pin to use for SWD clock\n");
  printf(" -d --swdio    GPIO pin to use for SWD data\n");
  printf(" -b --button   GPIO pin to use for Start button\n");
}

int parse_args(struct factory_config *cfg, int argc, char **argv) {

  int c;
  int idx = 0;
  static struct option long_options[] = {
    {"config", required_argument, NULL,  'c'},
    {"serial", required_argument, NULL,  's'},
    {"button", required_argument, NULL,  'b'},
    {"swclk",  required_argument, NULL,  'k'},
    {"swdio",  required_argument, NULL,  'd'},
    {"reset",  required_argument, NULL,  'r'},
    {"help",   no_argument,       NULL,  'h'},
  };

  while ((c = getopt_long(argc, argv, "c:k:d:b:h", long_options, &idx)) != -1) {
    switch (c) {
    case -1:
      return 0;

    case 'c':
      cfg->openocd_config = optarg;
      break;

    case 's':
      cfg->serial_path = optarg;
      break;

    case 'b':
      cfg->button_gpio = strtoul(optarg, NULL, 0);
      break;

    case 'k':
      cfg->swclk_gpio = strtoul(optarg, NULL, 0);
      break;

    case 'd':
      cfg->swdio_gpio = strtoul(optarg, NULL, 0);
      break;

    case 'h':
      print_help(argv[0]);
      return -1;

    default:
      printf("Unrecognized option: %c\n", c);
      print_help(argv[0]);
      return -1;
    }
  }
  return 0;
}

int main(int argc, char **argv) {

  struct factory_config cfg;

  memset(&cfg, 0, sizeof(cfg));
  cfg.swclk_gpio = -1;
  cfg.swdio_gpio = -1;

  if (getuid() != 0) {
    fprintf(stderr, "%s must be run as root\n", argv[0]);
    return 1;
  }

  if (parse_args(&cfg, argc, argv))
    return 1;

  printf("OpenOCD will use config file: %s\n", cfg.openocd_config);
  {
    openocd_run(&cfg);
  }

  return 0;
}
