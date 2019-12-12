/*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		19/10/2018		- Initial release
*       19/03/2019      - Implementation with new DMA PLIB driver.
* 
*********************************************************************/

#include "../PLIB.h"

static UART_MODULE m_uart_id = UART_NUMBER_OF_MODULES;
static DMA_MODULE m_dma_id = DMA_NUMBER_OF_MODULES;
static dma_channel_transfer_t dma_tx = {0};

void log_init(UART_MODULE id_uart, uint32_t data_rate)
{
    m_uart_id = id_uart;
    m_dma_id = dma_get_free_channel();
    
    uart_init(  id_uart, NULL, IRQ_NONE, data_rate, UART_STD_PARAMS);
    
    dma_init(   m_dma_id, 
                NULL, 
                DMA_CONT_PRIO_2, 
                DMA_INT_NONE, 
                DMA_EVT_START_TRANSFER_ON_IRQ, 
                uart_get_tx_irq(id_uart), 
                0xff);
}

static uint16_t _transform_integer_to_string(char *p_buffer, uint16_t index_p_buffer, uint32_t value, STR_BASE_t _base, uint8_t number_of_char)
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

void log_frontend(const char *p_message, LOG_LEVEL_t level, const uint32_t *p_args, uint8_t nargs)
{
    if (m_uart_id != UART_NUMBER_OF_MODULES)
    {        
        static char buffer[16000] = {0};
        char *p_str = (char *) p_message;        
        uint16_t index_buffer = 0;
        uint8_t index_args = 0;
        uint8_t _number_of_char_to_print = 0;
        
        // Wait while channel is enable (Block Transfer not yet done). 
        // When a transmission is finished (Block Transfer Done), the channel is automatically disable (not set in DMA_CONT_AUTO_ENABLE).
        while (dma_channel_is_enable(m_dma_id));
        dma_clear_flags(m_dma_id, DMA_FLAG_BLOCK_TRANSFER_DONE);            

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

        dma_tx.src_start_addr = buffer;
        dma_tx.dst_start_addr = (void *)uart_get_tx_reg(m_uart_id);
        dma_tx.src_size = index_buffer;
        dma_tx.dst_size = 1;
        dma_tx.cell_size = 1;
        dma_tx.pattern_data = 0x0000,

        dma_set_transfer_params(m_dma_id, &dma_tx);  
        dma_channel_enable(m_dma_id, ON, true);     // Do not take care of the 'force_transfer' boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer_params()).
    }
}
