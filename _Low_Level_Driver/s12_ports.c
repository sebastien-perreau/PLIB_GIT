
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
