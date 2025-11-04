#include "FreeRTOS.h"
#include "task.h"
#include "uninasoc.h"

static xlnx_tim_t timer = {
  .base_addr = TIM0_BASEADDR,
  .counter = 20000000,
  .reload_mode = TIM_RELOAD_AUTO,
  .count_direction = TIM_COUNT_DOWN
};

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

    printf("Hello from Task\n\r");
  }
}

void vExternalTimerTickHandler(void) {
  xlnx_tim_clear_int(&timer);
  xTaskIncrementTick();
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

int main() {

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
