#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define Button P0_4

//#define SpeedSens1 P5_6
//#define SpeedSens2 P6_2
//#define SpeedSens3 P5_5
//#define SpeedSens4 P10_0

#define Motor1_0 P5_0
#define Motor1_1 P5_1
#define Motor2_0 P5_2
#define Motor2_1 P5_3

#define Motor3_0 P5_4
#define Motor3_1 P5_5
#define Motor4_0 P5_6
#define Motor4_1 P5_7


//#define Ultra1_Trig P12_0
//#define Ultra1_Echo P12_3
//#define Ultra2_Trig P13_1
//#define Ultra2_Echo P13_0

#define On 0u
#define Off 1u

#define Speed_PWM 60
#define frequency 40000

cyhal_pwm_t pwm1, pwm2, pwm3, pwm4;
cyhal_pwm_t pwm1b, pwm2b, pwm3b, pwm4b;
bool start;

void motors() {
    cyhal_pwm_init(&pwm1, Motor1_0, NULL);
    cyhal_pwm_init(&pwm1b, Motor1_1, NULL);

    cyhal_pwm_init(&pwm2, Motor2_0, NULL);
    cyhal_pwm_init(&pwm2b, Motor2_1, NULL);

    cyhal_pwm_init(&pwm3, Motor3_0, NULL);
    cyhal_pwm_init(&pwm3b, Motor3_1, NULL);

    cyhal_pwm_init(&pwm4, Motor4_0, NULL);
    cyhal_pwm_init(&pwm4b, Motor4_1, NULL);

    cyhal_pwm_set_duty_cycle(&pwm1, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm2, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm3, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm4, Speed_PWM, frequency);

    cyhal_pwm_set_duty_cycle(&pwm1b, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm2b, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm3b, Speed_PWM, frequency);
    cyhal_pwm_set_duty_cycle(&pwm4b, Speed_PWM, frequency);
}
void left(bool start) {
	if(start){
	cyhal_pwm_stop(&pwm1b);
    cyhal_pwm_start(&pwm2b);
	cyhal_pwm_start(&pwm3b);
	cyhal_pwm_stop(&pwm4b);
	}else {
	cyhal_pwm_stop(&pwm1b);
    cyhal_pwm_stop(&pwm2b);
	cyhal_pwm_stop(&pwm3b);
	cyhal_pwm_stop(&pwm4b);
	}


}
void right(bool start) {
	if(start){
	cyhal_pwm_start(&pwm1);
    cyhal_pwm_start(&pwm2);
	cyhal_pwm_start(&pwm3);
	cyhal_pwm_start(&pwm4);
	}else {
	cyhal_pwm_stop(&pwm1);
    cyhal_pwm_stop(&pwm2);
	cyhal_pwm_stop(&pwm3);
	cyhal_pwm_stop(&pwm4);
	}
}

void motors_on(bool start) {
if(start){
	cyhal_pwm_start(&pwm1);
	cyhal_pwm_start(&pwm2b);
	cyhal_pwm_start(&pwm3b);
	cyhal_pwm_start(&pwm4);
}else{
	cyhal_pwm_stop(&pwm1);
	cyhal_pwm_stop(&pwm2);
	cyhal_pwm_stop(&pwm3);
	cyhal_pwm_stop(&pwm4);
}

}

void motors_off(bool start) {
if (start) {
	cyhal_pwm_stop(&pwm1b);
	cyhal_pwm_stop(&pwm2b);
	cyhal_pwm_stop(&pwm3b);
	cyhal_pwm_stop(&pwm4b);
} else{
	cyhal_pwm_stop(&pwm1b);
	cyhal_pwm_stop(&pwm2b);
	cyhal_pwm_stop(&pwm3b);
	cyhal_pwm_stop(&pwm4b);
}
}

int main(void) {
    cybsp_init();

    // Initialize pins as outputs
    motors();
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    // Initialize button pin as input
    cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, true);



    __enable_irq();

    for (;;) {
        if(cyhal_gpio_read(P0_4) == false) // Button is pressed
        {
        	start = true;
        	left(start);
        }
        else
        {
        	start = true;
        	right(start);
        }
    }
}
