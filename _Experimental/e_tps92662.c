/*********************************************************************
*	External TPS92662
*	Author : S�bastien PERREAU
*
*	Revision history	:
*		02/05/2019		- Initial release
* 
*   LED behavior function of current driver:
*   ----------------------------------------
* 
*   - CINCO driver: When using a LOW duty cycle, the LED can blink. To
*   reduce this blink/glitter, you can reduce the PWM frequency and/or 
*   keep a LED always to 100% (the CINCO driver will not enter in a
*   sleep mode - it enter in this mode when a short circuit is detected).
* 
*   - MOGNOT driver: No problem detected. 
*********************************************************************/

#include "../PLIB.h"

static const uint8_t tps92662_init_param[] =
{
    0x87, 0x99, 0x1e, 0xaa, 0x2d, 0x33, 0xb4,
    0x4b, 0xcc, 0xd2, 0x55, 0xe1, 0x66, 0x78
};

static const uint8_t tps92662_init_param_number_of_bytes[] =
{
    1, 2, 3, 4, 12, 16, 32,
    1, 2, 3, 4, 12, 16, 32
};

static const uint8_t tps92662_device_id[] = 
{
    0x20, 0x61, 0xe2, 0xa3, 0x64, 0x25, 0xa6, 0xe7,
    0xa8, 0xe9, 0x6a, 0x2b, 0xec, 0xad, 0x2e, 0x6f, 
    0xf0, 0xb1, 0x32, 0x73, 0xb4, 0xf5, 0x76, 0x37,
    0x78, 0x39, 0xba, 0xfb, 0x3c, 0x7d, 0xfe, 0xbf
};

static void _get_ic_identifier(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    var->p_registers[device_index].ic_identification = buffer[0];
}

static void _get_errors(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    var->p_registers[device_index].fault_error = (buffer[0] << 0) + ((buffer[1] & 0x0f) << 8);
    var->p_registers[device_index].crc_error = buffer[2];
}

static void _get_adc(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    var->p_registers[device_index].adc[0] = buffer[0];
    var->p_registers[device_index].adc[1] = buffer[1];
}

static void _set_system_config(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].system_config;
}

static void _set_slew_rate(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].slew_rate;
}

static void _set_overvoltage_limit(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].over_voltage_limit;
}

static void _set_parallel_led_string(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].parallel_led_string;
}

static void _set_default_pulse_width(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = ((var->p_registers[device_index].default_pulse_width[0] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[1] & 0x0f) << 4);
    buffer[1] = ((var->p_registers[device_index].default_pulse_width[2] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[3] & 0x0f) << 4);
    buffer[2] = ((var->p_registers[device_index].default_pulse_width[4] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[5] & 0x0f) << 4);
    buffer[3] = ((var->p_registers[device_index].default_pulse_width[6] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[7] & 0x0f) << 4);
    buffer[4] = ((var->p_registers[device_index].default_pulse_width[8] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[9] & 0x0f) << 4);
    buffer[5] = ((var->p_registers[device_index].default_pulse_width[10] & 0x0f) << 0) + ((var->p_registers[device_index].default_pulse_width[11] & 0x0f) << 4);
    
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10] = 0;
    buffer[11] = 0;
}

static void _set_watchdog_timer(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer) 
{
    buffer[0] = var->p_registers[device_index].watchdog_timer;
}

static void _set_pwm_tick_period(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].pwm_tick_period;
}

static void _set_adc_id(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].adc_id;
}

static void _set_softsync(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
	buffer[0] = var->p_registers[device_index].softsync;
}

static void _set_phase_and_width(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    buffer[0] = (var->p_registers[device_index].phase[0] & 0xff);
    buffer[1] = (var->p_registers[device_index].phase[1] & 0xff);
    buffer[2] = (var->p_registers[device_index].phase[2] & 0xff);
    buffer[3] = (((var->p_registers[device_index].phase[0] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[1] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[2] >> 8) & 0x03) << 4);
    
    buffer[4] = (var->p_registers[device_index].phase[3] & 0xff);
    buffer[5] = (var->p_registers[device_index].phase[4] & 0xff);
    buffer[6] = (var->p_registers[device_index].phase[5] & 0xff);
    buffer[7] = (((var->p_registers[device_index].phase[3] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[4] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[5] >> 8) & 0x03) << 4);
   
    buffer[8] = (var->p_registers[device_index].phase[6] & 0xff);
    buffer[9] = (var->p_registers[device_index].phase[7] & 0xff);
    buffer[10] = (var->p_registers[device_index].phase[8] & 0xff);
    buffer[11] = (((var->p_registers[device_index].phase[6] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[7] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[8] >> 8) & 0x03) << 4);
    
    buffer[12] = (var->p_registers[device_index].phase[9] & 0xff);
    buffer[13] = (var->p_registers[device_index].phase[10] & 0xff);
    buffer[14] = (var->p_registers[device_index].phase[11] & 0xff);
    buffer[15] = (((var->p_registers[device_index].phase[9] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[10] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[11] >> 8) & 0x03) << 4);
    
    buffer[16] = (var->p_registers[device_index].width[0] & 0xff);
    buffer[17] = (var->p_registers[device_index].width[1] & 0xff);
    buffer[18] = (var->p_registers[device_index].width[2] & 0xff);
    buffer[19] = (((var->p_registers[device_index].width[0] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[1] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[2] >> 8) & 0x03) << 4);
    
    buffer[20] = (var->p_registers[device_index].width[3] & 0xff);
    buffer[21] = (var->p_registers[device_index].width[4] & 0xff);
    buffer[22] = (var->p_registers[device_index].width[5] & 0xff);
    buffer[23] = (((var->p_registers[device_index].width[3] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[4] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[5] >> 8) & 0x03) << 4);
   
    buffer[24] = (var->p_registers[device_index].width[6] & 0xff);
    buffer[25] = (var->p_registers[device_index].width[7] & 0xff);
    buffer[26] = (var->p_registers[device_index].width[8] & 0xff);
    buffer[27] = (((var->p_registers[device_index].width[6] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[7] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[8] >> 8) & 0x03) << 4);
    
    buffer[28] = (var->p_registers[device_index].width[9] & 0xff);
    buffer[29] = (var->p_registers[device_index].width[10] & 0xff);
    buffer[30] = (var->p_registers[device_index].width[11] & 0xff);
    buffer[31] = (((var->p_registers[device_index].width[9] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[10] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[11] >> 8) & 0x03) << 4);
}

static void _set_phase(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    buffer[0] = (var->p_registers[device_index].phase[0] & 0xff);
    buffer[1] = (var->p_registers[device_index].phase[1] & 0xff);
    buffer[2] = (var->p_registers[device_index].phase[2] & 0xff);
    buffer[3] = (((var->p_registers[device_index].phase[0] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[1] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[2] >> 8) & 0x03) << 4);
    
    buffer[4] = (var->p_registers[device_index].phase[3] & 0xff);
    buffer[5] = (var->p_registers[device_index].phase[4] & 0xff);
    buffer[6] = (var->p_registers[device_index].phase[5] & 0xff);
    buffer[7] = (((var->p_registers[device_index].phase[3] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[4] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[5] >> 8) & 0x03) << 4);
   
    buffer[8] = (var->p_registers[device_index].phase[6] & 0xff);
    buffer[9] = (var->p_registers[device_index].phase[7] & 0xff);
    buffer[10] = (var->p_registers[device_index].phase[8] & 0xff);
    buffer[11] = (((var->p_registers[device_index].phase[6] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[7] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[8] >> 8) & 0x03) << 4);
    
    buffer[12] = (var->p_registers[device_index].phase[9] & 0xff);
    buffer[13] = (var->p_registers[device_index].phase[10] & 0xff);
    buffer[14] = (var->p_registers[device_index].phase[11] & 0xff);
    buffer[15] = (((var->p_registers[device_index].phase[9] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].phase[10] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].phase[11] >> 8) & 0x03) << 4);
}

static void _set_width(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer)
{
    buffer[0] = (var->p_registers[device_index].width[0] & 0xff);
    buffer[1] = (var->p_registers[device_index].width[1] & 0xff);
    buffer[2] = (var->p_registers[device_index].width[2] & 0xff);
    buffer[3] = (((var->p_registers[device_index].width[0] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[1] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[2] >> 8) & 0x03) << 4);
    
    buffer[4] = (var->p_registers[device_index].width[3] & 0xff);
    buffer[5] = (var->p_registers[device_index].width[4] & 0xff);
    buffer[6] = (var->p_registers[device_index].width[5] & 0xff);
    buffer[7] = (((var->p_registers[device_index].width[3] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[4] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[5] >> 8) & 0x03) << 4);
   
    buffer[8] = (var->p_registers[device_index].width[6] & 0xff);
    buffer[9] = (var->p_registers[device_index].width[7] & 0xff);
    buffer[10] = (var->p_registers[device_index].width[8] & 0xff);
    buffer[11] = (((var->p_registers[device_index].width[6] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[7] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[8] >> 8) & 0x03) << 4);
    
    buffer[12] = (var->p_registers[device_index].width[9] & 0xff);
    buffer[13] = (var->p_registers[device_index].width[10] & 0xff);
    buffer[14] = (var->p_registers[device_index].width[11] & 0xff);
    buffer[15] = (((var->p_registers[device_index].width[9] >> 8) & 0x03) << 0) | (((var->p_registers[device_index].width[10] >> 8) & 0x03) << 2) | (((var->p_registers[device_index].width[11] >> 8) & 0x03) << 4);
}

static uint8_t e_tps92662_read_request(TPS92662_PARAMS *var, TPS92662_INIT_PARAM cde_type, uint8_t device_index, uint8_t address_register, p_tps92662_function p_fct)
{
    uint16_t crc_calc, crc_uart;
    
    switch (var->state_machine_for_read_write_request.index)
    {
        case 0:     
            
            var->state_machine_for_read_write_request.index++;
            
        case 1:
            
            if (!dma_channel_is_enable(var->dma_tx_id))
            {
                var->state_machine_for_read_write_request.tick = mGetTick();            
                var->state_machine_for_read_write_request.index++;
            }
            break;
            
        case 2:
            
            var->p_transfer[0] = tps92662_init_param[cde_type];
            var->p_transfer[1] = tps92662_device_id[var->p_device_id[device_index]];
            var->p_transfer[2] = address_register;     
            crc_calc = fu_crc_16_ibm(var->p_transfer, 3);
            var->p_transfer[3] = (crc_calc >> 0) & 0xff;
            var->p_transfer[4] = (crc_calc >> 8) & 0xff;

            dma_abord_transfer(var->dma_rx_id);        
            var->dma_tx_params.src_size = 5;
            var->dma_rx_params.dst_size = (5 + tps92662_init_param_number_of_bytes[cde_type] + 2);

            dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE); 
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, true);
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, true, false);
        
            var->state_machine_for_read_write_request.tick = mGetTick();
            var->state_machine_for_read_write_request.index++;
            
        case 3:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                crc_calc = fu_crc_16_ibm(&var->p_receip[5], tps92662_init_param_number_of_bytes[cde_type]);
                crc_uart = (var->p_receip[5 + tps92662_init_param_number_of_bytes[cde_type]] << 0) + (var->p_receip[5 + tps92662_init_param_number_of_bytes[cde_type] + 1] << 8);
                if (crc_calc != crc_uart)
                {
                    var->state_machine_for_read_write_request.index = 1;   // Fail: incorrect CRC. Retry.
                }
                else
                {
                    (*p_fct)(var, device_index, &var->p_receip[5]);
                    var->state_machine_for_read_write_request.index = 0;   // End
                }
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE); 
            }
            
            if (mTickCompare(var->state_machine_for_read_write_request.tick) >= TICK_10MS)
            {
                var->state_machine_for_read_write_request.index = 1;   // Fail: nothing has been received. Retry.
            }
            break;
            
        default:
            break;
    }
    
    return var->state_machine_for_read_write_request.index;
}

static uint8_t e_tps92662_write_request(TPS92662_PARAMS *var, TPS92662_INIT_PARAM cde_type, uint8_t device_index, uint8_t address_register, p_tps92662_function p_fct)
{
    uint16_t crc_calc;
    
    switch (var->state_machine_for_read_write_request.index)
    {
        case 0:     
            
            var->state_machine_for_read_write_request.index++;
            
        case 1:
            
            if (!dma_channel_is_enable(var->dma_tx_id))
            {
                var->state_machine_for_read_write_request.tick = mGetTick();            
                var->state_machine_for_read_write_request.index++;
            }
            break;
            
        case 2:
            
            var->p_transfer[0] = tps92662_init_param[cde_type];
            var->p_transfer[1] = tps92662_device_id[var->p_device_id[device_index]];
            var->p_transfer[2] = address_register;     
            
            (*p_fct)(var, device_index, &var->p_transfer[3]);
            
            crc_calc = fu_crc_16_ibm(var->p_transfer, 3 + tps92662_init_param_number_of_bytes[cde_type]);
            var->p_transfer[3 + tps92662_init_param_number_of_bytes[cde_type]] = (crc_calc >> 0) & 0xff;
            var->p_transfer[4 + tps92662_init_param_number_of_bytes[cde_type]] = (crc_calc >> 8) & 0xff;
    
            dma_abord_transfer(var->dma_rx_id); 
            var->dma_tx_params.src_size = (3 + tps92662_init_param_number_of_bytes[cde_type] + 2);
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;
            
            dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE);             
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, true);
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, true, false);
        
            var->state_machine_for_read_write_request.tick = mGetTick();
            var->state_machine_for_read_write_request.index++;
            
        case 3:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                var->state_machine_for_read_write_request.index = 0;   // End
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE); 
            }
            break;
            
        default:
            break;
    }
    
    return var->state_machine_for_read_write_request.index;
}

uint8_t e_tps92662_deamon(TPS92662_PARAMS *var)
{
    uint8_t ret, i, device_index;
    
    if (!var->is_init_done)
    {
        if (var->chip_enable._port > 0)
        {
            ports_reset_pin_output(var->chip_enable);
            ports_clr_bit(var->chip_enable);
        }
        
        uart_init(  var->uart_id, NULL, IRQ_NONE, var->uart_baudrate, UART_STD_PARAMS);
        
        dma_init(   var->dma_tx_id, 
                    NULL,                      
                    DMA_CONT_PRIO_3, 
                    DMA_INT_NONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    uart_get_tx_irq(var->uart_id),  
                    0xff);
        
        dma_init(   var->dma_rx_id, 
                    NULL, 
                    DMA_CONT_PRIO_3, 
                    DMA_INT_BLOCK_TRANSFER_DONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    uart_get_rx_irq(var->uart_id), 
                    0xff);
              
        var->dma_tx_params.dst_start_addr = (void *) uart_get_tx_reg(var->uart_id);
        var->dma_rx_params.src_start_addr = (void *) uart_get_rx_reg(var->uart_id);
        
        for (device_index = 0 ; device_index < var->number_of_device ; device_index++)
        {
            var->p_registers[device_index].phase[0] = 0;
            var->p_registers[device_index].phase[1] = 85;
            var->p_registers[device_index].phase[2] = 170;
            var->p_registers[device_index].phase[3] = 255;
            var->p_registers[device_index].phase[4] = 340;
            var->p_registers[device_index].phase[5] = 425;
            var->p_registers[device_index].phase[6] = 510;
            var->p_registers[device_index].phase[7] = 595;
            var->p_registers[device_index].phase[8] = 680;
            var->p_registers[device_index].phase[9] = 765;
            var->p_registers[device_index].phase[10] = 850;
            var->p_registers[device_index].phase[11] = 935;
            
            var->p_registers[device_index].width[0] = 0;
            var->p_registers[device_index].width[1] = 0;
            var->p_registers[device_index].width[2] = 0;
            var->p_registers[device_index].width[3] = 0;
            var->p_registers[device_index].width[4] = 0;
            var->p_registers[device_index].width[5] = 0;
            var->p_registers[device_index].width[6] = 0;
            var->p_registers[device_index].width[7] = 0;
            var->p_registers[device_index].width[8] = 0;
            var->p_registers[device_index].width[9] = 0;
            var->p_registers[device_index].width[10] = 0;
            var->p_registers[device_index].width[11] = 0;
            
            e_tps92662_get_ic_identifier_ptr(var, device_index);
            e_tps92662_set_system_config_ptr(var, device_index, TPS92662_SYS_CONF_SET_PWR);
            e_tps92662_set_pwm_tick_period_ptr(var, device_index, TPS92662_DIV1_1 | TPS92662_DIV2_44);    // PWM_CLK = 8Mhz / (1x44x1024) = 177,56 Hz
            e_tps92662_send_phases_and_widths_ptr(var, device_index);
        }
        
        var->is_init_done = true;
        ret = 0xff;
    }
    else
    {     
        switch (var->state_machine.index)
        {
            case SM_TPS92662_HOME:
            case SM_TPS92662_SEARCH:
                   
                for (i = 1 ; i <= SM_TPS92662_MAX_FLAGS ; i++)
                {
                    for (device_index = 0 ; device_index < var->number_of_device ; device_index++)
                    {
                        if ((var->p_flags[device_index] >> i) & 0x01)
                        {
                            var->selected_device_index = device_index;
                            var->state_machine.index = i;
                            goto out_of_nested_loop;
                        }
                    }
                }
                var->state_machine.index = SM_TPS92662_HOME;
                out_of_nested_loop:
                break;
                
            case SM_TPS92662_READ_IC_IDENTIFIER:
                
                if (!e_tps92662_read_request(var, TPS_READ_1B, var->selected_device_index, TPS92662_ADDR_IC_IDENTIFIER, _get_ic_identifier))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_READ_ERRORS:
                
                if (!e_tps92662_read_request(var, TPS_READ_3B, var->selected_device_index, TPS92662_ADDR_ERRORS, _get_errors))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_READ_ADCS:
                
                if (!e_tps92662_read_request(var, TPS_READ_2B, var->selected_device_index, TPS92662_ADDR_ADC, _get_adc))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_SYSTEM_CONFIG:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_SYSTEM_CONFIG, _set_system_config))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_SLEW_RATE:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_SLEW_RATE, _set_slew_rate))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_OVER_VOLTAGE_LIMIT:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_OVER_VOLTAGE_LIMIT, _set_overvoltage_limit))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_PARALLEL_LED_STRING:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_PARALLEL_LED_STRING, _set_parallel_led_string))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_DEFAULT_PULSE_WIDTH:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_12B, var->selected_device_index, TPS92662_ADDR_DEFAULT_PULSE_WIDTH, _set_default_pulse_width))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_WATCHDOG_TIMER:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_WATCHDOG_TIMER, _set_watchdog_timer))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_PWM_TICK_PERIOD:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_PWM_TICK_PERIOD, _set_pwm_tick_period))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_ADC_ID:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_ADC_ID, _set_adc_id))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_SOFTSYNC:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_1B, var->selected_device_index, TPS92662_ADDR_SOFTSYNC, _set_softsync))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_PHASE_AND_WIDTH:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_32B, var->selected_device_index, TPS92662_ADDR_PHASE, _set_phase_and_width))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_PHASE:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_16B, var->selected_device_index, TPS92662_ADDR_PHASE, _set_phase))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            case SM_TPS92662_WRITE_WIDTH:
                
                if (!e_tps92662_write_request(var, TPS_WRITE_16B, var->selected_device_index, TPS92662_ADDR_WIDTH, _set_width))
                {
                    CLR_BIT(var->p_flags[var->selected_device_index], var->state_machine.index);
                    var->state_machine.index = SM_TPS92662_SEARCH;
                }
                break;
                
            default:
                break;
        }
        ret = var->state_machine.index;
    }
    
    return ret;
}