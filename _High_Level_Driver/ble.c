/*********************************************************************
*	BLE driver
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		08/11/2018		- Initial release
*********************************************************************/

#include "../PLIB.h"

static ble_params_t * p_ble;

static void _pa_lna(uint8_t *buffer);
static void _led_status(uint8_t *buffer);
static void _name(uint8_t *buffer);
static void _version(uint8_t *buffer);
static void _adv_interval(uint8_t *buffer);
static void _adv_timeout(uint8_t *buffer);
static void _reset(uint8_t *buffer);
static void _buffer(uint8_t *buffer);
static void _scenario(uint8_t *buffer);

static uint8_t vsd_outgoing_message_uart(p_function ptr);

static void ble_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_UART_ERROR:
            
            break;
            
        case IRQ_UART_RX:
            
            p_ble->uart.receive_in_progress = true;
            p_ble->uart.tick = mGetTick();
            p_ble->uart.buffer[p_ble->uart.index] = (uint8_t) (data);
            p_ble->uart.index++;
            break;
            
        case IRQ_UART_TX:
            
            break;
            
    }
}

void ble_init(ble_params_t * p_ble_params)
{   
    DmaChnOpen(DMA_CHANNEL2, DMA_CHN_PRI0, DMA_OPEN_MATCH);
    DmaChnSetEvEnableFlags(DMA_CHANNEL2, DMA_EV_BLOCK_DONE);	// enable the transfer done interrupt, when all buffer transferred
    DmaChnSetEventControl(DMA_CHANNEL2, DMA_EV_START_IRQ(_UART4_TX_IRQ));
    
    uart_init(UART4, ble_event_handler, IRQ_UART_RX, UART_BAUDRATE_1M, UART_STD_PARAMS);
    
    p_ble = p_ble_params;
    
    p_ble->flags.pa_lna = 1;
    p_ble->flags.led_status = 1;
    p_ble->flags.set_name = 1;
    p_ble->flags.get_version = 1;
    p_ble->flags.adv_interval = 1;
    p_ble->flags.adv_timeout = 1;
}

void __ISR(_DMA_2_VECTOR, IPL3SOFT) Dma2Handler(void)
{
    p_ble->uart.dma_tx_in_progress = false;
    irq_clr_flag(IRQ_DMA2);
}

void ble_stack_tasks()
{    
    p_ble->uart.transmit_in_progress = (p_ble->uart.dma_tx_in_progress | !U4STAbits.TRMT);
    
    if (p_ble->uart.index > 0)
    {
        if (mTickCompare(p_ble->uart.tick) >= TICK_300US)
        {
            if (	(p_ble->uart.index == 3) && \
                    (p_ble->uart.buffer[0] == 'A') && \
                    (p_ble->uart.buffer[1] == 'C') && \
                    (p_ble->uart.buffer[2] == 'K'))
            {
                p_ble->uart.message_type = UART_ACK_MESSAGE;
            }
            else if (	(p_ble->uart.index == 4) && \
                        (p_ble->uart.buffer[0] == 'N') && \
                        (p_ble->uart.buffer[1] == 'A') && \
                        (p_ble->uart.buffer[2] == 'C') && \
                        (p_ble->uart.buffer[3] == 'K'))
            {
                p_ble->uart.message_type = UART_NACK_MESSAGE;
            }
            else if (	(p_ble->uart.index > 5) && (p_ble->uart.buffer[1] == 'N'))
            {
                p_ble->uart.message_type = UART_NEW_MESSAGE;
            }
            else
            {
                p_ble->uart.message_type = UART_OTHER_MESSAGE;
            }
            p_ble->uart.index = 0;
            p_ble->uart.receive_in_progress = false;
        }
    }
    
    if (p_ble->uart.message_type == UART_NEW_MESSAGE)
    {
        uint8_t i;
        uint16_t crc_calc, crc_uart;
    
        p_ble->uart.message_type = UART_NO_MESSAGE;
        
        crc_calc = fu_crc_16_ibm(p_ble->uart.buffer, p_ble->uart.buffer[2]+3);
        crc_uart = (p_ble->uart.buffer[p_ble->uart.buffer[2]+3] << 8) + (p_ble->uart.buffer[p_ble->uart.buffer[2]+4] << 0);

        if (crc_calc == crc_uart)
        {
            p_ble->incoming_message_uart.id = p_ble->uart.buffer[0];
            p_ble->incoming_message_uart.type = p_ble->uart.buffer[1];
            p_ble->incoming_message_uart.length = p_ble->uart.buffer[2];
            for (i = 0 ; i < p_ble->incoming_message_uart.length ; i++)
            {
                p_ble->incoming_message_uart.data[i] = p_ble->uart.buffer[3+i];
            }
            DmaChnSetTxfer(DMA_CHANNEL2, "ACK", (void*)&U4TXREG, 3, 1, 1);
            DmaChnStartTxfer(DMA_CHANNEL2, DMA_WAIT_NOT, 0);
            p_ble->uart.dma_tx_in_progress = true;
        }
        else
        {
            p_ble->incoming_message_uart.id = 0x00;
            DmaChnSetTxfer(DMA_CHANNEL2, "NACK", (void*)&U4TXREG, 4, 1, 1);
            DmaChnStartTxfer(DMA_CHANNEL2, DMA_WAIT_NOT, 0);
            p_ble->uart.dma_tx_in_progress = true;
        }
        memset(p_ble->uart.buffer, 0, sizeof(p_ble->uart.buffer));
       
        switch (p_ble->incoming_message_uart.id)
        {
            case ID_GET_VERSION:
                for (i = 0 ; i < p_ble->incoming_message_uart.length ; i++)
                {
                    p_ble->infos.vsd_version[i] = p_ble->incoming_message_uart.data[i];
                }
                p_ble->infos.vsd_version[i] = '\0';
                break;
                
            case ID_CHAR_BUFFER:
                memcpy(p_ble->service.buffer.in_data, p_ble->incoming_message_uart.data, p_ble->incoming_message_uart.length);
                p_ble->service.buffer.in_length = p_ble->incoming_message_uart.length;
                p_ble->service.buffer.in_is_updated = true;
                break;
                
            case ID_CHAR_SCENARIO:
                p_ble->service.scenario.in_index = p_ble->incoming_message_uart.data[0];
                p_ble->service.scenario.in_is_updated = true;
                break;
                
            case ID_SOFTWARE_RESET:
                if ((p_ble->incoming_message_uart.length == 1) && ((p_ble->incoming_message_uart.data[0] == RESET_ALL) || (p_ble->incoming_message_uart.data[0] == RESET_PIC32)))
				{
					p_ble->flags.exec_reset = true;
				}
                break;

            default:
                break;

        }
    }
    
    if (p_ble->flags.w > 0)
    {
        if (p_ble->flags.pa_lna)
        {
            if (!vsd_outgoing_message_uart(_pa_lna))
            {
                p_ble->flags.pa_lna = 0;
            }
        }
        else if (p_ble->flags.led_status)
        {
            if (!vsd_outgoing_message_uart(_led_status))
            {
                p_ble->flags.led_status = 0;
            }
        }
        else if (p_ble->flags.set_name)
        {
            if (!vsd_outgoing_message_uart(_name))
            {
                p_ble->flags.set_name = 0;
            }
        }
        else if (p_ble->flags.get_version)
        {
            if (!vsd_outgoing_message_uart(_version))
            {
                p_ble->flags.get_version = 0;
            }
        }
        else if (p_ble->flags.adv_interval)
        {
            if (!vsd_outgoing_message_uart(_adv_interval))
            {
                p_ble->flags.adv_interval = 0;
            }
        }
        else if (p_ble->flags.adv_timeout)
        {
            if (!vsd_outgoing_message_uart(_adv_timeout))
            {
                p_ble->flags.adv_timeout = 0;
            }
        }
        else if (p_ble->flags.send_reset)
        {
            if (!vsd_outgoing_message_uart(_reset))
            {
                if ((p_ble->params.reset_type == RESET_ALL) || (p_ble->params.reset_type == RESET_PIC32))
                {
                    SoftReset();
                }
            }
        }
        else if (p_ble->flags.exec_reset)
        {
            if (!p_ble->uart.transmit_in_progress)
            {
                SoftReset();
            }
        }
        else if (p_ble->flags.send_buffer)
        {
            if (!vsd_outgoing_message_uart(_buffer))
            {
                p_ble->flags.send_buffer = 0;
            }
        }
        else if (p_ble->flags.send_scenario)
        {
            if (!vsd_outgoing_message_uart(_scenario))
            {
                p_ble->flags.send_scenario = 0;
            }
        }
    }
    
}

static void _pa_lna(uint8_t *buffer)
{
    uint8_t i = 0;
	uint16_t crc = 0;

	buffer[0] = ID_PA_LNA;
	buffer[1] = 'W';
    buffer[2] = 1;
    buffer[3] = p_ble->params.pa_lna_enable;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _led_status(uint8_t *buffer)
{
    uint8_t i = 0;
	uint16_t crc = 0;

	buffer[0] = ID_LED_STATUS;
	buffer[1] = 'W';
    buffer[2] = 1;
    buffer[3] = p_ble->params.led_status_enable;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _name(uint8_t *buffer)
{
    uint8_t i = 0;
	uint16_t crc = 0;

	buffer[0] = ID_SET_NAME;
	buffer[1] = 'W';
	
	for (i = 0 ; i < 20 ; i++)
    {
        if (p_ble->infos.device_name[i] == '\0')
        {
            break;
        }
        buffer[3+i] = p_ble->infos.device_name[i];
    }
    buffer[2] = i;
    
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _version(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_GET_VERSION;
	buffer[1] = 'W';
	buffer[2] = 1;
    buffer[3] = 0x00;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _adv_interval(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_ADV_INTERVAL;
	buffer[1] = 'W';
	buffer[2] = 2;
    buffer[3] = (p_ble->params.preferred_gap_params.adv_interval >> 8) & 0xff;
    buffer[4] = (p_ble->params.preferred_gap_params.adv_interval >> 0) & 0xff;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _adv_timeout(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_ADV_TIMEOUT;
	buffer[1] = 'W';
	buffer[2] = 2;
    buffer[3] = (p_ble->params.preferred_gap_params.adv_timeout >> 8) & 0xff;
    buffer[4] = (p_ble->params.preferred_gap_params.adv_timeout >> 0) & 0xff;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _reset(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_SOFTWARE_RESET;
	buffer[1] = 'W';
    buffer[2] = 1;
    buffer[3] = RESET_ALL;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _buffer(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_CHAR_BUFFER;
	buffer[1] = 'W';
	buffer[2] = p_ble->service.buffer.out_length;
    memcpy(&buffer[3], p_ble->service.buffer.out_data, p_ble->service.buffer.out_length);
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _scenario(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_CHAR_SCENARIO;
	buffer[1] = 'W';
	buffer[2] = 1;
    buffer[3] = p_ble->service.scenario.out_index;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static uint8_t vsd_outgoing_message_uart(p_function ptr)
{
    static state_machine_t sm;
	static uint8_t buffer[256] = {0};

	switch (sm.index)
	{

        case 0:
            sm.index++;
            sm.tick = mGetTick();
        case 1:
            
            if (!p_ble->uart.transmit_in_progress && !p_ble->uart.receive_in_progress)
            {
                sm.index++;
                sm.tick = mGetTick();
            }
            break;
            
        case 2:
            if (mTickCompare(sm.tick) >= TICK_400US)
        	{
        		if (!p_ble->uart.transmit_in_progress && !p_ble->uart.receive_in_progress)
				{
					sm.index++;
					sm.tick = mGetTick();
				}
        		else
        		{
        			sm.index = 1;
        		}
        	}
            break;
            
		case 3:
            
            (*ptr)(buffer);

            DmaChnSetTxfer(DMA_CHANNEL2, buffer, (void*)&U4TXREG, buffer[2]+5, 1, 1);
            DmaChnStartTxfer(DMA_CHANNEL2, DMA_WAIT_NOT, 0);
            p_ble->uart.dma_tx_in_progress = true;

			sm.index++;
			sm.tick = mGetTick();
			break;

		case 4:

            if (!p_ble->uart.transmit_in_progress)
            {
                memset(buffer, 0, sizeof(buffer));
                sm.index++;
                sm.tick = mGetTick();
            }
			break;

		case 5:

            if (p_ble->uart.message_type == UART_ACK_MESSAGE)
            {
                p_ble->uart.message_type = UART_NO_MESSAGE;
                sm.index = 0;
            }
            else if (p_ble->uart.message_type == UART_NACK_MESSAGE)
            {
                p_ble->uart.message_type = UART_NO_MESSAGE;
                sm.index = 3;
            }
            else if (mTickCompare(sm.tick) >= TICK_10MS)
            {
                sm.index = 3;
            }
			break;

		default:
            sm.index = 0;
			break;

	}

	return sm.index;
}
 