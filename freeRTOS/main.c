/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

//SemaphoreHandle_t guardinha;
uint8_t* vetor;
const HeapRegion_t xHeapRegions[] =

{

    { 0x2500, 1024 },

    { NULL, 0 } /* Marks the end of the array. */

};
//StaticTask_t t1Buffer;
//StackType_t s1Buffer[configMINIMAL_STACK_SIZE];
//StaticTask_t t2Buffer;
//StackType_t s2Buffer[configMINIMAL_STACK_SIZE];
//StaticSemaphore_t semBuffer;

//static void blink( void *pvParameters ){
//    while(1){
//        xSemaphoreTake(guardinha, portMAX_DELAY);
//        P1OUT ^= BIT0;
//    }
//}

//static void task2(void *pvParemeters){
//    while(1){
//        while((P5IN & BIT5));
//        vTaskDelay(50);
//        xSemaphoreGive(guardinha);
//        vTaskDelay(250);
//    }
//}

int main( void )
{

    /* Configure system. */
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;
    P1DIR |= BIT0;
    P1DIR |= BIT1;

    P5DIR &= ~BIT5;
    P5REN |= BIT5;
    P5OUT |= BIT5;

    P1OUT &= ~BIT0;
    P1OUT &= ~BIT1;

//#if( configSUPPORT_STATIC_ALLOCATION == 1 )
//    guardinha = xSemaphoreCreateBinaryStatic(&semBuffer);
//#endif
//#if( configSUPPORT_STATIC_ALLOCATION == 0 )
//    guardinha = xSemaphoreCreateBinary();
//#endif


    /* Create the Task */
//#if( configSUPPORT_STATIC_ALLOCATION == 1 )
//    xTaskCreateStatic( blink,                             /* The function that implements the task. */
//                "Blink",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
//                configMINIMAL_STACK_SIZE,           /* The size of the stack to allocate to the task. */
//                NULL,                               /* The parameter passed to the task - not used in this case. */
//                2,                                  /* The priority assigned to the task. */
//                s1Buffer,
//                &t1Buffer);                             /* The task handle is not required, so NULL is passed. */
//#endif
//#if( configSUPPORT_STATIC_ALLOCATION == 0 )
//    xTaskCreate( blink,                             /* The function that implements the task. */
//                    "Blink",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
//                    configMINIMAL_STACK_SIZE,           /* The size of the stack to allocate to the task. */
//                    NULL,                               /* The parameter passed to the task - not used in this case. */
//                    2,                                  /* The priority assigned to the task. */
//                    NULL);
//#endif
//#if( configSUPPORT_STATIC_ALLOCATION == 1 )
//    xTaskCreateStatic(task2,
//                "Poster",
//                configMINIMAL_STACK_SIZE,
//                NULL,
//                2,
//                s2Buffer,
//                &t2Buffer);
//#endif
//#if( configSUPPORT_STATIC_ALLOCATION == 0 )
//    xTaskCreate(task2,
//                    "Poster",
//                    configMINIMAL_STACK_SIZE,
//                    NULL,
//                    2,
//                    NULL);
    vPortDefineHeapRegions(xHeapRegions);
    vetor = (uint8_t*) pvPortMalloc(1015);
    if(vetor != 0 || vetor != NULL){
        P1OUT |= BIT1;
    }else{
        P1OUT |= BIT0;
    }
    vPortFree(vetor);
//#endif
    /* Start RTOS */
    //vTaskStartScheduler();

}

