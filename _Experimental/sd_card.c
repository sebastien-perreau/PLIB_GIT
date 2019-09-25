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

static uint8_t sd_card_send_command(sd_card_params_t *var, SD_CARD_COMMAND_TYPE cde_type, uint32_t args, SD_CARD_RESPONSE_COMMAND ret, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
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
            
            var->response_command.is_response_returned = false;
            memset((void *) var->_p_ram_tx, 0xff, (6 + 8 + (ret & 0x0f)));
            var->_p_ram_tx[0] = cde_type;
            var->_p_ram_tx[1] = (args >> 24) & 0xff;
            var->_p_ram_tx[2] = (args >> 16) & 0xff;
            var->_p_ram_tx[3] = (args >> 8) & 0xff;
            var->_p_ram_tx[4] = (args >> 0) & 0xff;
            var->_p_ram_tx[5] = sd_card_crc7(&var->_p_ram_tx[0], 5);            
            
            var->dma_tx_params.src_size = (6 + 8 + (ret & 0x0f));
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;
            var->dma_rx_params.dst_start_addr = var->_p_ram_rx;
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, false);
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, true, false);
            functionState++;
            break;
            
        case SM_WAIT_FULL_RECEPTION:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE);                                               
                
                uint8_t i;
                for (i = 6 ; i < (6 + 8 + (ret & 0x0f)) ; i++)
                {
                    if (var->_p_ram_rx[i] != 0xff)
                    {
                        var->response_command.R1.value = var->_p_ram_rx[i];
                        var->response_command.is_response_returned = true;
                        if (ret == SD_CARD_RET_R3)
                        {
                            var->response_command.R3.value = (uint32_t) ((var->_p_ram_rx[i + 1] << 24) | (var->_p_ram_rx[i + 2] << 16) | (var->_p_ram_rx[i + 3] << 8) | (var->_p_ram_rx[i + 4] << 0));
                        }
                        else if (ret == SD_CARD_RET_R7)
                        {
                            var->response_command.R7.value = (uint32_t) ((var->_p_ram_rx[i + 1] << 24) | (var->_p_ram_rx[i + 2] << 16) | (var->_p_ram_rx[i + 3] << 8) | (var->_p_ram_rx[i + 4] << 0));
                        }
                        break;                        
                    }
                }
                
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

static uint8_t sd_card_read(sd_card_params_t *var, uint16_t length, uint16_t offset_p_dest, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
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
            memset((void *) &var->_p_ram_tx[offset_p_dest], 0xff, length);
            var->dma_rx_params.dst_start_addr = &var->_p_ram_rx[offset_p_dest];
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
        SM_CMD_58,
        SM_ACMD_41_PART1,
        SM_ACMD_41_PART2,    
        SM_CMD_58_SECOND,        
        SM_CMD_16,
        SM_CMD_59,
        SM_END_OF_INIT,
        SM_FAIL
    } functionState = 0;
    static uint64_t functionTick = 0;
    
    switch (functionState)
    {
        case SM_FREE:      
            
            spi_set_frequency(var->spi_id, SD_CARD_FREQ_INIT);
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
            
            if (!sd_card_read(var, 10, 0, SPI_CS_SET, SPI_CS_DO_NOTHING))
            {
                functionState = SM_CMD_0;
            }
            break;
            
        case SM_CMD_0:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_0, 0x00000000, SD_CARD_RET_R1, SPI_CS_CLR, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && var->response_command.R1.idle_state)                
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
            
            // Argument of CMD8: 0xaa = pattern to check in R7 response, 0x100 = host voltage accepted (2,7V-3,6V) which should be returned by R7 response.
            if (!sd_card_send_command(var, SD_CARD_CMD_8, 0x000001aa, SD_CARD_RET_R7, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && var->response_command.R1.idle_state)                 
                {                    
                    if (var->response_command.R1.illegal_command)
                    {
                        // Ver1.X SD Memory Card
                        var->card_version = SD_CARD_VER_1_X;
                        functionState = SM_CMD_58;
                    }
                    else
                    {                        
                        // Ver2.00 or later SD Memory Card
                        if (var->response_command.R7.check_pattern == 0xaa)
                        {
                            if (var->response_command.R7.voltage_accepted == 1)
                            {
                                var->card_version = SD_CARD_VER_2_X_SDSC;
                                functionState = SM_CMD_58;                                
                            }
                            else
                            {
                                functionState = SM_FAIL;
                            }
                        }
                        else
                        {
                            functionState = SM_FAIL;
                        }
                    }
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_58:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_58, 0x00000000, SD_CARD_RET_R3, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    if (var->response_command.R3.voltage_windows == 0x1ff)
                    {
                        functionState = SM_ACMD_41_PART1;
                        mUpdateTick(functionTick);
                    }
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_ACMD_41_PART1:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_55, 0x00000000, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    functionState = SM_ACMD_41_PART2;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_ACMD_41_PART2:
            
            // As argument to ACMD41 set HCS - High Capacity Support - to '1' (bit 30 - others bits are reserved) 
            if (!sd_card_send_command(var, SD_CARD_ACMD_41, (var->card_version == SD_CARD_VER_1_X) ? 0x00000000 : 0x40000000, SD_CARD_RET_R3, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    functionState = var->response_command.R1.idle_state ? SM_ACMD_41_PART1 : (var->card_version == SD_CARD_VER_1_X ? SM_CMD_16 : SM_CMD_58_SECOND);
                }
                else
                {
                    functionState = SM_FAIL;
                }
                
                if (mTickCompare(functionTick) >= TICK_1S)
                {
                    functionState = SM_FAIL;        // Timeout: SD Card stay in Idle mode
                }
            }
            break;
            
        case SM_CMD_58_SECOND:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_58, 0x00000000, SD_CARD_RET_R3, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {                
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    if (var->response_command.R3.busy)
                    {
                        if (var->response_command.R3.CCS)
                        {
                            var->card_version = SD_CARD_VER_2_X_SDHC;
                            functionState = SM_CMD_59;
                        }
                        else
                        {
                            var->card_version = SD_CARD_VER_2_X_SDSC;
                            functionState = SM_CMD_16;
                        }
                    }
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_16:
            
            // Set 512 bytes per sector (case if the SD card is Ver2.X SDSC - Standard Capacity or Ver1.X)
            if (!sd_card_send_command(var, SD_CARD_CMD_16, 0x00000200, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    functionState = SM_CMD_59;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_59:
            
            // Disable CRC
            if (!sd_card_send_command(var, SD_CARD_CMD_59, 0x00000000, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command)
                {
                    functionState = SM_END_OF_INIT;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_END_OF_INIT:
            
            functionState = SM_FREE;
            ports_set_bit(var->spi_cs);
            spi_set_frequency(var->spi_id, SD_CARD_FREQ);
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

static uint8_t sd_card_read_single_block(sd_card_params_t *var, uint32_t sector)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_CMD_17,
        SM_READ_DATA_PART_1,
        SM_READ_DATA_PART_2,
        SM_FAIL
    } functionState = 0;
    static uint64_t functionTick = 0;
    
    switch (functionState)
    {
        case SM_FREE:      
                  
            functionState = SM_CMD_17;
            
        case SM_CMD_17:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_17, sector * 512, SD_CARD_RET_R1, SPI_CS_CLR, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_ERRORS) && var->response_command.is_response_returned && !var->response_command.R1.illegal_command && !var->response_command.R1.idle_state)
                {
                    dma_set_channel_event_control(var->dma_tx_id, DMA_EVT_START_TRANSFER_ON_IRQ | DMA_EVT_ABORD_TRANSFER_ON_IRQ);
                    dma_set_channel_event_control(var->dma_rx_id, DMA_EVT_START_TRANSFER_ON_IRQ | DMA_EVT_ABORD_TRANSFER_ON_PATTERN_MATCH);
                    functionState = SM_READ_DATA_PART_1;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_READ_DATA_PART_1:
            
            if (!sd_card_read(var, 2048, 0, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                // Get the first byte of the data packet (just after Data Token) which has been "lost" during the abord transmission               
                var->_p_ram_rx[0] = var->_p_ram_rx[dma_get_index_cell_pointer(var->dma_rx_id)];
                
                dma_set_channel_event_control(var->dma_tx_id, DMA_EVT_START_TRANSFER_ON_IRQ);
                dma_set_channel_event_control(var->dma_rx_id, DMA_EVT_START_TRANSFER_ON_IRQ);
                functionState = SM_READ_DATA_PART_2;
            }
            break;
            
        case SM_READ_DATA_PART_2:
            
            // The data to read is as follow: Data packet = 512 bytes + Data CRC = 2 bytes
            // We read only 511 bytes because the abord transmission in SM_READ_DATA_PART_1 is occurred during the first byte of the Data packet
            if (!sd_card_read(var, 511 + 2, 1, SPI_CS_DO_NOTHING, SPI_CS_SET))
            {
                if ((var->_p_ram_rx[510] == 0x55) && (var->_p_ram_rx[511] == 0xaa))
                {
                    functionState = SM_FREE;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_FAIL:
            
            // Count fail and STOP everything if count_fail >= 10 -> functionState = 0 and SET_BIT(var->_flags, SM_SD_CARD_INITIALIZATION);
            break;
            
        default:
            
            break;
    }
    
    return functionState;
}

static void sd_card_get_master_boot_record(sd_card_params_t *var)
{
    uint8_t i;
    for (i = 0 ; i < 4 ; i++)
    {
        var->master_boot_record.partition_entry[i].boot_descriptor = var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 0];
        var->master_boot_record.partition_entry[i].first_partition_sector = (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 1] << 16) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 2] << 8) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 3] << 0);    
        var->master_boot_record.partition_entry[i].file_system_descriptor = var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 4];
        var->master_boot_record.partition_entry[i].last_partition_sector = (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 5] << 16) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 6] << 8) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 7] << 0);   
        var->master_boot_record.partition_entry[i].first_sector_of_the_partition = (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 8] << 24) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 9] << 16) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 10] << 8) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 11] << 0);    
        var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition = (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 12] << 24) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 13] << 16) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 14] << 8) | (var->_p_ram_rx[SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 15] << 0);    
        var->master_boot_record.partition_entry[i].is_existing = (var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition > 0) ? true : false;        
    }    
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
                    dma_get_irq(var->dma_rx_id));
        
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
                    break;
                }
            }
            break;
            
        case SM_SD_CARD_INITIALIZATION:
            
            if (!sd_card_initialization(var))
            {
                CLR_BIT(var->_flags, SM_SD_CARD_INITIALIZATION);
                SET_BIT(var->_flags, SM_SD_CARD_MASTER_BOOT_RECORD);
                SET_BIT(var->_flags, SM_SD_CARD_BOOT_SECTOR);
                SET_BIT(var->_flags, SM_SD_CARD_FAT1);
                SET_BIT(var->_flags, SM_SD_CARD_ROOT_DIRECTORY);
                SET_BIT(var->_flags, SM_SD_CARD_DATA_SPACE);
                var->_sm.index = SM_SD_CARD_HOME;   
            }
            break;
            
        case SM_SD_CARD_MASTER_BOOT_RECORD:
                    
            if (!sd_card_read_single_block(var, 0))
            {
                sd_card_get_master_boot_record(var);
                
                CLR_BIT(var->_flags, SM_SD_CARD_MASTER_BOOT_RECORD);
                var->_sm.index = SM_SD_CARD_HOME;
            }
            break;
            
        case SM_SD_CARD_BOOT_SECTOR:
               
            break;
                        
    }
}

