/*********************************************************************
*   Software PWM driver
*   Author : Sébastien PERREAU
*
*   Revision history    :
*               12/03/2019              - Initial release
*
*   Timings (200 hertz with resolution = 1):
*   ---------------------------------------
*   4 S-PWM : 8,38 % busy (occupation time in the interrupt routine by the periodicity)
*   7 S-PWM : 13,59 % busy
*   14 S-PWM : 22,48 % busy
*   20 S-PWM : 30,14 % busy
*   30 S-PWM : 42,92 % busy
*********************************************************************/

#include "../PLIB.h"

static SOFTWAPRE_PWM_PARAMS  * p_software_pwm = NULL;
const PORTS_REGISTERS * p_ports_registers_array[] =
{
	(PORTS_REGISTERS*)_PORTA_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTB_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTC_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTD_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTE_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTF_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTG_BASE_ADDRESS
};

static void software_pwm_event_handler(uint8_t id)
{    
    static uint8_t i;
    
    for (i = 0 ; i < p_software_pwm->number_of_pwm_used ; i++)
    {
        PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) p_ports_registers_array[p_software_pwm->io[i]._port - 1];
        
        if (p_software_pwm->pwm[i] > p_software_pwm->counter)
        {
            pPorts->LATSET = (uint32_t) (1 << p_software_pwm->io[i]._indice);
        }
        else
        {
            pPorts->LATCLR = (uint32_t) (1 << p_software_pwm->io[i]._indice);
        }
    }
    
    p_software_pwm->counter += p_software_pwm->resolution;
}

void software_pwm_init(SOFTWAPRE_PWM_PARAMS *var)
{
    static uint8_t i;
    
    p_software_pwm = var;
    
    for (i = 0 ; i < var->number_of_pwm_used ; i++)
    {
        ports_reset_pin_output(var->io[i]);
    }
    
    timer_init_2345_hz(var->timer_module, software_pwm_event_handler, TMR_ON | TMR_SOURCE_INT | TMR_IDLE_CON | TMR_GATE_OFF, var->frequency_hz * 255 / var->resolution);
}
