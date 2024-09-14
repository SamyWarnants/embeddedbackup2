/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#define CMD_TO_CMD_DELAY        (1000UL)
/******************************************************************************
* Macros
*******************************************************************************/
/* I2C slave address to communicate with */
#define I2C_SLAVE_ADDR          (0x3F)

/* I2C bus frequency */
#define I2C_FREQ                (12000UL)
#define PACKET_SIZE             (2UL)
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
	 // Structure tm stores years since 1900
	#define TM_YEAR_BASE (1900u)
	cyhal_rtc_t my_rtc;

    char        buffer2[80];

    int mday = 30, month = 01, year = 2024; // Day of month, month and year
    int hours = 10, minutes = 56, seconds = 0; // Hours, minutes and seconds
    int wday = 1; // Days from Sunday. Sunday is 0, Monday is 1 and so on.
    int dst  = 0; // Daylight Savings. 0 - Disabled, 1 - Enabled
    cyhal_i2c_t mI2C;
    cyhal_i2c_cfg_t mI2C_cfg;

    // Setting the tm structure as 08 HRS:10 MIN:10 SEC ; 3rd March 2020; Wednesday ; DST off
    struct tm new_date_time =
    {
        .tm_sec   = seconds,
        .tm_min   = minutes,
        .tm_hour  = hours,
        .tm_mday  = mday,
        .tm_mon   = month - 1,
        .tm_year  = year - TM_YEAR_BASE,
        .tm_wday  = wday,
        .tm_isdst = dst
    };
    struct tm current_date_time = {0};



	    cybsp_init();

	    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, true);
	    cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
	                CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,CY_RETARGET_IO_BAUDRATE);



	    // Initialize RTC
	    cyhal_rtc_init(&my_rtc);

	    // Update the current time and date to the RTC peripheral
	    cyhal_rtc_write(&my_rtc, &new_date_time);

	    // Get the current time and date from the RTC peripheral
	    cyhal_rtc_read(&my_rtc, &current_date_time);

		// strftime() is a C library function which can be used to format date and time
		// into string. It comes under the header file "time.h".
		// (See http://www.cplusplus.com/reference/ctime/strftime/)
		strftime(buffer2, sizeof(buffer2), "%c", &current_date_time);
		// Print the buffer in serial terminal to view the current date and time
		int check = 0;
		printf("\x1b[2J\x1b[;H");
	    printf("****************** "
	           "HAL: I2C Master started on %s"
	           "****************** \r\n\n",buffer2);

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
	    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
    	cyhal_gpio_callback_data_t ButtonPressed = { .callback = ToggleAtEdge };
        cyhal_gpio_register_callback(P0_4, &ButtonPressed);
    	cyhal_gpio_enable_event(P0_4, CYHAL_GPIO_IRQ_FALL, 3 , true);
    	uint8_t buffer[2] = {Led};
    	__enable_irq();


    for (;;)
    {
    	if(cyhal_gpio_read(P0_4) == 0){
        cyhal_rtc_read(&my_rtc, &current_date_time);
    	strftime(buffer2, sizeof(buffer2), "%c", &current_date_time);

    	if(current_date_time.tm_sec != check){
        switch (State) {
                case 1:
                    buffer[1] = GroenLed;
                    printf("Groenlampjeaan \r\n\n");
                    break;
                case 2:
                    buffer[1] = GeelLed;
                    printf("Geellampjeaan \r\n\n");
                    break;
                case 3:
                    buffer[1] = RoodLed;
                    printf("Roodlampjeaan \r\n\n");
                    break;
                default:
                    State = 0; // Reset State to 0 if it goes beyond 4
                    break;
            }
            cyhal_i2c_master_write(&mI2C, I2C_SLAVE_ADDR, buffer, PACKET_SIZE, 0, true);
            cyhal_system_delay_ms(CMD_TO_CMD_DELAY);
            State++;

		}
    }
    	check = current_date_time.tm_sec;
 }
}

/* [] END OF FILE */

