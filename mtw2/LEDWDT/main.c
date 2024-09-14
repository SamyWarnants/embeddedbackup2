/*******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the Watchdog Timer Example for
* ModusToolbox.
*
* Related Document: See README.md
*
********************************************************************************
* Copyright 2022-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software is owned by Cypress Semiconductor Corporation or one of its affiliates
* and is protected by worldwide patent protection, U.S. copyright laws, and international
* treaty provisions. Use of this software is limited to the terms in the license agreement
* accompanying the software package.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED. Cypress reserves the right to make changes to the Software without notice.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyhal_wdt.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* WDT timeout for reset mode, in milliseconds. Max limit is given by
 * CYHAL_WDT_MAX_TIMEOUT_MS */
#define WDT_TIME_OUT_MS                     4000
#define ENABLE_BLOCKING_FUNCTION            0

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* WDT object */
cyhal_wdt_t wdt_obj;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void initialize_wdt(void);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function which demonstrates the WDT reset
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

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0); // Initialization failed
    }

    /* Initialize the User LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
            CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0); // LED initialization failed
    }

    /* Initialize the retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                     CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0); // retarget-io initialization failed
    }

    printf("\x1b[2J\x1b[;H");
    printf("******************"
           "HAL: Watchdog Timer"
           "****************** \r\n\n");

    /* Check the reason for device restart */
    if (CYHAL_SYSTEM_RESET_WDT == (cyhal_system_get_reset_reason() &
            CYHAL_SYSTEM_RESET_WDT))
    {
        printf("Reset event from Watchdog Timer\r\n");
        /* It's WDT reset event - blink LED twice */
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        cyhal_system_delay_ms(100);
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
        cyhal_system_delay_ms(200);
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        cyhal_system_delay_ms(100);
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
    }
    else
    {
        printf("Reset event from Power-On or XRES\r\n");
        /* It's Power-On reset or XRES event - blink LED once */
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        cyhal_system_delay_ms(100);
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
        cyhal_system_delay_ms(100);
    }

    /* Clears the reset cause register */
    cyhal_system_clear_reset_reason();

    /* Initialize WDT */
    initialize_wdt();

    /* Enable global interrupt */
    __enable_irq();

    for (;;)
    {
        #if (ENABLE_BLOCKING_FUNCTION)
            while(1);
        #else
            /* Reset WDT */
            cyhal_wdt_kick(&wdt_obj);

            /* Constant delay of 1000ms */
            cyhal_system_delay_ms(1000);

            /* Invert the state of LED */
            cyhal_gpio_toggle(CYBSP_USER_LED);
        #endif
    }
}

/*******************************************************************************
* Function Name: initialize_wdt
********************************************************************************
* Summary:
* This function initializes the WDT block
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void initialize_wdt(void)
{
    cy_rslt_t result;

    /* Initialize the WDT */
    result = cyhal_wdt_init(&wdt_obj, WDT_TIME_OUT_MS);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0); // WDT initialization failed
    }
}
