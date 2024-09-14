#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define MOTOR1_PIN1 (P9_1)
#define MOTOR1_PIN2 (P9_2)
#define MOTOR2_PIN1 (P9_4)
#define MOTOR2_PIN2 (P9_7)
#define MOTOR3_PIN1 (P5_4)
#define MOTOR3_PIN2 (P5_5)
#define MOTOR4_PIN1 (P5_6)
#define MOTOR4_PIN2 (P5_7)

void motors1_on() {
    cyhal_gpio_write(MOTOR1_PIN1, true);
    cyhal_gpio_write(MOTOR1_PIN2, false);
}

void motors1_off() {
    cyhal_gpio_write(MOTOR1_PIN1, false);
    cyhal_gpio_write(MOTOR1_PIN2, false);
}

void motors2_on() {
    cyhal_gpio_write(MOTOR2_PIN1, true);
    cyhal_gpio_write(MOTOR2_PIN2, false);
}

void motors2_off() {
    cyhal_gpio_write(MOTOR2_PIN1, false);
    cyhal_gpio_write(MOTOR2_PIN2, false);
}

void motors3_on() {
    cyhal_gpio_write(MOTOR3_PIN1, true);
    cyhal_gpio_write(MOTOR3_PIN2, false);
}

void motors3_off() {
    cyhal_gpio_write(MOTOR3_PIN1, false);
    cyhal_gpio_write(MOTOR3_PIN2, false);
}

void motors4_on() {
    cyhal_gpio_write(MOTOR4_PIN1, true);
    cyhal_gpio_write(MOTOR4_PIN2, false);
}

void motors4_off() {
    cyhal_gpio_write(MOTOR4_PIN1, false);
    cyhal_gpio_write(MOTOR4_PIN2, false);
}

int main(void) {
    cybsp_init();

    // Initialize pins as outputs
    cyhal_gpio_init(MOTOR1_PIN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR1_PIN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR2_PIN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR2_PIN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR3_PIN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR3_PIN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR4_PIN1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(MOTOR4_PIN2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    // Initialize button pin as input
    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, true);

    __enable_irq();

    for (;;) {
        if(cyhal_gpio_read(P0_4) == false) // Button is pressed
        {

            motors1_on();
            motors2_on();
            motors3_on();
            motors4_on();


           // printf("oh no you stinky why u press mah buttons\n");
        }
        else
        {
            motors1_off();
            motors2_off();
            motors3_off();
            motors4_off();

        }
    }
}
