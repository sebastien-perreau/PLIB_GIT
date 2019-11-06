/*********************************************************************
*	BLE driver
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		08/11/2018		- Initial release
*********************************************************************/

#include "../PLIB.h"

static ble_params_t * p_ble;
static UART_MODULE m_uart_id;
static DMA_MODULE m_dma_id;
static DMA_CHANNEL_TRANSFER dma_tx = {NULL, NULL, 0, 0, 0, 0x0000};

static const char _ack[] = "ACK";
static const char _nack[] = "NACK";

static void _pa_lna(uint8_t *buffer);
static void _led_status(uint8_t *buffer);
static void _name(uint8_t *buffer);
static void _version(uint8_t *buffer);
static void _adv_interval(uint8_t *buffer);
static void _adv_timeout(uint8_t *buffer);
static void _reset_ble_pickit(uint8_t *buffer);
static void _reset_all(uint8_t *buffer);
static void _conn_params(uint8_t *buffer);
static void _phy_params(uint8_t *buffer);
static void _att_size_params(uint8_t *buffer);
static void _buffer(uint8_t *buffer);

static uint8_t vsd_outgoing_message_uart(p_ble_function ptr);

static void __boot_sequence()
{
    p_ble->status.flags.pa_lna = 1;
    p_ble->status.flags.led_status = 1;
    p_ble->status.flags.set_name = 1;
    p_ble->status.flags.get_version = 1;
    p_ble->status.flags.adv_interval = 1;
    p_ble->status.flags.adv_timeout = 1;
    
    p_ble->status.flags.set_conn_params = 1;
    p_ble->status.flags.set_phy_params = 1;
    p_ble->status.flags.set_att_size_params = 1;
}

static void ble_uart_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_UART_ERROR:
            
            break;
            
        case IRQ_UART_RX:
            
            p_ble->uart.receive_in_progress = true;
            p_ble->uart.buffer[p_ble->uart.index] = (uint8_t) (data);
            p_ble->uart.index++;
            break;
            
        case IRQ_UART_TX:
            
            break;
            
    }
}

void ble_init(UART_MODULE uart_id, uint32_t data_rate, ble_params_t * p_ble_params)
{       
    p_ble = p_ble_params;
    m_uart_id = uart_id;
    m_dma_id = dma_get_free_channel();
    
    uart_init(  uart_id, ble_uart_event_handler, IRQ_UART_RX, data_rate, UART_STD_PARAMS);
    
    dma_init(   m_dma_id, 
                NULL, 
                DMA_CONT_PRIO_2, 
                DMA_INT_NONE, 
                DMA_EVT_START_TRANSFER_ON_IRQ, 
                uart_get_tx_irq(uart_id), 
                0xff);   
    
    p_ble->status.flags.send_reset_ble_pickit = 1;
}

void ble_stack_tasks()
{         
    
    if (p_ble->uart.index != p_ble->uart.old_index)
    {                
        mUpdateTick(p_ble->uart.tick);                        
        p_ble->uart.old_index = p_ble->uart.index;
    }
    else if (p_ble->uart.index > 0)
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
            p_ble->uart.old_index = p_ble->uart.index;
        }
    }
    
    if (p_ble->uart.message_type == UART_NEW_MESSAGE)
    {
        uint16_t i;
        uint16_t crc_calc, crc_uart;
    
        p_ble->uart.message_type = UART_NO_MESSAGE;
        
        if (p_ble->uart.buffer[0] == ID_CHAR_EXTENDED_BUFFER_NO_CRC)
        {            
            p_ble->incoming_message_uart.id = ID_NONE;
            p_ble->service.extended_buffer.in_length = (p_ble->uart.buffer[2] << 0) | (p_ble->uart.buffer[3] << 8);
            memcpy(p_ble->service.extended_buffer.in_data, &p_ble->uart.buffer[4], p_ble->service.extended_buffer.in_length);
            p_ble->service.extended_buffer.in_is_updated = true;
        }
        else
        {
            crc_calc = fu_crc_16_ibm(p_ble->uart.buffer, p_ble->uart.buffer[2] + 3);
            crc_uart = (p_ble->uart.buffer[p_ble->uart.buffer[2] + 3] << 8) + (p_ble->uart.buffer[p_ble->uart.buffer[2] + 4] << 0);

            if (crc_calc == crc_uart)
            {
                memcpy(&p_ble->incoming_message_uart, p_ble->uart.buffer, p_ble->uart.buffer[2] + 3);
                dma_tx.src_start_addr = (void *)_ack;
                dma_tx.dst_start_addr = (void *)uart_get_tx_reg(m_uart_id);
                dma_tx.src_size = 3;
                dma_tx.dst_size = 1;
                dma_tx.cell_size = 1;
                dma_set_transfer(m_dma_id, &dma_tx, true);
            }
            else
            {
                p_ble->incoming_message_uart.id = ID_NONE;
                dma_tx.src_start_addr = (void *)_nack;
                dma_tx.dst_start_addr = (void *)uart_get_tx_reg(m_uart_id);
                dma_tx.src_size = 4;
                dma_tx.dst_size = 1;
                dma_tx.cell_size = 1;
                dma_set_transfer(m_dma_id, &dma_tx, true);
            }            
        }    
        memset(p_ble->uart.buffer, 0, sizeof(p_ble->uart.buffer));
       
        switch (p_ble->incoming_message_uart.id)
        {
            case ID_BOOT_MODE:
                if ((p_ble->incoming_message_uart.type == 'N') && (p_ble->incoming_message_uart.length == 1) && (p_ble->incoming_message_uart.data[0] == 0x23))
				{
                    if (!p_ble->status.flags.send_reset_ble_pickit)
                    {
                        __boot_sequence();
                    }
                }                
                break;
                
            case ID_GET_VERSION:
                memcpy(p_ble->status.infos.vsd_version, p_ble->incoming_message_uart.data, p_ble->incoming_message_uart.length);
                p_ble->status.infos.vsd_version[i] = '\0';
                break;
                
            case ID_GET_CONN_STATUS:
                p_ble->status.connection.is_conn_status_updated = true;
                p_ble->status.connection.is_connected_to_a_central = ((p_ble->incoming_message_uart.data[0] >> 1) & 0x01);
                p_ble->status.connection.is_in_advertising_mode = ((p_ble->incoming_message_uart.data[0] >> 0) & 0x01);
                break;
                
            case ID_GET_BLE_PARAMS:
                p_ble->status.gap.is_gap_params_updated = true;
                
                p_ble->status.gap.current_gap_params.conn_params.min_conn_interval = (p_ble->incoming_message_uart.data[0] << 8) | (p_ble->incoming_message_uart.data[1] << 0);
                p_ble->status.gap.current_gap_params.conn_params.max_conn_interval = (p_ble->incoming_message_uart.data[2] << 8) | (p_ble->incoming_message_uart.data[3] << 0);
                p_ble->status.gap.current_gap_params.conn_params.slave_latency = (p_ble->incoming_message_uart.data[4] << 8) | (p_ble->incoming_message_uart.data[5] << 0);
                p_ble->status.gap.current_gap_params.conn_params.conn_sup_timeout = (p_ble->incoming_message_uart.data[6] << 8) | (p_ble->incoming_message_uart.data[7] << 0);
                
                p_ble->status.gap.current_gap_params.phys_params = p_ble->incoming_message_uart.data[8];
                
                p_ble->status.gap.current_gap_params.mtu_size_params.max_tx_octets = p_ble->incoming_message_uart.data[9];
                p_ble->status.gap.current_gap_params.mtu_size_params.max_rx_octets = p_ble->incoming_message_uart.data[10];
                
                p_ble->status.gap.current_gap_params.adv_timeout = p_ble->params.preferred_gap_params.adv_timeout;
                p_ble->status.gap.current_gap_params.adv_interval = p_ble->params.preferred_gap_params.adv_interval;
                
                p_ble->status.hardware.is_pa_lna_enabled = p_ble->incoming_message_uart.data[11];
                p_ble->status.hardware.is_led_status_enabled = p_ble->incoming_message_uart.data[12];
                break;
                
            case ID_PA_LNA:
                if ((p_ble->incoming_message_uart.data[0] & 1) != p_ble->params.pa_lna_enable)
                {
                    p_ble->params.pa_lna_enable = p_ble->incoming_message_uart.data[0];
                    p_ble->status.flags.send_reset_ble_pickit = 1;
                }
                break;
                
            case ID_CHAR_BUFFER:
                memcpy(p_ble->service.buffer.in_data, p_ble->incoming_message_uart.data, p_ble->incoming_message_uart.length);
                p_ble->service.buffer.in_length = p_ble->incoming_message_uart.length;
                p_ble->service.buffer.in_is_updated = true;
                break;
                
            case ID_SOFTWARE_RESET:
                if ((p_ble->incoming_message_uart.length == 1) && (p_ble->incoming_message_uart.data[0] == RESET_ALL))
				{
					p_ble->status.flags.exec_reset = true;
				}
                break;

            default:
                break;

        }
    }
    
    if (p_ble->status.flags.w > 0)
    {
        if (p_ble->status.flags.exec_reset)
        {
            if (uart_transmission_has_completed(m_uart_id))
            {
                SoftReset();
            }
        }
        else if (p_ble->status.flags.send_reset_ble_pickit)
        {
            if (!vsd_outgoing_message_uart(_reset_ble_pickit))
            {
                p_ble->status.flags.send_reset_ble_pickit = 0;
            }
        }
        else if (p_ble->status.flags.send_reset_all)
        {
            if (!vsd_outgoing_message_uart(_reset_ble_pickit))
            {
                SoftReset();
            }
        } 
        else if (p_ble->status.flags.pa_lna)
        {
            if (!vsd_outgoing_message_uart(_pa_lna))
            {
                p_ble->status.flags.pa_lna = 0;
            }
        }
        else if (p_ble->status.flags.led_status)
        {
            if (!vsd_outgoing_message_uart(_led_status))
            {
                p_ble->status.flags.led_status = 0;
            }
        }
        else if (p_ble->status.flags.set_name)
        {
            if (!vsd_outgoing_message_uart(_name))
            {
                p_ble->status.flags.set_name = 0;
            }
        }
        else if (p_ble->status.flags.get_version)
        {
            if (!vsd_outgoing_message_uart(_version))
            {
                p_ble->status.flags.get_version = 0;
            }
        }
        else if (p_ble->status.flags.adv_interval)
        {
            if (!vsd_outgoing_message_uart(_adv_interval))
            {
                p_ble->status.flags.adv_interval = 0;
            }
        }
        else if (p_ble->status.flags.adv_timeout)
        {
            if (!vsd_outgoing_message_uart(_adv_timeout))
            {
                p_ble->status.flags.adv_timeout = 0;
            }
        }               
        else if (p_ble->status.flags.set_conn_params)
        {
            if (!vsd_outgoing_message_uart(_conn_params))
            {
                p_ble->status.flags.set_conn_params = 0;
            }
        }
        else if (p_ble->status.flags.set_phy_params)
        {
            if (!vsd_outgoing_message_uart(_phy_params))
            {
                p_ble->status.flags.set_phy_params = 0;
            }
        }
        else if (p_ble->status.flags.set_att_size_params)
        {
            if (!vsd_outgoing_message_uart(_att_size_params))
            {
                p_ble->status.flags.set_att_size_params = 0;
            }
        }
        else if (p_ble->status.flags.send_buffer)
        {
            if (!vsd_outgoing_message_uart(_buffer))
            {
                p_ble->status.flags.send_buffer = 0;
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
        if (p_ble->status.infos.device_name[i] == '\0')
        {
            break;
        }
        buffer[3+i] = p_ble->status.infos.device_name[i];
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

static void _reset_ble_pickit(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_SOFTWARE_RESET;
	buffer[1] = 'W';
    buffer[2] = 1;
    buffer[3] = RESET_BLE_PICKIT;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _reset_all(uint8_t *buffer)
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

static void _conn_params(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_SET_BLE_CONN_PARAMS;
	buffer[1] = 'W';
    buffer[2] = 8;
    buffer[3] = (p_ble->params.preferred_gap_params.conn_params.min_conn_interval >> 8) & 0xff;
    buffer[4] = (p_ble->params.preferred_gap_params.conn_params.min_conn_interval >> 0) & 0xff;
    buffer[5] = (p_ble->params.preferred_gap_params.conn_params.max_conn_interval >> 8) & 0xff;
    buffer[6] = (p_ble->params.preferred_gap_params.conn_params.max_conn_interval >> 0) & 0xff;
    buffer[7] = (p_ble->params.preferred_gap_params.conn_params.slave_latency >> 8) & 0xff;
    buffer[8] = (p_ble->params.preferred_gap_params.conn_params.slave_latency >> 0) & 0xff;
    buffer[9] = (p_ble->params.preferred_gap_params.conn_params.conn_sup_timeout >> 8) & 0xff;
    buffer[10] = (p_ble->params.preferred_gap_params.conn_params.conn_sup_timeout >> 0) & 0xff;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _phy_params(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_SET_BLE_PHY_PARAMS;
	buffer[1] = 'W';
    buffer[2] = 1;
    buffer[3] = p_ble->params.preferred_gap_params.phys_params;
	crc = fu_crc_16_ibm(buffer, buffer[2]+3);
	buffer[buffer[2]+3] = (crc >> 8) & 0xff;
	buffer[buffer[2]+4] = (crc >> 0) & 0xff;
}

static void _att_size_params(uint8_t *buffer)
{
	uint16_t crc = 0;

	buffer[0] = ID_SET_BLE_ATT_SIZE_PARAMS;
	buffer[1] = 'W';
    buffer[2] = 2;
    buffer[3] = p_ble->params.preferred_gap_params.mtu_size_params.max_tx_octets;
    buffer[4] = p_ble->params.preferred_gap_params.mtu_size_params.max_rx_octets;
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

static uint8_t vsd_outgoing_message_uart(p_ble_function ptr)
{
    static state_machine_t sm;
	static uint8_t buffer[256] = {0};

	switch (sm.index)
	{

        case 0:
            sm.index++;
            sm.tick = mGetTick();
        case 1:
            
            if (uart_transmission_has_completed(m_uart_id) && !p_ble->uart.receive_in_progress)
            {
                memset(buffer, 0, sizeof(buffer));
                sm.index++;
                sm.tick = mGetTick();
            }
            break;
            
        case 2:
            if (mTickCompare(sm.tick) >= TICK_400US)
        	{
        		if (uart_transmission_has_completed(m_uart_id) && !p_ble->uart.receive_in_progress)
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

            dma_tx.src_start_addr = (void *)buffer;
            dma_tx.dst_start_addr = (void *)uart_get_tx_reg(m_uart_id);
            dma_tx.src_size = buffer[2] + 5;
            dma_tx.dst_size = 1;
            dma_tx.cell_size = 1;
            dma_set_transfer(m_dma_id, &dma_tx, true);

			sm.index++;
			sm.tick = mGetTick();
			break;

		case 4:
        
            if (uart_transmission_has_completed(m_uart_id))
            {
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
 