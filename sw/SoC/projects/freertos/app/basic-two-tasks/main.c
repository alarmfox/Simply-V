#include "FreeRTOS.h"
#include "task.h"
#include "uninasoc.h"

#define SOURCES_NUM 3 // regardless of embedded/hpc

#define TASK1_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK2_PRIORITY (tskIDLE_PRIORITY + 1)

#define TASK1_PARAMETER (1)
#define TASK2_PARAMETER (2)

xlnx_tim_t timer = {
    .base_addr = TIM0_BASEADDR,
    .counter = 20000000,
    .reload_mode = TIM_RELOAD_AUTO,
    .count_direction = TIM_COUNT_DOWN
};

static void Task1(void *pvParameters) {

    uint32_t a = 0;
    uint32_t b = 0;
    configASSERT( ( ( uint32_t ) pvParameters ) == TASK1_PARAMETER );
    size_t free_heap=0;
    free_heap=xPortGetFreeHeapSize();
    configASSERT( free_heap > 0 );

    while (1) {
    	free_heap=xPortGetFreeHeapSize();
        configASSERT( free_heap > 0 );
    	a=b+(uint32_t)pvParameters;
        taskYIELD();
    }
}

static void Task2(void *pvParameters) {

    uint32_t a = 0;
    uint32_t b = 3;
    configASSERT( ( ( uint32_t ) pvParameters ) == TASK2_PARAMETER );
    size_t free_heap=0;
    free_heap=xPortGetFreeHeapSize();
    configASSERT( free_heap > 0 );

    while (1) {
    	    free_heap=xPortGetFreeHeapSize();
            configASSERT( free_heap > 0 );
    
	    a=b-(uint32_t)pvParameters;
	    taskYIELD();
   } 
}

void vAssertCalled(const char *file, int line) {

     const char *f=file;
     int l=line;
     __asm("ebreak");
}

void vPortSetupTimerInterrupt(void){
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
}

void _ext_handler(void) {
    // Interrupts are automatically disabled by the microarchitecture.
    // Nested interrupts can be enabled manually by setting the IE bit in the mstatus register,
    // but this requires careful handling of registers.
    // Interrupts are automatically re-enabled by the microarchitecture when the MRET instruction is executed.

    // In this example, the core is connected to PLIC target 1 line.
    // Therefore, we need to access the PLIC claim/complete register 1 (base_addr + 0x200004).
    // The interrupt source ID is obtained from the claim register.
    uint32_t interrupt_id = plic_claim();
    switch(interrupt_id){
        case 0x0: // unused
            break;
        case 0x1:
        #ifdef IS_EMBEDDED
            // Not implemented, just clear to continue
            //xlnx_gpio_in_clear_int();
        #endif
        break;
        case 0x2:
            // Not implemented, just clear to continue
            //xlnx_tim_clear_int();
            break;
        default:
            break;
    }

    // To notify the handler completion, a write-back on the claim/complete register is required.
    plic_complete(interrupt_id);
}

int main(){

    // Init uninasoc HAL
    uninasoc_init();

    printf("SIMPLY-V FreeRTOS DEMO!\n\r");

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

