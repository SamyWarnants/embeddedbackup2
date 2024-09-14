#include "cyhal.h"
#include "cybsp.h"

int main(void)
{



    cybsp_init();

    __enable_irq();

    cyhal_gpio_init(P13_7, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);

    for(;;)
    {
        if(cyhal_gpio_read(P0_4) == 0){
            cyhal_gpio_write(P13_7, true);
            cyhal_system_delay_ms(100);

            cyhal_gpio_write(P13_7, false);
            cyhal_system_delay_ms(100);
        }
        else{
        cyhal_gpio_write(P13_7, true);
        }
    }
}


