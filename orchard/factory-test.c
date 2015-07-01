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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

const char *openocd_default_args[] = {
  "openocd",
  "-c", "interface sysfsgpio",
  "-c", "transport select swd",
  "-c", "source [find target/klx.cfg]",
  "-c", "klx.cpu configure -rtos ChibiOS",
};

struct factory_config {
  const char *openocd_config;
  const char *serial_path;
  int         button_gpio;
  int         swclk_gpio;
  int         swdio_gpio;
};

struct factory_flasher {
  int                   openocd_sock; /* TCL socket */
  int                   openocd_pid;  /* Process PID */
  int                   serial_fd;    /* TTL UART file */
  struct factory_config cfg;
};

struct factory_flasher *g_factory;

static void reap_children(int signal) {
  int status;
  pid_t pid;

  fprintf(stderr, "Got SIGCHLD\n");

  while (-1 != (pid = waitpid(-1, &status, WNOHANG))) {
    fprintf(stderr, "Child %d returned %d\n", pid, status);
    g_factory->openocd_pid = -1;
  }
  fprintf(stderr, "Done reaping\n");
}

static void cleanup(void) {
  if (g_factory->openocd_pid != -1)
    kill(g_factory->openocd_pid, SIGTERM);

  if (g_factory->openocd_sock != -1) {
    (void) shutdown(g_factory->openocd_sock, SHUT_RDWR);
    close(g_factory->openocd_sock);
  }
  g_factory->openocd_sock = -1;
}

static int openocd_run(struct factory_flasher *factory) {
  const char *openocd_args[ARRAY_SIZE(openocd_default_args) + 7];
  char swdio_str[128];
  char swclk_str[128];
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE(openocd_default_args); i++)
    openocd_args[i] = openocd_default_args[i];

  if (factory->cfg.swdio_gpio >= 0) {
    snprintf(swdio_str, sizeof(swdio_str) - 1, "sysfsgpio_swdio_num %d",
        factory->cfg.swdio_gpio);
    openocd_args[i++] = "-c";
    openocd_args[i++] = swdio_str;
  }

  if (factory->cfg.swclk_gpio >= 0) {
    snprintf(swclk_str, sizeof(swclk_str) - 1, "sysfsgpio_swclk_num %d",
        factory->cfg.swclk_gpio);
    openocd_args[i++] = "-c";
    openocd_args[i++] = swclk_str;
  }

  if (factory->cfg.openocd_config) {
    openocd_args[i++] = "-f";
    openocd_args[i++] = factory->cfg.openocd_config;
  }

  openocd_args[i++] = NULL;

  factory->openocd_pid = fork();
  if (factory->openocd_pid == -1) {
    perror("Unable to fork");
    return -1;
  }

  if (factory->openocd_pid == 0) {
    execvp(openocd_args[0], (char * const*) openocd_args);
    perror("Unable to exec");
    return -1;
  }

  return factory->openocd_pid;
}

static int openocd_connect(struct factory_flasher *factory) {

  const char *address = "127.0.0.1";
  const int port = 6666;
  struct sockaddr_in sockaddr;
  int ret;

  factory->openocd_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  if (-1 == factory->openocd_sock) {
    perror("cannot create socket");
    return -1;
  }
  
  memset(&sockaddr, 0, sizeof(sockaddr));
  
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  ret = inet_pton(AF_INET, address, &sockaddr.sin_addr);
  
  if (ret < 0) {
    perror("Seems like INET support doesn't exist");
    goto err;
  }
  else if (ret == 0) {
    perror("Unable to connect to address");
    goto err;
  }

  if (-1 == connect(factory->openocd_sock,
                    (struct sockaddr *)&sockaddr,
                    sizeof(sockaddr))) {
    perror("connect failed");
    goto err;
  }

  return 0;
  
err:
  close(factory->openocd_sock);
  factory->openocd_sock = -1;
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

static int serial_open(struct factory_flasher *factory) {
  struct termios t;
  int ret;

  if (!factory->cfg.serial_path)
    return 0;

  factory->serial_fd = open(factory->cfg.serial_path, O_RDWR);
  if (-1 == factory->serial_fd) {
    perror("Unable to open serial port");
    return -1;
  }

  ret = tcgetattr(factory->serial_fd, &t);
  if (-1 == ret) {
    perror("Failed to get attributes");
    goto err;
  }

  cfsetispeed(&t, B115200);
  cfsetospeed(&t, B115200);
  cfmakeraw(&t);

  ret = tcsetattr(factory->serial_fd, TCSANOW, &t);
  if (-1 == ret) {
    perror("Failed to set attributes");
    goto err;
  }

err:
  close(factory->serial_fd);
  factory->serial_fd = -1;
  return -1;
}

int main(int argc, char **argv) {

  struct factory_flasher factory;
  g_factory = &factory;

  memset(&factory, 0, sizeof(factory));
  factory.openocd_pid = -1;
  factory.openocd_sock = -1;
  factory.serial_fd = -1;
  factory.cfg.swclk_gpio = -1;
  factory.cfg.swdio_gpio = -1;

  atexit(cleanup);
  signal(SIGCHLD, reap_children);

  if (getuid() != 0) {
    fprintf(stderr, "%s must be run as root\n", argv[0]);
    return 1;
  }

  if (parse_args(&factory.cfg, argc, argv))
    return 1;

  if (serial_open(&factory))
    return -1;

  {
    openocd_run(&factory);

    int tries = 0;
    /* Keep trying to connect as long as OpenOCD is still running */
    while ((factory.openocd_pid != -1) && (factory.openocd_sock == -1)) {
      usleep(50000);
      openocd_connect(&factory);
      tries++;
    }

    printf("Connected to OpenOCD with fd %d after %d tries\n",
        factory.openocd_sock, tries);
  }

  return 0;
}
