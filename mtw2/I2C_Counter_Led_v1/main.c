/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* Delay of 1000ms between commands */
#define CMD_TO_CMD_DELAY        (1000UL)

/* Packet positions */

/* Start and end of packet markers */
//#define PACKET_SOP              (0x01UL)
//#define PACKET_EOP              (0x17UL)

/* I2C slave address to communicate with */
#define I2C_SLAVE_ADDR          (0x3F)

/* I2C bus frequency */
#define I2C_FREQ                (12000UL)

/* Command valid status */

/* Packet size */
#define PACKET_SIZE             (2UL)

#ifdef XMC7200D_E272K8384
#define KIT_XMC72
#endif

#define Led					(112)
#define	RoodLed				(96)
#define	GeelLed				(80)
#define	GroenLed			(48)

uint8_t State = 0;

void ToggleAtEdge(void* handler_arg, cyhal_gpio_event_t event){

	State += 1;
}


int main(void)
{

    cyhal_i2c_t mI2C;
    cyhal_i2c_cfg_t mI2C_cfg;

    /* Initialize the device and board peripherals */
    cybsp_init();

    /* Initialize the retarget-io */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "HAL: I2C Master "
           "****************** \r\n\n");

    /* I2C Master configuration settings */
    printf(">> Configuring I2C Master..... ");
    mI2C_cfg.is_slave = false;
    mI2C_cfg.address = 0;
    mI2C_cfg.frequencyhal_hz = I2C_FREQ;

    /* Init I2C master */
    cyhal_i2c_init(&mI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);

    /* Configure I2C Master */
    cyhal_i2c_configure(&mI2C, &mI2C_cfg);

    printf("Done\r\n\n");

    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);		// Button
    cyhal_gpio_init(P11_1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);	// Blue
    cyhal_gpio_init(P1_1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true );	// Green
    cyhal_gpio_init(P0_3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);		// Red


    //    Set callback interrupts :
    	cyhal_gpio_callback_data_t ButtonPressed = { .callback = ToggleAtEdge };
    		cyhal_gpio_register_callback(P0_4, &ButtonPressed);
    		cyhal_gpio_enable_event(P0_4, CYHAL_GPIO_IRQ_FALL, 3 , true);


    	uint8_t buffer[2] = {Led}; // Initialize buffer
    /* Enable interrupts */
    __enable_irq();

    for (;;) {
        switch (State) {
            case 1:
                buffer[1] = Led;
                printf("Count : 0 Leds Out \r\n\n");
                break;
            case 2:
                buffer[1] = GroenLed;
                printf("Count : 1 \r\n\n");
                break;
            case 3:
                buffer[1] = GeelLed;
                printf("Count : 2 \r\n\n");
                break;
            case 4:
                buffer[1] = RoodLed;
                printf("Count : 4 \r\n\n");
                break;

            default:
                State = 0; // Reset State to 0 if it goes beyond 8
                break;
        }
        cyhal_i2c_master_write(&mI2C, I2C_SLAVE_ADDR, buffer, PACKET_SIZE, 0, true);
        cyhal_system_delay_ms(CMD_TO_CMD_DELAY);
        State++;
    }
}
