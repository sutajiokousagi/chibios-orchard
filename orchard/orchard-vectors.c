#include <stdbool.h>

/**
 * @brief   Unhandled exceptions handler.
 * @details Any undefined exception vector points to this function by default.
 *          This function simply stops the system into an infinite loop.
 *
 * @notapi
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void HardFault_Handler(void) {
/*lint -restore*/

  /* Get back the existing stack frame, so backtrace will work */
  asm (
    /* Determine if we were using PSP or MSP before the exception */
    "    movs r0, #4          \n"
    "    movs r1, lr          \n"
    "    tst r0, r1           \n"
    "    beq reg_from_msp     \n"
    "\n"
    "    mrs r0, psp          \n"
    "    b restore_lr         \n"
    "reg_from_msp:            \n"
    "    mrs r0, msp          \n"
    "restore_lr:              \n"
    "    ldr r1, [r0, #20]    \n"
    "    mov lr, r1           \n"
    "    ldr r1, [r0, #24]    \n"  /* r1 contains previous sp */
    "break_into_debugger:     \n"
  );

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
