#include "cyhal.h"
#include "cybsp.h"
#include "task.h"
#include "FreeRTOS.h"


void task_blink(void* param) {
  /* Suppress warning for unused parameter */
  (void) param;

  /* Repeatedly running part of the task */
  for (;;) {
    cyhal_gpio_toggle(CYBSP_USER_LED);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
int main(void) {
  cy_rslt_t result;

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

  xTaskCreate(task_blink, "blink task", configMINIMAL_STACK_SIZE,
    NULL, configMAX_PRIORITIES - 7, NULL);

  vTaskStartScheduler();
  /* RTOS scheduler exited */
  /* Halt the CPU if scheduler exits */
  CY_ASSERT(0);

  for (;;) {
  }
}
