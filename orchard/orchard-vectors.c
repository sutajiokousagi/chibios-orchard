#include "ch.h"
#include "hal.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/

void **HARDFAULT_PSP;
register void *stack_pointer asm("sp");

void HardFault_Handler(void) {
/*lint -restore*/
  // Hijack the process stack pointer to make backtrace work
  asm("mrs %0, psp" : "=r"(HARDFAULT_PSP) : :);
  stack_pointer = HARDFAULT_PSP;

  /* Break into the debugger */
  asm("bkpt #0");

  while(1);
}

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void MemManage_Handler(void) {
/*lint -restore*/

  while (true) {
  }
}

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void BusFault_Handler(void) {
/*lint -restore*/

  while (true) {
  }
}

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void UsageFault_Handler(void) {
/*lint -restore*/

  while (true) {
  }
}

uintptr_t __stack_chk_guard = 0x12345678;
__attribute__((noreturn))
void __stack_chk_fail(void) {
  chSysHalt("Stack check fail");
  while(1);
}
