#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#define ADC_VPLUS1 (P10_0)


int32_t adc_out;
cyhal_adc_t         adc_obj;
cyhal_adc_channel_t adc_chan_0_obj;
cyhal_pwm_t pwm_obj;

static void adc_continuous_event_handler(void* arg, cyhal_adc_event_t event)
{

    if(0u != (event & CYHAL_ADC_EOS))
    {       int32_t adc_out;
    		int pwm_percent;
            adc_out= cyhal_adc_read(&adc_chan_0_obj);
            cyhal_system_delay_ms(500);
            if (adc_out >= 0)
            {
            	pwm_percent = ((adc_out*100)/(2047*2)) + 50 ;
            	cyhal_pwm_set_duty_cycle(&pwm_obj,pwm_percent, 100);
            	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
            }
            else
            {
            	pwm_percent = 50 - (adc_out*100)/(-2048*2);
            	cyhal_pwm_set_duty_cycle(&pwm_obj,pwm_percent, 100);
            	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
            }

    }
}

int main(void)
{




    cyhal_adc_config_t config =
    {
            .continuous_scanning = true
    };

    const cyhal_adc_channel_config_t channel_config =
        {
                .enable_averaging = false,
                .min_acquisition_ns = 220,
                .enabled = true
        };

    cybsp_init();

    __enable_irq();

    cyhal_pwm_init(&pwm_obj, P13_7, NULL);
    /* Set a duty cycle of 50% and frequency of 1Hz */
    cyhal_pwm_set_duty_cycle(&pwm_obj, 0, 1000);
    /* Start the PWM output */
    cyhal_pwm_start(&pwm_obj);


    cyhal_adc_init(&adc_obj, ADC_VPLUS1, NULL);
    cyhal_adc_configure(&adc_obj, &config);
    cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, ADC_VPLUS1, CYHAL_ADC_VNEG,    &channel_config);

    cyhal_adc_register_callback(&adc_obj, &adc_continuous_event_handler, &adc_obj);
    cyhal_adc_enable_event(&adc_obj, CYHAL_ADC_EOS, CYHAL_ISR_PRIORITY_DEFAULT, true);
    /* Call the ADC configure function to begin continuous scanning */
    cyhal_adc_configure(&adc_obj, &config);
    for (;;)
    {
    }
}
