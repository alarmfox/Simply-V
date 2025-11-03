#include "FreeRTOS.h"
#include "task.h"

extern void vExternalTimerTickHandler(void);

#ifdef USE_UNINASOC

#include "uninasoc.h"
xlnx_tim_t timer = {
  .base_addr = TIM0_BASEADDR,
  .counter = 20000000,
  .reload_mode = TIM_RELOAD_AUTO,
  .count_direction = TIM_COUNT_DOWN
};

#endif // USE_UNINASOC

#define TASK1_PRIORITY (tskIDLE_PRIORITY + 1)

#define TASK1_PARAMETER (1)

static void Task(void *pvParameters) {

  uint32_t a;
  uint32_t b = 3;

  configASSERT(((uint32_t)pvParameters) == TASK1_PARAMETER);
  size_t free_heap;

  while (1) {
    free_heap = xPortGetFreeHeapSize();
    configASSERT(free_heap > 0);

    a = b + (uint32_t)pvParameters;
    (void)a;

#ifdef USE_UNINASOC
    printf("Hello from Task\n\r");
#endif // USE_UNINASOC
  }
}

void freertos_risc_v_application_interrupt_handler(void) {

    uint32_t interrupt_id = plic_claim();
    switch (interrupt_id) {
    case 0x2:
        printf("Handiling TIM0 interrupt!\r\n");
        vExternalTimerTickHandler();
      break;
    default:
      break;
    }
    plic_complete(interrupt_id);
}

void vAssertCalled(const char *file, int line) {

  const char *f = file;
  int l = line;
  (void)f;
  (void)l;
  __asm("ebreak");
}

// define if you need to set the timer interrupt,otherwise an empty definition
// is still necessary to overwrite the weak definition in port.c and to avoid
// unwanted jumps to reset handler
#ifdef USE_UNINASOC
void vPortSetupTimerInterrupt(void) {
  int ret;
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
#else
// Define a function to configure the timer
void vPortSetupTimerInterrupt(void) {

}
#endif // USE_UNINASOC

int main() {

#ifdef USE_UNINASOC
  int ret;
  uint32_t priorities[3] = { 1, 1, 1 };

  uninasoc_init();

  plic_init();
  plic_configure(priorities, 3);
  plic_enable_all();
 
  xlnx_tim_init(&timer);
 
  ret = xlnx_tim_configure(&timer);
  if (ret != UNINASOC_OK) {
    printf("Cannot configure timer\r\n");
    goto end;
  }

  printf("SIMPLY-V FreeRTOS DEMO!\n\r");
#endif // USE_UNINASOC

  // Create FreeRTOS Task
  BaseType_t mem_1 = xTaskCreate(Task, "t1", configMINIMAL_STACK_SIZE,
                                 (void *)TASK1_PARAMETER, TASK1_PRIORITY, NULL);

  configASSERT(mem_1 == pdPASS);

  size_t free_heap = xPortGetFreeHeapSize();

  configASSERT(free_heap > 0);
  vTaskStartScheduler();

  configASSERT(0); // insufficient RAM->scheduler task returns->vAssertCalled() called

end:
  while (1);

  return 0;
}
