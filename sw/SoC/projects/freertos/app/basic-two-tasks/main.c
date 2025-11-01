#include "FreeRTOS.h"
#include "task.h"

#ifdef USE_UNINASOC
#include "uninasoc.h"

#define SOURCES_NUM 3
static xlnx_tim_t timer = {
    .base_addr = TIM0_BASEADDR,
    .counter = 20000000,
    .reload_mode = TIM_RELOAD_AUTO,
    .count_direction = TIM_COUNT_DOWN
};
#endif // USE_UNINASOC

#define TASK1_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK2_PRIORITY (tskIDLE_PRIORITY + 1)

#define TASK1_PARAMETER (1)
#define TASK2_PARAMETER (2)


static void Task1(void *pvParameters) {

    uint32_t a;
    uint32_t b = 3;

    configASSERT( ( ( uint32_t ) pvParameters ) == TASK1_PARAMETER );
    size_t free_heap = xPortGetFreeHeapSize();
    configASSERT( free_heap > 0 );

    while (1) {
    	free_heap = xPortGetFreeHeapSize();
        configASSERT( free_heap > 0 );
    	a = b + (uint32_t) pvParameters;
        (void) a;
        taskYIELD();
    }
}

static void Task2(void *pvParameters) {

    uint32_t a;
    uint32_t b = 3;
    
    configASSERT( ( ( uint32_t ) pvParameters ) == TASK2_PARAMETER );
    size_t free_heap = xPortGetFreeHeapSize();
    configASSERT( free_heap > 0 );

    while (1) {
    	    free_heap = xPortGetFreeHeapSize();
            configASSERT( free_heap > 0 );

	    a = b - (uint32_t) pvParameters;
            (void)a;
	    taskYIELD();
   } 
}

void vAssertCalled(const char *file, int line) {

     const char *f=file;
     int l=line;
     (void) f;
     (void) l;
     __asm("ebreak");
}

void vPortSetupTimerInterrupt(void){

#ifdef USE_UNINASOC
    int ret;
    uint32_t priorities[SOURCES_NUM] = { 1, 1, 1 };
    plic_init();
    plic_configure(priorities, SOURCES_NUM);
    plic_enable_all();

    ret = xlnx_tim_init(&timer);
    if(ret != UNINASOC_OK) {
	printf("Cannot init TIMER\n\r");
	return;
    }
 
    ret = xlnx_tim_configure(&timer);
    if(ret != UNINASOC_OK) {
	printf("Cannot configure TIMER\n\r");
	return;
    }

    ret = xlnx_tim_enable_int(&timer);
    if (ret != UNINASOC_OK) {
        printf("Cannot enable timer interrupt\r\n");
	return;
    }

    ret = xlnx_tim_start(&timer);
    if (ret != UNINASOC_OK) {
        printf("Cannot start timer\r\n");
	return;
    }
#endif // USE_UNINASOC
}

int main(){

#ifdef USE_UNINASOC
    uninasoc_init();
    printf("SIMPLY-V FreeRTOS DEMO!\n\r");
#endif // USE_UNINASOC


    // Create FreeRTOS Task
    BaseType_t mem_1=xTaskCreate(Task1,"t1", configMINIMAL_STACK_SIZE,
			(void *)TASK1_PARAMETER,
			TASK1_PRIORITY,
			NULL);

    BaseType_t mem_2=xTaskCreate(Task2,"t2", configMINIMAL_STACK_SIZE,
			(void *)TASK2_PARAMETER, TASK2_PRIORITY,
			NULL);

    configASSERT( mem_1 == pdPASS );
  
    configASSERT( mem_2 == pdPASS );
 
    size_t free_heap = xPortGetFreeHeapSize();
 
    configASSERT( free_heap > 0 );
    vTaskStartScheduler();

    configASSERT(0); //insufficient RAM->scheduler task returns->vAssertCalled() called

    while(1);

    return 0;
}

