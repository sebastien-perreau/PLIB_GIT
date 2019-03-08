/*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		19/10/2018		- Initial release
* 
*   Description:
*   ------------ 
 * TO DO
 * STOP to print if overtaken buffer
 * Compatible with UART & DMA perso
*********************************************************************/

#include "../PLIB.h"

static UART_MODULE module_id;
static bool dma_tx_in_progress = false;
const uint8_t uart_tx_irq[] = 
{
    _UART1_TX_IRQ,
    _UART2_TX_IRQ,
    _UART3_TX_IRQ,
    _UART4_TX_IRQ,
    _UART5_TX_IRQ,
    _UART6_TX_IRQ
};
const void *p_tx_reg[] = 
{
    (void*)&U1TXREG,
    (void*)&U2TXREG,
    (void*)&U3TXREG,
    (void*)&U4TXREG,
    (void*)&U5TXREG,
    (void*)&U6TXREG
};

void log_init(UART_MODULE id, uint32_t data_rate)
{
    module_id = id;
    
    DmaChnOpen(DMA_CHANNEL6, DMA_CHN_PRI0, DMA_OPEN_MATCH);
    DmaChnSetEvEnableFlags(DMA_CHANNEL6, DMA_EV_BLOCK_DONE);
    DmaChnSetEventControl(DMA_CHANNEL6, DMA_EV_START_IRQ(uart_tx_irq[module_id]));

    uart_init(id, NULL, IRQ_NONE, data_rate, UART_STD_PARAMS);
}

static uint16_t _transform_integer_to_string(char *p_buffer, uint16_t index_p_buffer, uint32_t value, LOG_BASE_t _base, uint8_t number_of_char)
{
    static char dictionary_char[]= "0123456789ABCDEF";
	static char buffer[20] = {0}; 
	char *ptr = &buffer[19];
    uint8_t _number_of_char = number_of_char;
    
    if (_base == BASE_2)
    {
        p_buffer[index_p_buffer++] = 'b';
    }
    else if (_base == BASE_16)
    {
        p_buffer[index_p_buffer++] = '0';
        p_buffer[index_p_buffer++] = 'x';
    }
    
    if (number_of_char == 0)    
    {
        do 
        { 
            *--ptr = dictionary_char[value%_base]; 
            value /= _base; 
        }
        while (value != 0); 
    }
    else
    {
        do 
        { 
            *--ptr = dictionary_char[value%_base]; 
            value /= _base; 
        }
        while (--_number_of_char != 0); 
    }
    
    do
    {
        p_buffer[index_p_buffer++] = *ptr++;
    }
    while (*ptr != '\0');
    
    return index_p_buffer;
}

static uint16_t _get_header_to_string(char *p_buffer, uint16_t index_buffer, LOG_LEVEL_t level)
{
    uint8_t i = 0;
    uint64_t time           = mGetTick();
    uint64_t time_us        = (time / TICK_1US);
    uint64_t time_ms        = (time / TICK_1MS);
    uint64_t time_s         = ((time / TICK_1S) % 60);
    uint64_t time_m         = ((time / TICK_1S / 60) % 60);
    uint64_t time_h         = ((time / TICK_1S / 3600) % 24);
    uint64_t time_d         = (time / TICK_1S / 86400);
    
    if (level == LEVEL_0)
    {
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_d, BASE_10, 0);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_h, BASE_10, 2);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_m, BASE_10, 2);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_s, BASE_10, 2);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_ms, BASE_10, 3);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_us, BASE_10, 3);
        p_buffer[index_buffer++] = ':';
        p_buffer[index_buffer++] = ' ';
    }
    else if (level == LEVEL_1)
    {
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_ms, BASE_10, 0);
        p_buffer[index_buffer++] = ':';
        index_buffer = _transform_integer_to_string(p_buffer, index_buffer, time_us, BASE_10, 3);
        p_buffer[index_buffer++] = ':';
        p_buffer[index_buffer++] = ' ';
    }
    
    return index_buffer;
}

void log_wait_end_of_transmission()
{
    while (dma_tx_in_progress)
    {
        if (irq_get_flag(IRQ_DMA6))
        {
            irq_clr_flag(IRQ_DMA6);
            dma_tx_in_progress = false;
        }
    }
}

void log_frontend(const char *p_message, LOG_LEVEL_t level, const uint32_t *p_args, uint8_t nargs)
{
    char *p_str = (char *) p_message;
    char buffer[16000] = {0};
    uint16_t index_buffer = 0;
    uint8_t index_args = 0;
    uint8_t _number_of_char_to_print = 0;
    
    if (level != LEVEL_2)
    {
        index_buffer = _get_header_to_string(buffer, index_buffer, level);
    }
    
    while (*p_str != '\0')
    {
        if (*p_str != '%')
        {
            buffer[index_buffer++] = *p_str;
        }
        else
        {
            _number_of_char_to_print = 0;
            p_str++;
            if ((*p_str >= '0') && (*p_str <= '9'))
            {
                _number_of_char_to_print = (*p_str - '0');
                p_str++;
                if ((*p_str >= '0') && (*p_str <= '9'))
                {
                    _number_of_char_to_print *= 10;
                    _number_of_char_to_print += (*p_str - '0');
                    p_str++;
                }
            }
            
            switch (*p_str)
            {           
                case 'c':
                    if (index_args < nargs)
                    {
                        buffer[index_buffer++] = p_args[index_args];
                        index_args++;
                    }
                    break;
                    
                case 's':
                    if (index_args < nargs)
                    {
                        char *str = (char *) p_args[index_args];
                        do
                        {
                            buffer[index_buffer++] = *str++;
                        }
                        while (*str != '\0');
                        index_args++;
                    }
                    break;
                    
                case 'b':
                    if (index_args < nargs)
                    {
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, p_args[index_args], BASE_2, _number_of_char_to_print);
                        index_args++;
                    }
                    break;
                    
                case 'o':
                    if (index_args < nargs)
                    {
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, p_args[index_args], BASE_8, _number_of_char_to_print);
                        index_args++;
                    }
                    break;
                    
                case 'd':
                    if (index_args < nargs)
                    {
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, p_args[index_args], BASE_10, _number_of_char_to_print);
                        index_args++;
                    }
                    break;
                    
                case 'x':
                    if (index_args < nargs)
                    {
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, p_args[index_args], BASE_16, _number_of_char_to_print);
                        index_args++;
                    }
                    break;
                    
                case 'f':
                    if (index_args < nargs)
                    {
                        float *p_val = (float *) (uint32_t *) p_args[index_args];
                        float v = *p_val;
                        uint32_t integer = fu_get_integer_value(v);
                        uint32_t decimal = fu_get_decimal_value(v, (_number_of_char_to_print == 0) ? 3 : _number_of_char_to_print);
                        
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, integer, BASE_10, 0);
                        buffer[index_buffer++] = ',';
                        index_buffer = _transform_integer_to_string(buffer, index_buffer, decimal, BASE_10, (_number_of_char_to_print == 0) ? 3 : _number_of_char_to_print);
                        index_args++;
                    }
                    break;
                    
                default:
                    if (index_args < nargs)
                    {
                        index_args++;
                    }
                    break;
            }
        }
        p_str++;
    }
    
    buffer[index_buffer++] = '\n'; 
    buffer[index_buffer++] = '\r';
    DmaChnSetTxfer(DMA_CHANNEL6, buffer, (void *)p_tx_reg[module_id], index_buffer, 1, 1);
    DmaChnStartTxfer(DMA_CHANNEL6, DMA_WAIT_NOT, 0);
    dma_tx_in_progress = true;
}
