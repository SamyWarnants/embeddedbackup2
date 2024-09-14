/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define trig1 	(P5_0)
#define echo1 	(P5_1)
#define trig2 	(P5_2)
#define echo2 	(P5_3)
#define pwm1 	(P5_4)
#define pwm2 	(P5_5)
#define Speed1 	(P5_6)

#define frequenty	(1000)

/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_timer_t 	timer_obj;
cyhal_pwm_t 	pwm1_obj;
cyhal_pwm_t 	pwm2_obj;
uint32_t 		Pulses;		// Pulses Per Revolution
uint32_t		distance1;
uint32_t		distance2;
bool			bNewVal1;
bool			bNewVal2;
bool			bGo;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void myhal_timer_init();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

void SpeedCount(void* handler_arg, cyhal_gpio_event_t event)
{
	if (Pulses > 0)
		Pulses--;
	else
	{
		bGo = false;
		cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
	}
}

void isr_echo1(void* handler_arg, cyhal_gpio_event_t event)
{
    if (event & CYHAL_GPIO_IRQ_RISE)
    {
        cyhal_timer_start(&timer_obj);
    }
    if (event & CYHAL_GPIO_IRQ_FALL)
    {
        cyhal_timer_stop(&timer_obj);
        // from HC 04 Ultra Sonic module's datasheet, each 58 uS = 1 cm
        distance1 = cyhal_timer_read(&timer_obj) / 58;
        cyhal_timer_reset(&timer_obj);
        bNewVal1 = true;
    }
}

void isr_echo2(void* handler_arg, cyhal_gpio_event_t event)
{
    if (event & CYHAL_GPIO_IRQ_RISE)
    {
        cyhal_timer_start(&timer_obj);
    }
    if (event & CYHAL_GPIO_IRQ_FALL)
    {
        cyhal_timer_stop(&timer_obj);
        // from HC 04 Ultra Sonic module's datasheet, each 58 uS = 1 cm
        distance2 = cyhal_timer_read(&timer_obj) / 58;
        cyhal_timer_reset(&timer_obj);
        bNewVal2 = true;
    }
}

/*******************************************************************************
* Function Name: main
*******************************************************************************/
int main(void)
{
    cybsp_init();

    __enable_irq();

    // Button and LED

    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);

    // Ultrasonic sensor 1

    cyhal_gpio_callback_data_t 	cbecho1;
    cbecho1.callback = isr_echo1;
	cyhal_gpio_init(trig1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(echo1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
	cyhal_gpio_register_callback(echo1, &cbecho1);
    cyhal_gpio_enable_event(echo1, CYHAL_GPIO_IRQ_BOTH, 3, true);

    // Ultrasonic sensor 2

	cyhal_gpio_callback_data_t 	cbecho2;
	cbecho2.callback = isr_echo2;
	cyhal_gpio_init(trig2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
	cyhal_gpio_init(echo2, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
	cyhal_gpio_register_callback(echo2, &cbecho2);
	cyhal_gpio_enable_event(echo2, CYHAL_GPIO_IRQ_BOTH, 3, true);

    myhal_timer_init();

    // Motion control

	cyhal_gpio_callback_data_t cb_Speed1 =
	{
	    .callback     = SpeedCount
	};

	cyhal_gpio_init(Speed1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, true);
	cyhal_gpio_register_callback(Speed1, &cb_Speed1);
	cyhal_gpio_enable_event(Speed1, CYHAL_GPIO_IRQ_RISE, 3, true);

	cyhal_pwm_init(&pwm1_obj, pwm1, NULL);
	cyhal_pwm_init(&pwm2_obj, pwm2, NULL);

	cyhal_pwm_set_duty_cycle(&pwm1_obj, 0, frequenty);
	cyhal_pwm_set_duty_cycle(&pwm2_obj, 0, frequenty);

	cyhal_pwm_start(&pwm1_obj);
	cyhal_pwm_start(&pwm2_obj);

	// Give a objective

	Pulses = 20;
	uint32_t Speed = 100;
	uint32_t SpeedPrev = 0;

	// Wait a bit... and GO!

	cyhal_system_delay_ms(2000);

	for (;;)
    {
    	if (!cyhal_gpio_read(CYBSP_USER_BTN) & !bGo)
		{
			Pulses = 400;
			bGo = true;
			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
		}

		if ( Speed != SpeedPrev )
		{
			cyhal_pwm_set_duty_cycle(&pwm1_obj, Speed, frequenty);
			cyhal_pwm_set_duty_cycle(&pwm2_obj, Speed, frequenty);
			SpeedPrev = Speed;
		}

    	if ( (Pulses == 0) || (distance1 < 10) )
			Speed = 0;
    	else
    		Speed = 90;

    	if (!cyhal_gpio_read(CYBSP_USER_BTN))
        {
    		cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);

    		// send a 10µs trigger for sensor 1
			cyhal_gpio_write(trig1, true);
			cyhal_system_delay_us(10);
			cyhal_gpio_write(trig1, false);

			while(!bNewVal1);

			// send a 10µs trigger for sensor 1
			cyhal_gpio_write(trig2, true);
			cyhal_system_delay_us(10);
			cyhal_gpio_write(trig2, false);

			while(!bNewVal2);

			cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);

			if ( (distance2-distance1 > 1) || (distance1-distance2 > 1) )
			{
				for(int i = 0; i < 4; i++)
				{
					cyhal_gpio_toggle(CYBSP_USER_LED);
					cyhal_system_delay_ms(250);
				}
			}
			bNewVal2 = bNewVal1 = false;
        }
    }
}

/*******************************************************************************
* Initialization function for the timer
*******************************************************************************/

void myhal_timer_init()
{
    const cyhal_timer_cfg_t timer_cfg = {
        .compare_value = 0,                  // Timer compare value, not used
        .period = 23200,                     // Max 400 cm => 400 x 58 = 23200 µs
        .direction = CYHAL_TIMER_DIR_UP,     // Timer counts up
        .is_compare = false,                 // Don't use compare mode
        .is_continuous = false,              // Do not run timer indefinitely
        .value = 0                          // Initial value of the counter
    };

    cyhal_timer_init(&timer_obj, NC, NULL);
    cyhal_timer_configure(&timer_obj, &timer_cfg);
    cyhal_timer_set_frequency(&timer_obj, 1000000);
}

/* [] END OF FILE */
