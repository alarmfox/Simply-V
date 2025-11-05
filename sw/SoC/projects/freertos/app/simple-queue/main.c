#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "uninasoc.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

#define mainQUEUE_LENGTH (1)

#define mainQUEUE_SEND_PARAMETER (0x1111UL)
#define mainQUEUE_RECEIVE_PARAMETER (0x22UL)

static QueueHandle_t xQueue = NULL;

static void prvQueueMyTask1(void *pvParameters) {
  const unsigned long ulValueToSend = 100UL;

  configASSERT(((unsigned long)pvParameters) == mainQUEUE_SEND_PARAMETER);

  size_t free_heap;

  while (1) {

    free_heap = xPortGetFreeHeapSize();
    configASSERT(free_heap > 0);

    printf("Task1: sending %ld...", ulValueToSend);

    xQueueSend(xQueue, &ulValueToSend, 0U);

    printf("done\n\r");

    taskYIELD();
  }
}

static void prvQueueMyTask2(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    unsigned long ulReceivedValue;

    printf("Task2: receiving value...");

    xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

    printf("got %ld\n\r", ulReceivedValue);

    size_t free_heap = 0;
    free_heap = xPortGetFreeHeapSize();
    configASSERT(free_heap > 0);

    if (ulReceivedValue == 100UL) { ulReceivedValue = 0U; }
    configASSERT(ulReceivedValue == 0U);
    taskYIELD();
  }
}

void vAssertCalled(const char *file, int line) {

  const char *f = file;
  int l = line;
  (void)f;
  (void)l;
  __asm("ebreak");
}

void vPortSetupTimerInterrupt(void) {
  // define if you need to set the timer interrupt,otherwise an empty definition
  // is still necessary to overwrite the weak definition in port.c and to avoid
  // unwanted jumps to reset handler
}

#ifdef configCHECK_FOR_STACK_OVERFLOW
/* Called if a task overflows its stack space */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;

  __asm volatile("ebreak");
}
#endif // configCHECK_FOR_STACK_OVERFLOW

#ifdef configUSE_MALLOC_FAILED_HOOK
/* Called if pvPortMalloc() fails (heap exhausted) */
void vApplicationMallocFailedHook(void) {
  __asm volatile("ebreak");
}
#endif // configUSE_MALLOC_FAILED_HOOK

int main() {

  uninasoc_init();
  printf("================= Simply-V Producer - Consumer with Yield "
         "==================\n\r");

  xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));
  configASSERT(xQueue != NULL);

  BaseType_t mem_1 =
      xTaskCreate(prvQueueMyTask2, "MyTask2", configMINIMAL_STACK_SIZE,
                  (void *)mainQUEUE_RECEIVE_PARAMETER,
                  mainQUEUE_RECEIVE_TASK_PRIORITY, NULL);

  BaseType_t mem_2 = xTaskCreate(
      prvQueueMyTask1, "MyTask1", configMINIMAL_STACK_SIZE,
      (void *)mainQUEUE_SEND_PARAMETER, mainQUEUE_SEND_TASK_PRIORITY, NULL);

  configASSERT(mem_1 == pdPASS);

  configASSERT(mem_2 == pdPASS);

  size_t free_heap = xPortGetFreeHeapSize();

  configASSERT(free_heap > 0);

  vTaskStartScheduler();

  configASSERT(
      0); // insufficient RAM->scheduler task returns->vAssertCalled() called

  while (1)
    ;

  return 0;
}
