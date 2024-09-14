/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for RTC alarm periodic wakeup Example
* using HAL APIs.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2023, Cypress Semiconductor Corporation (an Infineon company) or
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

/******************************************************************************
* Header files
******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Initial Time and Date definitions */
#define RTC_INITIAL_DATE_SEC        0u
#define RTC_INITIAL_DATE_MIN        17u
#define RTC_INITIAL_DATE_HOUR       16u
#define RTC_INITIAL_DATE_DAY        28u
#define RTC_INITIAL_DATE_MONTH      2u
#define RTC_INITIAL_DATE_YEAR       2022u
#define RTC_INTERRUPT_PRIORITY      3u
#define STRING_BUFFER_SIZE          80u
/* Constants to define LONG and SHORT presses on User Button (x10 = ms) */
#define SHORT_PRESS_COUNT           10u     /* 100 ms < press < 2 sec */
#define LONG_PRESS_COUNT            200u    /* press > 2 sec */
/* Glitch delays */
#define SHORT_GLITCH_DELAY_MS       10u     /* in ms */
#define LONG_GLITCH_DELAY_MS        100u    /* in ms */
/* Macro for Alarm set by seconds */
#define USE_SECONDS_FOR_ALARM 10

/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_rtc_t rtc_obj;
/* User button was pressed for how long */
typedef enum
{
    SWITCH_NO_EVENT     = 0u,
    SWITCH_SHORT_PRESS  = 1u,
    SWITCH_LONG_PRESS   = 2u,
} en_switch_event_t;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void set_rtc_alarm_date_time(void);
en_switch_event_t get_switch_event(void);
void debug_printf(const char *str);
void handle_error(uint32_t status);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for the MCU. It does...
*    1. Initialize the UART and RTC blocks.
*    2. Check the reset reason, if it is wakeup from Hibernate power mode, then
*       set RTC initial time and date.
*    Do Forever loop:
*    3. Check if User button was pressed and for how long.
*    4. If short pressed, set the RTC alarm and then go to DeepSleep mode.
*    5. If long pressed, set the RTC alarm and then go to Hibernate mode.
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
    handle_error(result);

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io for uart logging */
    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                    CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,
                                    CY_RETARGET_IO_BAUDRATE);

    /* Retarget-io init failed. Stop program execution */
    handle_error(result);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("*************************************************************\r\n");
    printf("HAL: RTC periodic wakeup alarm example\r\n");
    printf("*************************************************************\r\n");
    printf("Short press 'SW2' key to DeepSleep mode.\r\n\r\n");
    printf("Long press 'SW2' key to Hibernate mode.\r\n\r\n");

    /* Initialize the User Button */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    /* User button init failed. Stop program execution */
    handle_error(result);

    /* Initialize RTC */
    result = cyhal_rtc_init(&rtc_obj);
    /* RTC init failed. Stop program execution */
    handle_error(result);

    /* Check the reset reason */
    if(CYHAL_SYSTEM_RESET_HIB_WAKEUP ==
        (cyhal_system_get_reset_reason() & CYHAL_SYSTEM_RESET_HIB_WAKEUP))
    {
        /* The reset has occurred on a wakeup from Hibernate power mode */
        debug_printf("Wakeup from the Hibernate mode\r\n");
    }
    else
    {
        /* Update the initialization time and date to the RTC peripheral */
        result = cyhal_rtc_write_direct(&rtc_obj,
                                        RTC_INITIAL_DATE_SEC,
                                        RTC_INITIAL_DATE_MIN,
                                        RTC_INITIAL_DATE_HOUR,
                                        RTC_INITIAL_DATE_DAY,
                                        RTC_INITIAL_DATE_MONTH,
                                        RTC_INITIAL_DATE_YEAR);
        handle_error(result);
    }

    /* Print the current date and time by UART */
    debug_printf("Current date and time.\r\n");

    /* Enable the alarm event to trigger an interrupt */
    cyhal_rtc_enable_event(&rtc_obj,
                            CYHAL_RTC_ALARM,
                            RTC_INTERRUPT_PRIORITY,
                            true);

    for (;;)
    {
        switch (get_switch_event())
        {
            case SWITCH_SHORT_PRESS:
                debug_printf("Go to DeepSleep mode\r\n");

                /* Set the RTC generate alarm after 10 seconds */
                set_rtc_alarm_date_time();
                cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);

                /* Go to deep sleep */
                cyhal_syspm_deepsleep();
                debug_printf("Wakeup from DeepSleep mode\r\n");
                break;

            case SWITCH_LONG_PRESS:
                debug_printf("Go to Hibernate mode\r\n");

                /*Set the RTC generate alarm after 10 seconds */
                set_rtc_alarm_date_time();
                cyhal_system_delay_ms(LONG_GLITCH_DELAY_MS);

                /*Go to hibernate and configure the RTC alarm as wakeup source*/
                cyhal_syspm_hibernate(CYHAL_SYSPM_HIBERNATE_RTC_ALARM);
                break;

            default:
                break;
        }
    }
}

/*******************************************************************************
* Function Name: set_rtc_alarm_date_time
********************************************************************************
* Summary:
*  This functions sets the RTC alarm date and time.
*
* Parameter:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void set_rtc_alarm_date_time(void)
{
    cy_rslt_t result;

    /* Print the RTC alarm time by UART */
    debug_printf("RTC alarm will be generated after 10 seconds\r\n");
    /* Set the RTC alarm for the specified number of seconds in the future by
     editing the macro(USE_SECONDS_FOR_ALARM) */
    result = cyhal_rtc_set_alarm_by_seconds(&rtc_obj, USE_SECONDS_FOR_ALARM);
    /* Setting RTC Alarm failed. Stop program execution */
    handle_error(result);
}

/*******************************************************************************
* Function Name: get_switch_event
********************************************************************************
* Summary:
*  Returns how the User button was pressed:
*  - SWITCH_NO_EVENT: No press
*  - SWITCH_SHORT_PRESS: Short press was detected
*  - SWITCH_LONG_PRESS: Long press was detected
*
* Parameter:
*  void
*
* Return:
*  Switch event that occurred, if any.
*
*******************************************************************************/
en_switch_event_t get_switch_event(void)
{
    en_switch_event_t event = SWITCH_NO_EVENT;
    uint32_t press_count = 0;

    /* Check if User button is pressed */
    while (cyhal_gpio_read(CYBSP_USER_BTN) == CYBSP_BTN_PRESSED)
    {
        /* Wait for 10 ms */
        cyhal_system_delay_ms(SHORT_GLITCH_DELAY_MS);

        /* Increment counter. Each count represents 10 ms */
        press_count++;
    }

    /* Check for how long the button was pressed */
    if (press_count > LONG_PRESS_COUNT)
    {
        event = SWITCH_LONG_PRESS;
    }
    else if (press_count > SHORT_PRESS_COUNT)
    {
        event = SWITCH_SHORT_PRESS;
    }

    /* Add a delay to avoid glitches */
    cyhal_system_delay_ms(SHORT_GLITCH_DELAY_MS);

    return event;
}

/*******************************************************************************
* Function Name: debug_printf
********************************************************************************
* Summary:
* This function prints out the current date time and user string.
*
* Parameters:
*  str      Point to the user print string.
*
* Return:
*  void
*
*******************************************************************************/
void debug_printf(const char *str)
{
    struct tm current_date_time = {0};
    char buffer[STRING_BUFFER_SIZE];
    cy_rslt_t result;

    /* Get the current time and date from the RTC peripheral */
    result = cyhal_rtc_read(&rtc_obj, &current_date_time);
    handle_error(result);
    /* strftime() is a C library function which is used to format date and time
     * into string.
     * It comes under the header file "time.h" which is included by HAL (See
     * http://www.cplusplus.com/reference/ctime/strftime/)
     */
    strftime(buffer, sizeof(buffer), "%X %F", &current_date_time);
    /* Print the the current date and time and user string */
    printf("%s: %s", buffer, str);
}



/* [] END OF FILE */
