/******************************************************************************
* File Name:   main.c
*
* Description: This example project demonstrates the basic operation of the
* I2C resource as master with EzI2C resource as slave using HAL APIs. 
* The I2C master sends the command packets to the EzI2C slave to control an user LED.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2019-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "resource_map.h"

/***************************************
*            Constants
****************************************/
#define CMD_TO_CMD_DELAY        (1000UL)
#define PACKET_SOP_POS          (0UL)
#define PACKET_CMD_POS          (1UL)
#define PACKET_EOP_POS          (2UL)
#define PACKET_RPLY_SOP_POS     (3UL)
#define PACKET_RPLY_STS_POS     (4UL)
#define PACKET_RPLY_EOP_POS     (5UL)

/* Start and end of packet markers */
#define PACKET_SOP              (0x01UL)
#define PACKET_EOP              (0x17UL)

/* I2C slave address to communicate with */
#define EzI2C_SLAVE_ADDR        (0x3F)

/* I2C slave interrupt priority */
#define I2C_SLAVE_IRQ_PRIORITY  (7u)

/* Command valid status */
#define STS_CMD_DONE            (0x00UL)
#define STS_CMD_FAIL            (0xFFUL)

/* Buffer and packet size */
#define PACKET_SIZE             (4UL)
#define EZI2C_BUFFER_SIZE       (6UL)

/***************************************
*          Global Variables
****************************************/
#if ((I2C_MODE == I2C_MODE_BOTH) || (I2C_MODE == I2C_MODE_SLAVE))
uint8_t ezi2c_buffer[EZI2C_BUFFER_SIZE] = {0};
#endif

#ifdef XMC7200D_E272K8384
#define XMC_Series
#endif

#ifdef XMC7100D_F176K4160
#define XMC_Series
#endif

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

#if ((I2C_MODE == I2C_MODE_BOTH) || (I2C_MODE == I2C_MODE_SLAVE))
/*******************************************************************************
* Function Name: handle_slave_event
********************************************************************************
* Summary:
* This is a callback event for EzI2C slave. If a write event occurs, 
* the command packet is verified and executed.
*
* Parameters:
*  callback_arg : extra argument that can be passed to callback
*  event        : EzI2C event
*
* Return:
*  void
*
*******************************************************************************/
void handle_slave_event(void *callback_arg, cyhal_ezi2c_status_t event)
{

    if (0UL == (CYHAL_EZI2C_STATUS_ERR & event))
    {
        if (0UL != (CYHAL_EZI2C_STATUS_WRITE1 & event))
        {
            /* Check start and end of packet markers. */
            if ((ezi2c_buffer[PACKET_SOP_POS] == PACKET_SOP) &&
                (ezi2c_buffer[PACKET_EOP_POS] == PACKET_EOP))
                {
                    /* Execute command */
                    cyhal_gpio_write( CYBSP_USER_LED, ezi2c_buffer[PACKET_CMD_POS]);
                    
                    /* Update status of received command. */
                    ezi2c_buffer[PACKET_RPLY_SOP_POS] = PACKET_SOP;
                    ezi2c_buffer[PACKET_RPLY_STS_POS] = STS_CMD_DONE;
                    ezi2c_buffer[PACKET_RPLY_EOP_POS] = PACKET_EOP;
                }

            /* Update buffer for the next write */
            ezi2c_buffer[PACKET_SOP_POS] = 0;
            ezi2c_buffer[PACKET_EOP_POS] = 0;
        }

        if (0UL != (CYHAL_EZI2C_STATUS_READ1 & event))
        {
            /* Update buffer for the next write */
            ezi2c_buffer[PACKET_RPLY_SOP_POS] = PACKET_SOP;
            ezi2c_buffer[PACKET_RPLY_STS_POS] = STS_CMD_FAIL;
            ezi2c_buffer[PACKET_RPLY_EOP_POS] = PACKET_EOP;
        }
    }
}
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function.
*   1. I2C Master sends command packet to the slave
*   2. I2C Master reads the response packet to generate the next command
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    /* Set up the device based on configurator selections */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    result = cy_retarget_io_init( CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 
                                  CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

#if defined(XMC_Series)
        printf("Configuring clock \r\n");
        /*Configure clock settings for KIT_XMC72_EVK */
        cyhal_clock_t clock_fll, clock_hf, clock_peri;
        result = cyhal_clock_reserve(&clock_hf, &CYHAL_CLOCK_HF[0]);
        result = cyhal_clock_reserve(&clock_fll, &CYHAL_CLOCK_FLL);
        if(result == CY_RSLT_SUCCESS){
        result = cyhal_clock_set_source(&clock_hf, &clock_fll);
        }
        /* Set divider to 1 for Peripheral Clock */
        result = cyhal_clock_reserve(&clock_peri, CYHAL_CLOCK_PERI);
        if(result == CY_RSLT_SUCCESS){
        result = cyhal_clock_set_divider(&clock_peri,1);
        }
#endif

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("*********************************\r\n");
    printf("HAL: I2C Master EzI2C Slave\r\n");
    printf("*********************************\r\n\n");

#if ((I2C_MODE == I2C_MODE_BOTH) || (I2C_MODE == I2C_MODE_SLAVE))
    cyhal_ezi2c_t sEzI2C;
    cyhal_ezi2c_slave_cfg_t sEzI2C_sub_cfg;
    cyhal_ezi2c_cfg_t sEzI2C_cfg;

    /* Configure user LED */
    printf(">> Configuring user LED..... ");
    result = cyhal_gpio_init( CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, 
                              CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("Done\r\n");

    /* Configure EzI2C Slave */
    printf(">> Configuring EzI2C Slave..... ");
    sEzI2C_sub_cfg.buf = ezi2c_buffer;
    sEzI2C_sub_cfg.buf_rw_boundary = EZI2C_BUFFER_SIZE;
    sEzI2C_sub_cfg.buf_size = EZI2C_BUFFER_SIZE;
    sEzI2C_sub_cfg.slave_address = EzI2C_SLAVE_ADDR;

    sEzI2C_cfg.data_rate = CYHAL_EZI2C_DATA_RATE_400KHZ;
    sEzI2C_cfg.enable_wake_from_sleep = false;
    sEzI2C_cfg.slave1_cfg = sEzI2C_sub_cfg;
    sEzI2C_cfg.sub_address_size = CYHAL_EZI2C_SUB_ADDR8_BITS;
    sEzI2C_cfg.two_addresses = false;

    result = cyhal_ezi2c_init( &sEzI2C, sI2C_SDA, sI2C_SCL, NULL, &sEzI2C_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("Done\r\n");

    cyhal_ezi2c_register_callback( &sEzI2C, handle_slave_event, NULL);
    cyhal_ezi2c_enable_event( &sEzI2C,
                              (CYHAL_EZI2C_STATUS_ERR    
                             | CYHAL_EZI2C_STATUS_WRITE1 
                             | CYHAL_EZI2C_STATUS_READ1),
                             I2C_SLAVE_IRQ_PRIORITY, true);
#endif

#if ((I2C_MODE == I2C_MODE_BOTH) || (I2C_MODE == I2C_MODE_MASTER))
    cyhal_i2c_t mI2C;
    cyhal_i2c_cfg_t mI2C_cfg;
    uint8_t cmd = CYBSP_LED_STATE_ON;
    uint8_t buffer = 0;
    uint8 step = 0;


    /* Configure I2C Master */
    printf(">> Configuring I2C master..... ");
    mI2C_cfg.is_slave = false;
    mI2C_cfg.address = 0;
    mI2C_cfg.frequencyhal_hz = CYHAL_EZI2C_DATA_RATE_400KHZ;
    result = cyhal_i2c_init( &mI2C, mI2C_SDA, mI2C_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    result = cyhal_i2c_configure( &mI2C, &mI2C_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("Done\r\n\n");
#endif
    /* Enable interrupts */
    __enable_irq();

#if ((I2C_MODE == I2C_MODE_BOTH) || (I2C_MODE == I2C_MODE_SLAVE))
    printf("User LED should start blinking \r\n");
#endif

    for (;;)
        {
            /* create packet to be sent to slave.  */
            switch (step) {
                case 0:
                    buffer = (uint8_t)~0x40;//GREEN
                    break;
                case 1:
                    buffer = (uint8_t)~0x20;//YELLOW
                    break;
                case 2:
                	buffer = (uint8_t)~0x10;//RED
                    break;
                case 3:
                    buffer = (uint8_t)~0x80; //OFF
                    break;
                default:
                    buffer = (uint8_t)~0x00;
                    break;
            }

            /* Send packet with command to the slave. */
            if (CY_RSLT_SUCCESS == cyhal_i2c_master_write(&mI2C, EzI2C_SLAVE_ADDR, &buffer, 1UL, 0, true)){step++;}
            if(step == 4){step = 0;}
            /* Give delay between commands. */
            cyhal_system_delay_ms(CMD_TO_CMD_DELAY);
         }
    }
