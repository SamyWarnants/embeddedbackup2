#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdio.h>
#include <time.h>

#define TM_YEAR_BASE (1900u)

cyhal_rtc_t my_rtc;
volatile bool button_pressed = false; // Flag to track button presses

void print_time(const char *message);
void button_isr(void *handler_arg, cyhal_gpio_event_t event);
void RTC_alarm_isr(void *handler_arg, cyhal_rtc_event_t event);

int main(void)
{
    cy_rslt_t result;
    cyhal_rtc_init(&my_rtc);
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to enable printfs */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
    CY_ASSERT(CY_RSLT_SUCCESS == result);

    /* Enable global interrupts */
    __enable_irq();

    /* Set initial date and time */
    int mday = 2, month = 9, year = 2024; /* Day of month, month and year */
    int hours = 15, minutes = 31, seconds = 00; /* Hours, minutes and seconds */
    int wday = 3; /* Days from Sunday. Sunday is 0, Monday is 1 and so on. */
    int dst = 0; /* Daylight Savings. 0 - Disabled, 1 - Enabled */

    /* Setting the tm structure as 08 HRS:10 MIN:10 SEC ; 3rd March 2020; Wednesday ; DST off */
    struct tm new_date_time =
    {
        .tm_sec = seconds,
        .tm_min = minutes,
        .tm_hour = hours,
        .tm_mday = mday,
        .tm_mon = month - 1,
        .tm_year = year - TM_YEAR_BASE,
        .tm_wday = wday,
        .tm_isdst = dst
    };

    /* Update the current time and date to the RTC peripheral */
    result = cyhal_rtc_write(&my_rtc, &new_date_time);
    CY_ASSERT(CY_RSLT_SUCCESS == result);

    /* Print the initial time */
    print_time("RTC started on");

    /* Set up the button interrupt */
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);

    /* Create and configure the callback data structure */
    cyhal_gpio_callback_data_t cb_data =
    {
        .callback = button_isr,
        .callback_arg = NULL
    };
    cyhal_gpio_register_callback(CYBSP_USER_BTN, &cb_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, CYHAL_ISR_PRIORITY_DEFAULT, true);


    while (1)
    {
        cyhal_rtc_set_alarm_by_seconds(&my_rtc, 15);
        cyhal_rtc_register_callback(&my_rtc, RTC_alarm_isr, NULL);
        cyhal_rtc_enable_event(&my_rtc, CYHAL_RTC_ALARM , CYHAL_ISR_PRIORITY_DEFAULT, true);


        if (button_pressed)
        {
            button_pressed = false; // Clear the flag
            print_time("TIME REQUEST WITH BUTTON: ");
        }

        cyhal_syspm_sleep();
    }
}

void print_time(const char *message)
{
    struct tm current_date_time = {0};
    char buffer[80];

    /* Get the current time and date from the RTC peripheral */
    cy_rslt_t result = cyhal_rtc_read(&my_rtc, &current_date_time);
    CY_ASSERT(CY_RSLT_SUCCESS == result);

    /* Format the current date and time */
    strftime(buffer, sizeof(buffer), "%c", &current_date_time);
    printf("%s %s\r\n", message, buffer);
}
void button_isr(void *handler_arg, cyhal_gpio_event_t event)
{
    (void)handler_arg;
    (void)event;
    button_pressed = true;
}


void RTC_alarm_isr(void *handler_arg, cyhal_rtc_event_t event)
{
    (void)handler_arg;
    (void)event;
    print_time("ALARM TIME REQUEST: ");
}
