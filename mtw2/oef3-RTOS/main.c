
/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"



void HighTask(void* param) {
  /* Suppress warning for unused parameter */
  (void) param;

  /* Repeatedly running part of the task */
for(;;){
    	printf("Hello World! \r\n");
    	vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void MedTask(void* param) {
  /* Suppress warning for unused parameter */
  (void) param;
int counter = 0;
  /* Repeatedly running part of the task */
TaskHandle_t xHandle;

xTaskCreate(HighTask, "HighTask", configMINIMAL_STACK_SIZE,
  NULL, configMAX_PRIORITIES - 7, &xHandle);

  for(;;){
	  	  printf("Counter = %d \r\n",counter);
	  	  counter++;
	  	  vTaskDelay(pdMS_TO_TICKS(2000));

	  	  if(counter == 5){
	  		printf("Suspended Task 1 \r\n");
	  		vTaskSuspend(xHandle);
	  	  }
	  	  if(counter == 7){
	  		printf("Resumed Task 1 \r\n");
	  		vTaskResume(xHandle);
	  	  }

	  	  if(counter == 10){counter = 0;}
  	}
}


int main(void) {
  cy_rslt_t result;
//  TaskHandle_t xHandle;

  /* Initialize the device and board peripherals */
  result = cybsp_init();
  if (result != CY_RSLT_SUCCESS) {
    CY_ASSERT(0);
  }

  /* Initialize the User LED */
  result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
      CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

  /* GPIO init failed. Stop program execution */
  if (result != CY_RSLT_SUCCESS) {
    CY_ASSERT(0);
  }

  __enable_irq();



  cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
  	                CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,CY_RETARGET_IO_BAUDRATE);

  printf("--------RTOS Test oef3------------ \r\n");


  xTaskCreate(MedTask, "MedTask", configMINIMAL_STACK_SIZE,
     NULL, configMAX_PRIORITIES - 7, NULL);

  vTaskStartScheduler();
  /* RTOS scheduler exited */
  /* Halt the CPU if scheduler exits */
  CY_ASSERT(0);

  for (;;) {
  }
}

/* [] END OF FILE */
