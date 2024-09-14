#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t taskHandle1, taskHandle2, taskHandle3;

void task1(void *param) {
    for (;;) {
        printf("Hello from task1\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *param) {
    for (;;) {
        printf("Hello from task2\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void task3(void *param) {
    for (;;) {
        printf("Hello from task3\n");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

int main(void) {
    cy_rslt_t result;

    // Initialize the device and board peripherals
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    // Initialize the User LED
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    // GPIO init failed. Stop program execution
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    __enable_irq();

    // Initialize retarget-io to use the debug UART port
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    // Create tasks
    xTaskCreate(task1, "HighTask", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &taskHandle1);
    xTaskCreate(task2, "MedTask", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, &taskHandle2);
    xTaskCreate(task3, "LowTask", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, &taskHandle3);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // RTOS scheduler exited
    // Halt the CPU if scheduler exits
    CY_ASSERT(0);

    for (;;) {
    }
}
