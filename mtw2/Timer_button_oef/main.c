/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "stdio.h"

#define HIGH 0
#define LOW 1

int main(void)
{
    volatile uint32_t read_val = 0;
    bool input_button = 0;
    bool gamestarted = 0;

    /* Timer object used */
    cyhal_timer_t timer_obj;
    const cyhal_timer_cfg_t timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = 6000,                    /* Timer period set to 6 seconds */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = false,             //One shot right here, als da 'true' was dan was het continuous
        .value = 0                          /* Initial value of counter */
    };

    cyhal_timer_init(&timer_obj, NC, NULL);
    cyhal_timer_configure(&timer_obj, &timer_cfg);

    cyhal_timer_set_frequency(&timer_obj, 1000);

    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_STRONG, LOW);
    cyhal_gpio_init(P13_7, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);

    for (;;)
    {
        input_button = cyhal_gpio_read(P0_4);

        if(input_button == HIGH && gamestarted == 0){
            gamestarted = 1;
            cyhal_timer_start(&timer_obj);
        }

        if(input_button == LOW && gamestarted == 1){
            read_val = cyhal_timer_read(&timer_obj);
            cyhal_timer_stop(&timer_obj);
            gamestarted = 0;
            if(read_val >= 3900 && read_val <= 6000) {
                cyhal_gpio_write(P13_7, false);
                cyhal_system_delay_ms(5000);
                cyhal_gpio_write(P13_7, true);
            }
            cyhal_timer_reset(&timer_obj);
        }

    }
}
