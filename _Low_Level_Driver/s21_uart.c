/*********************************************************************
*	UART modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		05/11/2018		- Initial release
*********************************************************************/

#include "../PLIB.h"

extern const UART_REGISTERS * UartModules[];
const UART_REGISTERS * UartModules[] =
{
	(UART_REGISTERS*)_UART1_BASE_ADDRESS,
	(UART_REGISTERS*)_UART2_BASE_ADDRESS,
	(UART_REGISTERS*)_UART3_BASE_ADDRESS,
	(UART_REGISTERS*)_UART4_BASE_ADDRESS,
	(UART_REGISTERS*)_UART5_BASE_ADDRESS,
	(UART_REGISTERS*)_UART6_BASE_ADDRESS
};
static uint32_t real_baudrate_tab[UART_NUMBER_OF_MODULES] = {0};
static uart_event_handler_t uart_event_handler[UART_NUMBER_OF_MODULES] = {NULL};

/*******************************************************************************
 * Function: 
 *      void uart_init(     UART_MODULE id, 
 *                  uart_event_handler_t evt_handler,
 *                  IRQ_EVENT_TYPE event_type_enable,
 *                  UART_BAUDRATE baudrate, 
 *                  UART_ENABLE_MODE enable_mode,
 *                  UART_CONFIG_MODE config_mode,
 *                  UART_LINE_CONTROL_MODE control_mode,
 *                  UART_FIFO_MODE fifo_mode,
 *                  UART_ADDRESS_DETECTION address_detection)
 * 
 * Description:
 *      This routine is used to initialize an uart module.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      evt_handler: The handler (function) to call when an interruption occurs.
 *      event_type_enable: The event(s) you want to enable by interruption (See. 
 *      IRQ_EVENT_TYPE). 
 *      baudrate: The desire baudrate in bit per second.
 *      enable_mode: Params follow the EXP_UART_ENABLE_MODE enumeration.
 *      config_mode: Params follow the EXP_UART_CONFIG_MODE enumeration.
 *      control_mode: Params follow the EXP_UART_LINE_CONTROL_MODE enumeration.
 *      fifo_mode: Params follow the EXP_UART_FIFO_MODE enumeration.
 *      address_detection: Params follow the EXP_UART_ADDRESS_DETECTION enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/

void uart_init(     UART_MODULE id, 
                    uart_event_handler_t evt_handler,
                    IRQ_EVENT_TYPE event_type_enable,
                    UART_BAUDRATE baudrate, 
                    UART_ENABLE_MODE enable_mode,
                    UART_CONFIG_MODE config_mode,
                    UART_LINE_CONTROL_MODE control_mode,
                    UART_FIFO_MODE fifo_mode,
                    UART_ADDRESS_DETECTION address_detection)
{

    uart_event_handler[id] = evt_handler;
    irq_init(IRQ_U1E + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_UART_ERROR) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_uart_priority(id));
    irq_init(IRQ_U1RX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_UART_RX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_uart_priority(id));
    irq_init(IRQ_U1TX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_UART_TX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_uart_priority(id));
 
    uart_enable(id, UART_DISABLE);
    uart_set_params(id, config_mode);
    uart_set_line_control(id, control_mode);
    uart_set_baudrate(id, baudrate);
    uart_set_fifo(id, fifo_mode);
    uart_set_adress_detection(id, (uint8_t) address_detection, address_detection & UART_ADDRESS_DETECTION_MASK);
    uart_enable(id, enable_mode);
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_enable(EXP_UART_MODULE id, EXP_UART_ENABLE_MODE enable_mode)
 * 
 * Description:
 *      This routine is used to enable the uart module and the RX/TX pins.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      enable_mode: Params follow the EXP_UART_ENABLE_MODE enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_enable(UART_MODULE id, UART_ENABLE_MODE enable_mode)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    if (enable_mode & UART_ENABLE)
    {
        p_uart->STASET = (enable_mode & (UART_ENABLE_RX_PIN | UART_ENABLE_TX_PIN));
        p_uart->MODESET = 0x00008000;
    }
    else
    {
        p_uart->STACLR = UART_ENABLE_RX_PIN | UART_ENABLE_TX_PIN;
        p_uart->MODECLR = 0x00008000;
    }
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_set_params(EXP_UART_MODULE id, EXP_UART_CONFIG_MODE config_mode)
 * 
 * Description:
 *      This routine is used to configure the MODE register.
 *      This register does the following:
 *          - Enables or disables the UART module
 *          - Enables or disables the IrDA encoder and decoder
 *          - Enables or disables the WAKE, ABAUD and Loopback features
 *          - Enables or disables the UxRTS and UxCTS pins
 *          - Configures the UxRTS pin for the desired mode of operation
 *          - Configures the polarity of the UxRX pin
 *          - Selects the type of baud rate
 *          - Selects the number of data bits, parity and stop bits
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      config_mode: Params follow the EXP_UART_CONFIG_MODE enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_set_params(UART_MODULE id, UART_CONFIG_MODE config_mode)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    p_uart->MODECLR = (UART_CONFIG_MASK & 0x0000ffff);
    p_uart->STACLR = ((UART_CONFIG_MASK >> 16) & 0x0000ffff);
    p_uart->MODESET = (config_mode & 0x0000ffff);
    p_uart->STASET = ((config_mode >> 16) & 0x0000ffff);
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_set_line_control(EXP_UART_MODULE id, EXP_UART_LINE_CONTROL_MODE control_mode)
 * 
 * Description:
 *      This routine is used to configure the data format .
 *      This register does the following:
 *          - Enables or disables the UART module
 *          - Enables or disables the IrDA encoder and decoder
 *          - Enables or disables the WAKE, ABAUD and Loopback features
 *          - Enables or disables the UxRTS and UxCTS pins
 *          - Configures the UxRTS pin for the desired mode of operation
 *          - Configures the polarity of the UxRX pin
 *          - Selects the type of baud rate
 *          - Selects the number of data bits, parity and stop bits
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      control_mode: Params follow the EXP_UART_LINE_CONTROL_MODE enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_set_line_control(UART_MODULE id, UART_LINE_CONTROL_MODE control_mode)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    p_uart->MODECLR = UART_LINE_CONTROL_MASK;
    p_uart->MODESET = (control_mode & UART_LINE_CONTROL_MASK);
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_set_fifo(EXP_UART_MODULE id, EXP_UART_FIFO_MODE fifo_mode)
 * 
 * Description:
 *      This routine is used to set the type of interruption for Rx et Tx.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      fifo_mode: Params follow the EXP_UART_FIFO_MODE enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_set_fifo(UART_MODULE id, UART_FIFO_MODE fifo_mode)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    p_uart->STACLR = UART_FIFO_MASK;
    p_uart->STASET = (fifo_mode & UART_FIFO_MASK);
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_set_adress_detection(EXP_UART_MODULE id, uint8_t address, EXP_UART_ADDRESS_DETECTION address_detection)
 * 
 * Description:
 *      This routine is used to set the address when detection is enable.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      address: Set the address you want to detect.
 *      address_detection: Enable or disable the address detection.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_set_adress_detection(UART_MODULE id, uint8_t address, UART_ADDRESS_DETECTION address_detection)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    p_uart->STACLR = UART_ADDRESS_DETECTION_MASK;
    p_uart->STASET = (((uint32_t) address) << 16) | address_detection;
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_set_baudrate(EXP_UART_MODULE id, uint32_t baudrate)
 * 
 * Description:
 *      This routine is used to set the baudrate for the UART module.
 *      Note:   The desired baudrate may not be the one passed as parameter. It
 *              can be an approximation, that is why the "real baudrate" is 
 *              store in a tab.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      baudrate: The desire baudrate in bit per second.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_set_baudrate(UART_MODULE id, uint32_t baudrate)
{
	UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    uint32_t v_baudrate;
    uint32_t v_source_clock = (PERIPHERAL_FREQ >> 1);

    if(!(p_uart->MODE & _U1MODE_BRGH_MASK))
    {
        v_source_clock >>= 2;
    }

    v_baudrate = v_source_clock / baudrate;
    v_baudrate++;
    v_baudrate >>= 1;
    v_baudrate--;

    p_uart->BRG = v_baudrate & 0x0000FFFF;

    real_baudrate_tab[id] = (v_source_clock >> 1) / ( v_baudrate + 1 );
}

/*******************************************************************************
 * Function: 
 *      uint32_t exp_uart_get_baudrate(EXP_UART_MODULE id)
 * 
 * Description:
 *      This routine is used to get the real baudrate for the selected UART module.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 * 
 * Return:
 *      The real baudrate in bit per second.
 ******************************************************************************/
uint32_t uart_get_baudrate(UART_MODULE id)
{
    return real_baudrate_tab[id];
}

/*******************************************************************************
 * Function: 
 *      bool exp_uart_transmission_has_completed(EXP_UART_MODULE id)
 * 
 * Description:
 *      This routine is used to get the status of a transmit data byte for a 
 *      selected UART module.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 * 
 * Return:
 *      The status of the transmission (true = completed / false = on going).
 ******************************************************************************/
bool uart_transmission_has_completed(UART_MODULE id)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    return (_U1STA_TRMT_MASK == (p_uart->STA & _U1STA_TRMT_MASK));
}

/*******************************************************************************
 * Function: 
 *      bool exp_uart_is_tx_ready(EXP_UART_MODULE id)
 * 
 * Description:
 *      This routine is used to get the status of a transmit buffer for a 
 *      selected UART module.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 * 
 * Return:
 *      The status of the tx buffer (true = ready / false = not ready).
 ******************************************************************************/
bool uart_is_tx_ready(UART_MODULE id)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    return (bool)(!(p_uart->STA & _U1STA_UTXBF_MASK));
}

/*******************************************************************************
 * Function: 
 *      bool exp_uart_is_rx_data_available(EXP_UART_MODULE id)
 * 
 * Description:
 *      This routine is used to get the status of a receive buffer for a 
 *      selected UART module.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 * 
 * Return:
 *      The status of the rx buffer (true = data available / false = data not available.
 ******************************************************************************/
bool uart_is_rx_data_available(UART_MODULE id)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    return (bool)(_U1STA_URXDA_MASK == (p_uart->STA & _U1STA_URXDA_MASK));
}

/*******************************************************************************
 * Function: 
 *      void exp_uart_send_break(EXP_UART_MODULE id)
 * 
 * Description:
 *      This routine is used to send a break characters.
 *      A Break character transmit consists of a Start bit, followed by twelve 
 *      bits of ?0?, and a Stop bit. A Frame Break character is sent whenever 
 *      the UART module is enabled, and the UTXBRK (UxSTA<11>) and UTXEN 
 *      (UxSTA<10>) bits are set while the UxTXREG register is loaded with data. 
 *      A dummy write to the UxTXREG register is necessary to initiate the Break 
 *      character transmission. The data value written to the UxTXREG register 
 *      for the Break character is ignored. 
 * 
 * Parameters:
 *      id: The UART module you want to use.
 * 
 * Return:
 *      The status of the break transmission (0: done / 1: not sent)
 ******************************************************************************/
bool uart_send_break(UART_MODULE id)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    if ((bool)(!(p_uart->STA & _U1STA_UTXBF_MASK)))
    {
        p_uart->STASET = _U1STA_UTXBRK_MASK;
        p_uart->TX = 0;
        return 0;
    }
    return 1;
}

/*******************************************************************************
 * Function: 
 *      bool exp_uart_send_data(EXP_UART_MODULE id, uint16_t data)
 * 
 * Description:
 *      This routine is used to send a data (8 bits or 9 bits).
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      data: The data (8 or 9 bits) to send.
 * 
 * Return:
 *      The status of the data transmission (0: done / 1: not sent)
 ******************************************************************************/
bool uart_send_data(UART_MODULE id, uint16_t data)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    if ((bool)(!(p_uart->STA & _U1STA_UTXBF_MASK)))
    {
        p_uart->TX = data;
        return 0;
    }
    return 1;
}

/*******************************************************************************
 * Function: 
 *      bool exp_uart_get_data(EXP_UART_MODULE id, uint16_t *data)
 * 
 * Description:
 *      This routine is used to get a data (8 bits or 9 bits).
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      p_data: A pointer to store the data receive.
 * 
 * Return:
 *      The status of the data reception (0: done / 1: not receive)
 ******************************************************************************/
bool uart_get_data(UART_MODULE id, uint16_t *p_data)
{
    UART_REGISTERS * p_uart = (UART_REGISTERS *) UartModules[id];
    if ((bool)(_U1STA_URXDA_MASK == (p_uart->STA & _U1STA_URXDA_MASK)))
    {
        *p_data = (uint16_t) p_uart->RX;
        return 0;
    }
    return 1;
}

/*******************************************************************************
 * Function: 
 *      void uart_interrupt_handler(TIMER_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      id: The UART module you want to use.
 *      evt_type: The type of event (RX, TX, ERROR...). See IRQ_EVENT_TYPE.
 *      data: The data (in case of a reception) read in the interruption.
 * 
 * Return:
 *      none
 ******************************************************************************/
void uart_interrupt_handler(UART_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    if (uart_event_handler[id] != NULL)
    {
        (*uart_event_handler[id])(id, evt_type, data);
    }
}
