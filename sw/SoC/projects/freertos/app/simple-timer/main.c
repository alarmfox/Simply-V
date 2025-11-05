#include "FreeRTOS.h"
#include "task.h"
#include "uninasoc.h"

static xlnx_tim_t timer = {
  .base_addr = TIM0_BASEADDR,
  .counter = 200000,
  .reload_mode = TIM_RELOAD_AUTO,
  .count_direction = TIM_COUNT_DOWN
};

#define TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define TASK_PARAMETER (1)

static void Task(void *pvParameters) {

  configASSERT(((uint32_t)pvParameters) == TASK_PARAMETER);

  TickType_t xLastWakeTime;
  uint32_t amt = (uint32_t) pvParameters;
  uint32_t counter = 0;
  const TickType_t xFrequency = 10;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  while (1) {
    counter += amt;
    printf("Task counter: %d\n\r", counter);
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

// https://rcc.freertos.org/Documentation/02-Kernel/05-RTOS-implementation-tutorial/02-Building-blocks/03-The-RTOS-tick
void vExternalTickIncrement() {
  BaseType_t xSwitchRequired;

  // Clear interrupt flag
  xlnx_tim_clear_int(&timer);

  // Increment RTOS tick count
  xSwitchRequired = xTaskIncrementTick();

  // If a task was unblocked, yield to it
  if (xSwitchRequired != pdFALSE) {
    portYIELD_FROM_ISR(xSwitchRequired);
  }
}

void freertos_risc_v_application_interrupt_handler(uint32_t mcause) {
  (void) mcause;

  uint32_t interrupt_id = plic_claim();

  switch (interrupt_id) {
    case 0x2:
      vExternalTickIncrement();
      break;
    default:
      break;
  }
  plic_complete(interrupt_id);
}

void vAssertCalled(const char *file, int line) {
  (void)file;
  (void)line;

  __asm("ebreak");
}

#ifdef configCHECK_FOR_STACK_OVERFLOW
/* Called if a task overflows its stack space */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;

  __asm volatile ("ebreak");
}
#endif // configCHECK_FOR_STACK_OVERFLOW


#ifdef configUSE_MALLOC_FAILED_HOOK
/* Called if pvPortMalloc() fails (heap exhausted) */
void vApplicationMallocFailedHook(void) {
    __asm volatile ("ebreak");
}
#endif // configUSE_MALLOC_FAILED_HOOK

// define if you need to set the timer interrupt,otherwise an empty definition
// is still necessary to overwrite the weak definition in port.c and to avoid
// unwanted jumps to reset handler
void vPortSetupTimerInterrupt(void) {
  int ret;
  uint32_t priorities[3] = { 1, 1, 1 };
  
  plic_init();
  plic_configure(priorities, 3);
  plic_enable_all();

  xlnx_tim_init(&timer);
 
  ret = xlnx_tim_configure(&timer);
  if (ret != UNINASOC_OK) {
    printf("Cannot configure timer\r\n");
    return;
  }

  ret = xlnx_tim_enable_int(&timer);
  if (ret != UNINASOC_OK) {
    printf("Cannot enable timer\r\n");
    return;
  }

  ret = xlnx_tim_start(&timer);
  if (ret != UNINASOC_OK) {
    printf("Cannot start timer\r\n");
    return;
  }

  // Enable local interrupts lines
  // MEI (External Interrupt)
  __asm volatile ( "csrs mie, %0" ::"r" ( 0x800 ) );
}

int main() {

  uninasoc_init();

  // Create FreeRTOS Task
  BaseType_t res = xTaskCreate(Task, "task", configMINIMAL_STACK_SIZE,
                                 (void *)TASK_PARAMETER, TASK_PRIORITY, NULL);

  configASSERT(res== pdPASS);

  size_t free_heap = xPortGetFreeHeapSize();

  configASSERT(free_heap > 0);
  vTaskStartScheduler();

  configASSERT(0); // insufficient RAM->scheduler task returns->vAssertCalled() called

  while (1);

  return 0;
}
