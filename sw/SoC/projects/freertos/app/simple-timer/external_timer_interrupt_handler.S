#include "portContext.h"

  .extern xTaskIncrementTick
  .extern vTaskSwitchContext
  .extern xISRStackTop
  .extern xlnx_tim_clear_int
  .extern timer
  .global vExternalTimerTickHandler
vExternalTimerTickHandler:
    portcontextSAVE_INTERRUPT_CONTEXT
    la a0, timer
    call xlnx_tim_clear_int
    call xTaskIncrementTick
    beqz a0, exit_without_context_switch    /* Don't switch context if incrementing tick didn't unblock a task. */
    call vTaskSwitchContext
exit_without_context_switch:
    portcontextRESTORE_CONTEXT
