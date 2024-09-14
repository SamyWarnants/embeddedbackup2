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
#define PACKET_SOP              (0x01UL)
#define PACKET_EOP              (0x17UL)

/* I2C slave address to communicate with */
#define I2C_SLAVE_ADDR          (0x3F)

/* I2C bus frequency */
#define I2C_FREQ                (100000UL)

/* Command valid status */
#define STATUS_CMD_DONE         (0x00UL)

/* Packet size */
#define PACKET_SIZE             (1UL)

#ifdef XMC7200D_E272K8384
#define KIT_XMC72
#endif


void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}



int main(void)
{
    cy_rslt_t result;
    cyhal_i2c_t mI2C;
    cyhal_i2c_cfg_t mI2C_cfg;
    uint8_t buffer[PACKET_SIZE];

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    handle_error(result);


    /* Initialize the retarget-io */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                  CY_RETARGET_IO_BAUDRATE);
    /* Retarget-io init failed. Stop program execution */
    handle_error(result);

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
    result = cyhal_i2c_init(&mI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    /* I2C master init failed. Stop program execution */
    handle_error(result);

    /* Configure I2C Master */
    result = cyhal_i2c_configure(&mI2C, &mI2C_cfg);
    /* I2C master configuration failed. Stop program execution */
    handle_error(result);

    printf("Done\r\n\n");

    /* Enable interrupts */
    __enable_irq();

    for (;;)
        {
            uint8_t cmd = (0B01100000);
            buffer[1] = cmd;

            cyhal_i2c_master_write(&mI2C, I2C_SLAVE_ADDR, buffer, PACKET_SIZE, 0, true);
                /* Give delay between commands */
            cyhal_system_delay_ms(CMD_TO_CMD_DELAY);

            cmd = (0B1010000);

            buffer[1] = cmd;

            cyhal_i2c_master_write(&mI2C, I2C_SLAVE_ADDR, buffer, PACKET_SIZE, 0, true);
                        /* Give delay between commands */
            cyhal_system_delay_ms(CMD_TO_CMD_DELAY);

            cmd = (0B11011111);

            buffer[1] = cmd;

            cyhal_i2c_master_write(&mI2C, I2C_SLAVE_ADDR, buffer, PACKET_SIZE, 0, true);
                                /* Give delay between commands */
            cyhal_system_delay_ms(CMD_TO_CMD_DELAY);

        }
}
