
#include "../PLIB.h"

extern const PORTS_REGISTERS * PortsModules[];
const PORTS_REGISTERS * PortsModules[] =
{
	(PORTS_REGISTERS*)_PORTA_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTB_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTC_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTD_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTE_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTF_BASE_ADDRESS,
	(PORTS_REGISTERS*)_PORTG_BASE_ADDRESS
};
static event_handler_t ports_event_handler = NULL;

void ports_change_notice_init(uint32_t cn_pull_up, uint32_t cn_pins_enable, event_handler_t evt_handler)
{
    ports_event_handler = evt_handler;
    irq_init(IRQ_CN, (evt_handler != NULL) ? IRQ_ENABLED : IRQ_DISABLED, irq_change_notice_priority());
    CNCON = ((evt_handler != NULL) && (cn_pins_enable > 0)) ? CN_ON : CN_OFF;
    CNEN = cn_pins_enable;      // CN pins enable
    CNPUE = cn_pull_up;         // CN pins pull-up enable
}

void ports_reset_all_pins_input()
{
    PORTS_REGISTERS * pPorts;
    uint8_t i;
    for (i = 0 ; i < (sizeof(PortsModules)/sizeof(PORTS_REGISTERS)) ; i++)
    {
        pPorts = (PORTS_REGISTERS *) PortsModules[i];
        pPorts->TRISSET =  0xffffffff;
        pPorts->LATCLR = 0xffffffff;
    }
}

void ports_reset_pin_input(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    pPorts->TRISSET =  (uint32_t) (1 << io._indice);
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

void ports_reset_pin_output(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    pPorts->TRISCLR =  (uint32_t) (1 << io._indice);
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

bool ports_get_bit(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    return ((pPorts->PORT & (1 << io._indice)) >> io._indice);
}

void ports_set_bit(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    pPorts->LATSET = (uint32_t) (1 << io._indice);
}

void ports_clr_bit(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

void ports_toggle_bit(_IO io)
{
    PORTS_REGISTERS * pPorts = (PORTS_REGISTERS *) PortsModules[io._port - 1];
    pPorts->LATINV = (uint32_t) (1 << io._indice);
}


/*******************************************************************************
 * Function: 
 *      void ports_interrupt_handler()
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      none
 ******************************************************************************/
void ports_interrupt_handler()
{
    if (ports_event_handler != NULL)
    {
        (*ports_event_handler)();
    }
}
