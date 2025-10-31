#include "FreeRTOS.h"
#include "task.h"


#define TASK1_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK2_PRIORITY (tskIDLE_PRIORITY + 1)

#define TASK1_PARAMETER (1)
#define TASK2_PARAMETER (2)

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

void vPortSetupTimerInterrupt( void ){

//define if you need to set the timer interrupt,otherwise an empty definition is still necessary to overwrite the weak definition in port.c and to avoid
//unwanted jumps to reset handler

}

int main(){

    /* Insert your code here */
       
    BaseType_t mem_1=xTaskCreate(Task1,"t1", configMINIMAL_STACK_SIZE,
			(void *)TASK1_PARAMETER,
			TASK1_PRIORITY,
			NULL);

    BaseType_t mem_2=xTaskCreate(Task2,"t2", configMINIMAL_STACK_SIZE,
			(void *)TASK2_PARAMETER, TASK2_PRIORITY,
			NULL);
			
    configASSERT( mem_1 == pdPASS );
    
    configASSERT( mem_2 == pdPASS );
    
    size_t free_heap=xPortGetFreeHeapSize();
    
    configASSERT( free_heap > 0 );
			    
    vTaskStartScheduler();

    configASSERT(0); //insufficient RAM->scheduler task returns->vAssertCalled() called
    
    while(1);

    return 0;
}

