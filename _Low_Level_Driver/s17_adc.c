/*********************************************************************
*	SPI modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		16/10/2015		- Initial release
*       06/10/2018      - Compatibility PLIB
*                       - No dependencies to xc32 library
*                       - Add comments   
* 
*   Informations:
*   -------------
*   The user can read a data store in a buffer at any time. 
*   Interruptions are disable and acquisitions are made all the time.
*   For example if AN1, AN9 and AN15 are enable then the user can 
*   retrieve the data at any time with the routine ADC10Read(channel).
*********************************************************************/

#include "../PLIB.h"

static event_handler_t adc10_event_handler = NULL;

/*******************************************************************************
 * Function: 
 *      void adc10_init(ADC10_ANALOG_PIN channels, ADC10_VOLTAGE_REF vref)
 * 
 * Description:
 *      This routine is used to initialize the ADC10 module. The pins
 *      used by the module are automatically sets as input & analog.
 * 
 * Parameters:
 *      channels: Indicate all channels used .
 *      vref: Indicate the reference voltage used.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      See. _EXAMPLE_AVERAGE_AND_NTC()
 ******************************************************************************/
void adc10_init(ADC10_ANALOG_PIN channels, ADC10_VOLTAGE_REF vref, event_handler_t evt_handler)
{
    uint8_t i = 0;
    uint8_t numberOfSamplesBetweenInterrupts = 0;
    
    adc10_event_handler = evt_handler;
    irq_init(IRQ_AD1, (evt_handler != NULL) ? IRQ_ENABLED : IRQ_DISABLED, irq_adc10_priority());
    
    for(i = 0 ; i < 16 ; i++)
    {
        if((channels >> i) & 0x0001)
        {
            numberOfSamplesBetweenInterrupts++;
        }
    }
    
    AD1CON1CLR = ADC_MODULE_ON_MASK;
    AD1CSSL = channels;         // Select input channel to scan
    TRISB |= channels;          // Set IO as input pin
    AD1PCFG = ~channels;        // Set IO as analog pin
    AD1CHS = 0;                 // Ignore these bits because ADC_SCAN_ON in AD1CON2.
    AD1CON3 = ADC_CONV_CLK_INTERNAL_RC | ADC_SAMPLE_TIME_15;
    AD1CON2 = vref | ADC_SCAN_ON | (numberOfSamplesBetweenInterrupts << 2);
    AD1CON1 = ADC_MODULE_ON | ADC_FORMAT_INTG16 | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON;
}

/*******************************************************************************
 * Function: 
 *      uint16_t adc10_read(ADC10_ANALOG_PIN channel)
 * 
 * Description:
 *      This routine is used to read the value of a channel.
 * 
 * Parameters:
 *      channel: The channel you want to read its value.
 * 
 * Return:
 *      The channel's value (10 bits 0..1023).
 * 
 * Example:
 *      See. _EXAMPLE_AVERAGE_AND_NTC()
 ******************************************************************************/
uint16_t adc10_read(ADC10_ANALOG_PIN channel)
{
    uint8_t i, j;
    for(i = 0, j = 0 ; i < 16 ; i++)
    {
        if((AD1CSSL >> i) & 0x0001)
        {
            j++;
        }
        if((channel >> i) & 0x0001)
        {
            return (*(&ADC1BUF0+((j-1) * 4)));
        }
    }
    return 0;
}

/*******************************************************************************
 * Function: 
 *      void adc10_interrupt_handler()
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      none
 ******************************************************************************/
void adc10_interrupt_handler()
{
    if (adc10_event_handler != NULL)
    {
        (*adc10_event_handler)();
    }
}
