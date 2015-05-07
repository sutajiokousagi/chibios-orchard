#ifndef __ORCHARD_SHELL_H__
#define __ORCHARD_SHELL_H__

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "orchard.h"

void orchardShellInit(void);
void orchardShellRestart(void);

#define orchard_command_start() \
({ \
  static char start[0] __attribute__((unused,  \
    aligned(4), section(".chibi_list_cmd_1")));        \
  (const ShellCommand *)&start;            \
})

#define orchard_command(_name, _func) \
  const ShellCommand _orchard_cmd_list_##_func \
  __attribute__((unused, aligned(4), section(".chibi_list_cmd_2_" _name))) = \
     { _name, _func }

#define orchard_command_end() \
  const ShellCommand _orchard_cmd_list_##_func \
  __attribute__((unused, aligned(4), section(".chibi_list_cmd_3_end"))) = \
     { NULL, NULL }

#endif /* __ORCHARD_SHELL_H__ */
