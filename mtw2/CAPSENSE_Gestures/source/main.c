
/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cybsp.h"
#include "cyhal.h"
#include "cycfg.h"
#include "cycfg_capsense.h"
#include "led.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CAPSENSE_INTR_PRIORITY      (7u)
#define EZI2C_INTR_PRIORITY         (6u) /* EZI2C interrupt priority must be
                                          * higher than CapSense interrupt */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static uint32_t initialize_capsense(void);
static void process_touch( uint32_t gestureStatus);
static void initialize_capsense_tuner(void);
static void capsense_isr(void);
static void capsense_callback();
void handle_error(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_stc_scb_ezi2c_context_t ezi2c_context;
cyhal_ezi2c_t sEzI2C;
cyhal_ezi2c_slave_cfg_t sEzI2C_sub_cfg;
cyhal_ezi2c_cfg_t sEzI2C_cfg;
volatile bool capsense_scan_complete = false;

uint32_t gestureStatus = 0;

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}



int main(void)
{
    cy_rslt_t result;


    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();
    
    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("**********************************************************\r\n");
    printf("PSoC 6 MCU: CAPSENSE buttons and slider\r\n");
    printf("**********************************************************\r\n");


    initialize_led();
    initialize_capsense_tuner();
    result = initialize_capsense();

    if (CYRET_SUCCESS != result)
    {
        /* Halt the CPU if CapSense initialization failed */
        CY_ASSERT(0);
    }

    /* Initiate first scan */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    for (;;)
    {
        if (capsense_scan_complete)
        {

        	/* Increments the timestamp register for the predefined timestamp interval */
        	Cy_CapSense_IncrementGestureTimestamp(&cy_capsense_context);

        	/* Process all widgets */
        	Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

        	/* Process gesture for the widget */
        	gestureStatus = Cy_CapSense_DecodeWidgetGestures(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);


            /* Process touch input */
            process_touch(gestureStatus);

            /* Establishes synchronized operation between the CapSense
             * middleware and the CapSense Tuner tool.
             */
            Cy_CapSense_RunTuner(&cy_capsense_context);

            /* Initiate next scan */
            Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

            capsense_scan_complete = false;
        }

    }
    
}

/*******************************************************************************
* Function Name: process_touch
********************************************************************************
* Summary:
*  Gets the details of touch position detected, processes the touch input
*  and updates the LED status.
*
*******************************************************************************/
static void process_touch(uint32_t gestureStatus)
{

	uint16_t direction;
	static led_data_t led_data = {LED_ON, LED_MAX_BRIGHTNESS};

	/* If Two-finger Scroll gesture is detected */
	if (CY_CAPSENSE_GESTURE_NO_GESTURE != (gestureStatus & CY_CAPSENSE_GESTURE_ONE_FNGR_FLICK_MASK ))
	{
	    direction = (gestureStatus >> CY_CAPSENSE_GESTURE_DIRECTION_OFFSET) &\
	                      CY_CAPSENSE_GESTURE_DIRECTION_MASK_ONE_FLICK;

	    switch(direction >> CY_CAPSENSE_GESTURE_DIRECTION_OFFSET_ONE_FLICK)
	    {
	        case CY_CAPSENSE_GESTURE_DIRECTION_LEFT:
	            led_data.state = LED_OFF;
	            break;
	        case CY_CAPSENSE_GESTURE_DIRECTION_RIGHT:
	            led_data.state = LED_ON;
	            break;
	        default:
	            break;
	    }
	}

	/* Single click gesture is detected */

	if((CY_CAPSENSE_GESTURE_ONE_FNGR_SINGLE_CLICK_MASK & gestureStatus) != CY_CAPSENSE_GESTURE_NO_GESTURE)
	{
	    if (led_data.state == LED_ON)
	        led_data.state = LED_OFF;
	    else
	        led_data.state = LED_ON;
	}

	update_led_state(&led_data);


}

/*******************************************************************************
* Function Name: initialize_capsense
********************************************************************************
* Summary:
*  This function initializes the CapSense and configure the CapSense
*  interrupt.
*
*******************************************************************************/
static uint32_t initialize_capsense(void)
{
    uint32_t status = CYRET_SUCCESS;

    /* CapSense interrupt configuration parameters */
    static const cy_stc_sysint_t capSense_intr_config =
    {
        .intrSrc = csd_interrupt_IRQn,
        .intrPriority = CAPSENSE_INTR_PRIORITY,
    };

    /* Capture the CSD HW block and initialize it to the default state. */
    status = Cy_CapSense_Init(&cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return status;
    }

    /* Initialize CapSense interrupt */
    cyhal_system_set_isr(csd_interrupt_IRQn, csd_interrupt_IRQn, CAPSENSE_INTR_PRIORITY, &capsense_isr);
    NVIC_ClearPendingIRQ(capSense_intr_config.intrSrc);
    NVIC_EnableIRQ(capSense_intr_config.intrSrc);

    /* Initialize the CapSense firmware modules. */
    status = Cy_CapSense_Enable(&cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return status;
    }

    /* Assign a callback function to indicate end of CapSense scan. */
    status = Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E,
            capsense_callback, &cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return status;
    }

    return status;
}

/*******************************************************************************
* Function Name: capsense_isr
********************************************************************************
* Summary:
*  Wrapper function for handling interrupts from CapSense block.
*
*******************************************************************************/
static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}

/*******************************************************************************
* Function Name: capsense_callback()
********************************************************************************
* Summary:
*  This function sets a flag to indicate end of a CapSense scan.
*
* Parameters:
*  cy_stc_active_scan_sns_t* : pointer to active sensor details.
*
*******************************************************************************/
void capsense_callback(cy_stc_active_scan_sns_t * ptrActiveScan)
{
    capsense_scan_complete = true;
}


/*******************************************************************************
* Function Name: initialize_capsense_tuner
********************************************************************************
* Summary:
*  Initializes interface between Tuner GUI and PSoC 6 MCU.
*
*******************************************************************************/
static void initialize_capsense_tuner(void)
{
    cy_rslt_t result;

    /* Configure Capsense Tuner as EzI2C Slave */
    sEzI2C_sub_cfg.buf = (uint8 *)&cy_capsense_tuner;
    sEzI2C_sub_cfg.buf_rw_boundary = sizeof(cy_capsense_tuner);
    sEzI2C_sub_cfg.buf_size = sizeof(cy_capsense_tuner);
    sEzI2C_sub_cfg.slave_address = 8U;

    sEzI2C_cfg.data_rate = CYHAL_EZI2C_DATA_RATE_400KHZ;
    sEzI2C_cfg.enable_wake_from_sleep = false;
    sEzI2C_cfg.slave1_cfg = sEzI2C_sub_cfg;
    sEzI2C_cfg.sub_address_size = CYHAL_EZI2C_SUB_ADDR16_BITS;
    sEzI2C_cfg.two_addresses = false;
    
    result = cyhal_ezi2c_init(&sEzI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL, &sEzI2C_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

}

/* [] END OF FILE */
