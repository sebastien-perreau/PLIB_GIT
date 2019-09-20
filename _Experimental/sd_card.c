 /*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		19/09/2019		- Initial release
* 
*   Description:
*   ------------ 
*********************************************************************/

#include "../PLIB.h"

static uint8_t sd_card_crc7(uint8_t *buffer, uint8_t length)
{
    uint8_t crc = 0, data, i;
    
    for (i = 0 ; i < length ; i++)
    {
        data = buffer[i] ^ (crc << 1);
        if (data & 0x80)
        {
            data ^= 9;
        }
        crc = data ^ (crc & 0x78) ^ (crc << 4) ^ ((crc >> 3) & 0x0f);
    }
    crc = (crc << 1) ^ (crc << 4) ^ (crc & 0x70) ^ ((crc >> 3) & 0x0f);

	return (crc | 0x01);
}

static uint8_t sd_card_send_command(sd_card_params_t *var, SD_CARD_COMMAND_TYPE cde_type, uint32_t args, SD_CARD_COMMAND_RETURN ret, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_WAIT_FULL_RECEPTION
    } functionState = 0;    
    
    switch (functionState)
    {
        case SM_FREE:
            
            if (cs_at_begining_of_transmission == SPI_CS_SET)
            {
                ports_set_bit(var->spi_cs);
            }
            else if (cs_at_begining_of_transmission == SPI_CS_CLR)
            {
                ports_clr_bit(var->spi_cs);
            }
            
            memset((void *) var->dma_tx_params.src_start_addr, 0xff, (8 + ret));
            {
                uint8_t *_p_tx = (uint8_t *) var->dma_tx_params.src_start_addr;
                _p_tx[1] = cde_type;
                _p_tx[2] = (args >> 24) & 0xff;
                _p_tx[3] = (args >> 16) & 0xff;
                _p_tx[4] = (args >> 8) & 0xff;
                _p_tx[5] = (args >> 0) & 0xff;
                _p_tx[6] = sd_card_crc7(&_p_tx[1], 5);
            }
            
            var->dma_tx_params.src_size = (8 + ret);
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, false);
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, true, false);
            functionState++;
            break;
            
        case SM_WAIT_FULL_RECEPTION:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE);
                if (cs_at_end_of_transmission == SPI_CS_SET)
                {
                    ports_set_bit(var->spi_cs);
                }
                else if (cs_at_end_of_transmission == SPI_CS_CLR)
                {
                    ports_clr_bit(var->spi_cs);
                }
                functionState = SM_FREE;
            }
            break;
    }
    
    return functionState;
}

static uint8_t sd_card_read(sd_card_params_t *var, uint16_t length, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_WAIT_FULL_RECEPTION
    } functionState = 0;
    
    switch (functionState)
    {
        case SM_FREE:
            
            if (cs_at_begining_of_transmission == SPI_CS_SET)
            {
                ports_set_bit(var->spi_cs);
            }
            else if (cs_at_begining_of_transmission == SPI_CS_CLR)
            {
                ports_clr_bit(var->spi_cs);
            }
            memset((void *) var->dma_tx_params.src_start_addr, 0xff, length);
            var->dma_tx_params.src_size = length;
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, false);
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, true, false);
            functionState++;
            break;
            
        case SM_WAIT_FULL_RECEPTION:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE);
                if (cs_at_end_of_transmission == SPI_CS_SET)
                {
                    ports_set_bit(var->spi_cs);
                }
                else if (cs_at_end_of_transmission == SPI_CS_CLR)
                {
                    ports_clr_bit(var->spi_cs);
                }
                functionState = SM_FREE;
            }
            break;
    }
    
    return functionState;
}

static uint8_t sd_card_initialization(sd_card_params_t *var)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_POWER_SEQUENCE_START,
        SM_POWER_SEQUENCE_WAIT,
        SM_POWER_SEQUENCE_DUMMY_CLOCK,
        SM_CMD_0,
        SM_CMD_8,
        SM_ACMD_41_PART1,
        SM_ACMD_41_PART2,
        SM_CMD_1,
        SM_CMD_58,
        SM_CMD_16,
        SM_FAIL
    } functionState = 0;
    static uint64_t functionTick = 0;
    uint8_t *_p_rx = (uint8_t *) var->dma_rx_params.dst_start_addr;
    
    switch (functionState)
    {
        case SM_FREE:      
            
            functionState = SM_POWER_SEQUENCE_START;        
            
        case SM_POWER_SEQUENCE_START:
            
            ports_clr_bit(var->spi_cs);
            functionState = SM_POWER_SEQUENCE_WAIT;
            mUpdateTick(functionTick);        
            break;
            
        case SM_POWER_SEQUENCE_WAIT:
            
            if (mTickCompare(functionTick) >= TICK_10MS)
            {
                mUpdateTick(functionTick);
                functionState = SM_POWER_SEQUENCE_DUMMY_CLOCK;                
            }
            break;
            
        case SM_POWER_SEQUENCE_DUMMY_CLOCK:
            
            if (!sd_card_read(var, 10, SPI_CS_SET, SPI_CS_DO_NOTHING))
            {
                functionState = SM_CMD_0;
            }
            break;
            
        case SM_CMD_0:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_0, 0x00000000, SD_CARD_RET_R1, SPI_CS_CLR, SPI_CS_DO_NOTHING))
            {
                if (!(_p_rx[8] & 0xfe))
                {
                    functionState = SM_CMD_8;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_8:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_8, 0x000001aa, SD_CARD_RET_R3_R7, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(_p_rx[8] & 0xfe))
                {
                    uint32_t vR7 = (_p_rx[11] << 8) | (_p_rx[12] << 0);
                    if (vR7 != 0x1aa)
                    {
                        functionState = SM_FAIL;    // Error voltage range
                    }
                    else
                    {
                        functionState = SM_ACMD_41_PART1;
                        mUpdateTick(functionTick);
                    }
                }
                else
                {
                    functionState = SM_FAIL;        // R1 Response error or illegal command
                }
            }
            break;
            
        case SM_ACMD_41_PART1:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_55, 0x00000000, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(_p_rx[8] & 0xfe))
                {
                    functionState = SM_ACMD_41_PART2;
                }
                else
                {
                    functionState = SM_FAIL;        // R1 Response error or illegal command
                }
            }
            break;
            
        case SM_ACMD_41_PART2:
            
            // As argument to ACMD41 set HCS - High Capacity Support - to '1' (bit 30 - others bits are reserved) 
            if (!sd_card_send_command(var, SD_CARD_ACMD_41, 0x40000000, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(_p_rx[8] & 0xfe))
                {
                    functionState = _p_rx[8] ? SM_ACMD_41_PART1 : SM_CMD_58;
                }
                else
                {
                    functionState = SM_FAIL;        // R1 Response error or illegal command
                }
                
                if (mTickCompare(functionTick) >= TICK_1S)
                {
                    functionState = SM_FAIL;        // Timeout: Do not leave Idle mode
                }
            }
            break;
            
        case SM_CMD_58:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_58, 0x00000000, SD_CARD_RET_R3_R7, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!_p_rx[8])
                {
                    uint32_t vR3 = (_p_rx[9] << 24) | (_p_rx[10] << 16) | (_p_rx[11] << 8) | (_p_rx[12] << 0);
                    if (GET_BIT(vR3, 31))           // Card power up status bit (BUSY) ('1' power up routine finished / '0' power up routine not finished)
                    {
                        if (GET_BIT(vR3, 30))       // Card Capacity Status (CCS) (This bit is valid only when the card power up status bit is set)
                        {
                            functionState = SM_FREE;
                            ports_set_bit(var->spi_cs);
                        }
                        else
                        {
                            functionState = SM_CMD_16;
                        }
                    }               
                }
                else
                {
                    functionState = SM_FAIL;        // R1 Response error or illegal command
                }
            }
            break;
            
        case SM_CMD_16:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_16, 0x000000200, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!_p_rx[8])
                {
                    functionState = SM_FREE;
                    ports_set_bit(var->spi_cs);
                }
                else
                {
                    functionState = SM_FAIL;        // R1 Response error or illegal command
                }
            }
            break;
            
        case SM_FAIL:
            
            ports_set_bit(var->spi_cs);
            mUpdateTick(functionTick);
            functionState++;
            break;
            
        default:
            
            if (mTickCompare(functionTick) >= TICK_1S)
            {
                functionState = SM_POWER_SEQUENCE_START;
            }
            break;
    }
    
    return functionState;
}

void sd_card_deamon(sd_card_params_t *var)
{
    static uint8_t i = 0;
    
    if (!var->is_init_done)
    {
    
        ports_reset_pin_output(var->spi_cs);
        ports_set_bit(var->spi_cs);
        
        var->dma_tx_id = dma_get_free_channel();
        var->dma_rx_id = dma_get_free_channel();
        
        spi_init(   var->spi_id, 
                    NULL, 
                    IRQ_NONE, 
                    SD_CARD_FREQ_INIT, 
                    SPI_CONF_MSTEN | SPI_CONF_FRMPOL_LOW | SPI_CONF_SMP_MIDDLE | SPI_CONF_MODE8 | SPI_CONF_CKP_HIGH | SPI_CONF_CKE_OFF);
                
        dma_init(   var->dma_tx_id, 
                    NULL, 
                    DMA_CONT_PRIO_3, 
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
        
        SET_BIT(var->_flags, SM_SD_CARD_INITIALIZATION);        
        
        var->is_init_done = true;
    }
    
    switch (var->_sm.index)
    {
        case SM_SD_CARD_HOME:
            
            for (i = 1 ; i < SM_SD_CARD_MAX_FLAGS ; i++)
            {
                if (GET_BIT(var->_flags, i))
                {
                    var->_sm.index = i;
                    if (i == SM_SD_CARD_INITIALIZATION)
                    {
                        spi_set_frequency(var->spi_id, SD_CARD_FREQ_INIT);
                    }
                    break;
                }
            }
            break;
            
        case SM_SD_CARD_INITIALIZATION:
            
            if (!sd_card_initialization(var))
            {
                CLR_BIT(var->_flags, SM_SD_CARD_INITIALIZATION);
                SET_BIT(var->_flags, SM_SD_CARD_READ_SECTOR_0);
                
                spi_set_frequency(var->spi_id, SD_CARD_FREQ);
                var->_sm.index = SM_SD_CARD_READ_SECTOR_0;
            }
            break;
            
        case SM_SD_CARD_READ_SECTOR_0:
            
            break;
                        
    }
}

