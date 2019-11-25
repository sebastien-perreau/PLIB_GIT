/********************************************************************* 
 *	PORTS module 
 *	Author : Sébastien PERREAU
 *
 *	Revision history	:
 *               14/03/2019      - Initial release
 ********************************************************************/

#include "../PLIB.h"

extern const ports_registers_t * PortsModules[];
const ports_registers_t * PortsModules[] =
{
	(ports_registers_t*)_PORTA_BASE_ADDRESS,
	(ports_registers_t*)_PORTB_BASE_ADDRESS,
	(ports_registers_t*)_PORTC_BASE_ADDRESS,
	(ports_registers_t*)_PORTD_BASE_ADDRESS,
	(ports_registers_t*)_PORTE_BASE_ADDRESS,
	(ports_registers_t*)_PORTF_BASE_ADDRESS,
	(ports_registers_t*)_PORTG_BASE_ADDRESS
};
static ports_event_handler_t ports_event_handler = NULL;

/*******************************************************************************
  Function:
    void ports_change_notice_init(uint32_t cn_pull_up, uint32_t cn_pins_enable, ports_event_handler_t evt_handler)

  Description:
    This routine is used to initialize the change notice feature of the PORT module.
    The CN module allow both, detections on a pin and pin pull-up configuration (on
    CN pin only).

  Parameters:
    cn_pull_up      - The pull-up configuration (CN0_PULLUP_ENABLE | CN1_PULLUP_ENABLE...).
    cn_pins_enable  - The CN pin enable configuration (enable one or more pins to trigger
                    a CN event when a detection (low or high) is made). The CN flag is set
                    when using this feature.
    evt_handler     - The handler (function) to call when an interruption occurs.
  *****************************************************************************/
void ports_change_notice_init(uint32_t cn_pull_up, uint32_t cn_pins_enable, ports_event_handler_t evt_handler)
{
    ports_event_handler = evt_handler;
    irq_init(IRQ_CN, (evt_handler != NULL) ? IRQ_ENABLED : IRQ_DISABLED, irq_change_notice_priority());
    CNCON = ((evt_handler != NULL) && (cn_pins_enable > 0)) ? CN_ON : CN_OFF;
    CNEN = cn_pins_enable;      // CN pins enable
    CNPUE = cn_pull_up;         // CN pins pull-up enable
}

/*******************************************************************************
  Function:
    void ports_reset_all_pins_input()

  Description:
    This routine is used to initialize all PIC32 pins as there default state (INPUT mode
    and PORT state as 0).

  Parameters:
    none
  *****************************************************************************/
void ports_reset_all_pins_input()
{
    ports_registers_t * pPorts;
    uint8_t i;
    for (i = 0 ; i < (sizeof(PortsModules)/sizeof(ports_registers_t)) ; i++)
    {
        pPorts = (ports_registers_t *) PortsModules[i];
        pPorts->TRISSET =  0xffffffff;
        pPorts->LATCLR = 0xffffffff;
    }
}

/*******************************************************************************
  Function:
    void ports_reset_pin_input(_io_t io)

  Description:
    This routine is used to initialize ONE pin as input 
    (tris.pin = input & port.pin = 0).

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
  *****************************************************************************/
void ports_reset_pin_input(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    pPorts->TRISSET =  (uint32_t) (1 << io._indice);
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

/*******************************************************************************
  Function:
    void ports_reset_pin_output(_io_t io)

  Description:
    This routine is used to initialize ONE pin as output 
    (tris.pin = output & port.pin = 0).

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
  *****************************************************************************/
void ports_reset_pin_output(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    pPorts->TRISCLR =  (uint32_t) (1 << io._indice);
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

/*******************************************************************************
  Function:
    bool ports_get_bit(_io_t io)

  Description:
    This routine is used to get a ONE pin's value (1 or 0).

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
 
  Return:
    1 if bit as a high level, 0 if bit as a low level. 
  *****************************************************************************/
bool ports_get_bit(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    return ((pPorts->PORT & (1 << io._indice)) >> io._indice);
}

/*******************************************************************************
  Function:
    void ports_set_bit(_io_t io)

  Description:
    This routine is used to set (1) a ONE pin's bit.

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
  *****************************************************************************/
void ports_set_bit(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    pPorts->LATSET = (uint32_t) (1 << io._indice);
}

/*******************************************************************************
  Function:
    void ports_clr_bit(_io_t io)

  Description:
    This routine is used to clear (0) a ONE pin's bit.

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
  *****************************************************************************/
void ports_clr_bit(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    pPorts->LATCLR = (uint32_t) (1 << io._indice);
}

/*******************************************************************************
  Function:
    void ports_toggle_bit(_io_t io)

  Description:
    This routine is used to toggle (!pin) a ONE pin's bit.

  Parameters:
    io  - The _io_t parameter (PORT & INDICE).
  *****************************************************************************/
void ports_toggle_bit(_io_t io)
{
    ports_registers_t * pPorts = (ports_registers_t *) PortsModules[io._port - 1];
    pPorts->LATINV = (uint32_t) (1 << io._indice);
}

/*******************************************************************************
  Function:
    void ports_interrupt_handler()

  Description:
    This routine is called when an interruption occurs. This interrupt 
    handler calls the user _event_handler (if existing) otherwise do nothing.

  Parameters:
    none
  *****************************************************************************/
void ports_interrupt_handler()
{
    if (ports_event_handler != NULL)
    {
        (*ports_event_handler)();
    }
}
