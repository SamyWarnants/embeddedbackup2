/******************************************************************************
* File Name:   main.c
*
* Description:
*  This file provides reference usage of SCB-UART self tests for  PSoC6 MCU and
*  XMC7000 MCU.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2024, Cypress Semiconductor Corporation (an Infineon company) or
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

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "SelfTest.h"
#include "stdio_user.h"
#include <stdio.h>

/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_INDEX_VAL 0xFFF0u
#define BUFF_CLEAR_DELAY 100u

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_stc_scb_uart_context_t CYBSP_UART_context;
cy_stc_scb_uart_context_t CYBSP_DUT_UART_context;


/***************************************
* Function Prototypes
***************************************/
void SelfTest_UART_SCB_Init(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It configures one SCB instance as an UART and also
* a Smart I/O™ to internally connect the TX and RX pins. The function validates
* bidirectional communication within the UART interface by transmitting data
* via TX and receiving it back through RX.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    uint16_t idx = 100u;
    uint8_t ret;
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
        {
            CY_ASSERT(0);
        }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */

    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
            CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,CY_RETARGET_IO_BAUDRATE);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("Class-B Safety Test: UART Loopback\r\n");

    /* Init UART SelfTest*/
    SelfTest_UART_SCB_Init();

#if defined (CYBSP_SMARTIO_UART_LOOPBACK_HW)
    uint8_t bypassMask;
    /* Configure the Smart I/O block 2 */
    if(CY_SMARTIO_SUCCESS != Cy_SmartIO_Init(CYBSP_SMARTIO_UART_LOOPBACK_HW, &CYBSP_SMARTIO_UART_LOOPBACK_config))
    {
        /* Insert error handling */
        CY_ASSERT(0);
    }

    /* Enable Smart I/O block 2 */
    Cy_SmartIO_Enable(CYBSP_SMARTIO_UART_LOOPBACK_HW);


    /* Bypass all the SmartIO configuration for normal mode */
    bypassMask = 0xFFu;
    Cy_SmartIO_Disable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
    Cy_SmartIO_SetChBypass(CYBSP_SMARTIO_UART_LOOPBACK_HW, bypassMask);
    Cy_SmartIO_Enable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
    /* Need to clear buffer after MUX switch */
    Cy_SysLib_DelayUs(BUFF_CLEAR_DELAY);
#endif

    Cy_SCB_UART_ClearRxFifo(CYBSP_DUT_UART_HW);
    Cy_SCB_UART_ClearTxFifo(CYBSP_DUT_UART_HW);

    for (;;)
    {
        /* If using high level UART APIs with context, run the appropriate stop and abort APIS
         * to ensure interrupt driven high level UART APIs do not trigger during self test*/
        Cy_SCB_UART_StopRingBuffer(CYBSP_DUT_UART_HW, &CYBSP_DUT_UART_context);
        Cy_SCB_UART_AbortReceive(CYBSP_DUT_UART_HW, &CYBSP_DUT_UART_context);
        Cy_SCB_UART_AbortTransmit(CYBSP_DUT_UART_HW, &CYBSP_DUT_UART_context);
#if defined (CYBSP_SMARTIO_UART_LOOPBACK_HW)
        /* Turn on loopback by disabling the bypass on all the SmartIO configuration for test mode */
        Cy_SmartIO_Disable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
        Cy_SmartIO_SetChBypass(CYBSP_SMARTIO_UART_LOOPBACK_HW, CY_SMARTIO_CHANNEL_NONE);
        Cy_SmartIO_Enable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
#endif
        /* Clear RX, TX buffers */
        Cy_SCB_UART_ClearRxFifo(CYBSP_DUT_UART_HW);
        Cy_SCB_UART_ClearTxFifo(CYBSP_DUT_UART_HW);

        /*******************************/
        /* Run UART Self Test...       */
        /*******************************/
        ret = SelfTest_UART_SCB(CYBSP_DUT_UART_HW);

#if defined (CYBSP_SMARTIO_UART_LOOPBACK_HW)
        /* Turn off loopback by enabling the bypass on all the SmartIO configuration for normal mode */
        Cy_SmartIO_Disable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
        Cy_SmartIO_SetChBypass(CYBSP_SMARTIO_UART_LOOPBACK_HW, CY_SMARTIO_CHANNEL_ALL);
        Cy_SmartIO_Enable(CYBSP_SMARTIO_UART_LOOPBACK_HW);
#endif
        /* Need to clear buffer after MUX switch */
        Cy_SysLib_DelayUs(BUFF_CLEAR_DELAY);
        Cy_SCB_UART_ClearRxFifo(CYBSP_DUT_UART_HW);
        Cy_SCB_UART_ClearTxFifo(CYBSP_DUT_UART_HW);

        if ((PASS_COMPLETE_STATUS != ret) && (PASS_STILL_TESTING_STATUS != ret))
        {

            /* Process error */
            printf("Error: UART\r\n");

            /* For demo purposes in case of error detection
             * message is printed to DEBUG Output and code execution
             * is stopped here in a while loop */
            while (1)
            {
            }
        }

        /* Print test counter */
        printf("Test Counter = %x\r\n",idx);

        idx++;
        if (idx > MAX_INDEX_VAL)
        {
            idx = 0u;
        }
    }

}

/*****************************************************************************
* Function Name: SelfTest_UART_SCB_Init
******************************************************************************
*
* Summary:
*  Init UART component and clear internal UART buffers
*  Should be called once before tests
*
* Parameters:
*  NONE
*
* Return:
*  NONE
*
* Note:
*
*****************************************************************************/
void SelfTest_UART_SCB_Init(void)
{

    cy_en_scb_uart_status_t initstatus;

    /* Initialize the UART */
    initstatus = Cy_SCB_UART_Init(CYBSP_DUT_UART_HW, &CYBSP_DUT_UART_config, &CYBSP_DUT_UART_context);

    /* Initialization failed. Handle error */
    if(initstatus!=CY_SCB_UART_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(CYBSP_DUT_UART_HW);

    /* Clear RX, TX buffers */
    Cy_SCB_UART_ClearRxFifo(CYBSP_DUT_UART_HW);
    Cy_SCB_UART_ClearTxFifo(CYBSP_DUT_UART_HW);
}

/* [] END OF FILE */
