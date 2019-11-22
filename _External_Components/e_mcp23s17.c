/*********************************************************************
 * 
 *	External Components MCP23S17 (16 bits I/O expander with serial interface)
 * 
 *	Author : Sébastien PERREAU
 * 
*********************************************************************/

#include "../PLIB.h"

static uint8_t _mcp23s17_write_and_read(mcp23s17_params_t *var)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_SEND,
        SM_READ,
        SM_WAIT_END_OF_DMA_TRANSMISSION
    } functionState = 0;
    static mcp23s17_registers_t dummy;
    
    switch (functionState)
    {
        case SM_FREE:
                  
            var->__current_selected_device = 0;
            functionState = SM_SEND;
            
        case SM_SEND:
            
            var->write[var->__current_selected_device]._header = 0x0040 | (var->__p_device_addresses[var->__current_selected_device] << 1);
            var->write[var->__current_selected_device]._iocon_copy = var->write[var->__current_selected_device].IOCON;
            var->dma_tx_params.src_start_addr = (void *) &var->write[var->__current_selected_device];
            var->dma_rx_params.dst_start_addr = (void *) &dummy;
            
            ports_clr_bit(var->spi_cs);
            
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, false);   // Do not force the transfer (it occurs automatically when data is received - SPI Rx generates the transfer)
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true);    // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).            
            
            functionState = SM_WAIT_END_OF_DMA_TRANSMISSION;
            
            break;
            
        case SM_READ:
            
            var->read[var->__current_selected_device]._header = 0x0041 | (var->__p_device_addresses[var->__current_selected_device] << 1);
            var->dma_tx_params.src_start_addr = (void *) &var->read[var->__current_selected_device];
            var->dma_rx_params.dst_start_addr = (void *) &var->read[var->__current_selected_device];
            
            ports_clr_bit(var->spi_cs);
                        
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, false);   // Do not force the transfer (it occurs automatically when data is received - SPI Rx generates the transfer)
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true);    // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).
            
            functionState = SM_WAIT_END_OF_DMA_TRANSMISSION;
            
            break;
            
        case SM_WAIT_END_OF_DMA_TRANSMISSION:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE); 
            
                ports_set_bit(var->spi_cs);
                
                if (var->dma_tx_params.src_start_addr == (void *) &var->write[var->__current_selected_device])
                {
                    functionState = SM_READ;
                }
                else
                {                    
                    if (++var->__current_selected_device >= var->__number_of_device)
                    {
                        functionState = SM_FREE;
                    }
                    else
                    {
                        functionState = SM_SEND;
                    }
                }
            }
            
            break;
    }
    
    return functionState;
}

uint8_t e_mcp23s17_deamon(mcp23s17_params_t *var)
{
    uint8_t i, ret;
    
    if (!var->is_init_done)
    {
        
        ports_reset_pin_output(var->spi_cs);
        ports_set_bit(var->spi_cs);
        
        var->dma_tx_id = dma_get_free_channel();
        var->dma_rx_id = dma_get_free_channel();
        
        spi_init(   var->spi_id, 
                    NULL, 
                    IRQ_NONE, 
                    MCP23S17_FREQ, 
                    SPI_CONF_MSTEN | SPI_CONF_FRMPOL_LOW | SPI_CONF_SMP_MIDDLE | SPI_CONF_MODE8 | SPI_CONF_CKP_HIGH | SPI_CONF_CKE_OFF);
        
        dma_init(   var->dma_tx_id, 
                    NULL, 
                    DMA_CONT_PRIO_2, 
                    DMA_INT_NONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    spi_get_tx_irq(var->spi_id), 
                    0xff);
        
        dma_init(   var->dma_rx_id, 
                    NULL, 
                    DMA_CONT_PRIO_3, 
                    DMA_INT_BLOCK_TRANSFER_DONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    spi_get_rx_irq(var->spi_id), 
                    0xff);
        
        var->dma_tx_params.dst_start_addr = (void *) spi_get_tx_reg(var->spi_id);
        var->dma_rx_params.src_start_addr = (void *) spi_get_rx_reg(var->spi_id);
        
        for (i = 0 ; i < var->__number_of_device ; i++)
        {
            var->write[i].IOCON = MCP23S17_IOCON_ADDRESS_PINS_ENABLE;
        }
        
        var->is_init_done = true;
    }  
    
    ret = _mcp23s17_write_and_read(var);
    
    return ret;
}

