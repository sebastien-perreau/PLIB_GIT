 /*********************************************************************
  * Author : Sébastien PERREAU 
  * Creation Date:  19/09/2019
  
  * General overview: 
  * -----------------
  * SD Card driver uses 1 SPI and 2 DMA modules and is compatible ONLY with FAT16 CHS/LBA mode and
  * FAT32 CHS/LBA mode.
  * READ requests are implemented.
  * (WRITE requests are not implemented.)
  * A maximum of 120 files can be opened at same time. 
  * The maximum SPI frequency is 10 MHz (in theory 25 MHz). 
  * The card detection is implemented in the communication (no need to have a 
  * CD signal). If a card is removed then the software re-launch the initialization
  * sequence (up to SUCCESS). 
  * A flag is used (in the SD_CARD_DEF) to enable/disable the LOG (need to activate the 
  * LOG driver...).
  * File name (including path) can not exceed 255 bytes length. 
  * One cluster = 1 / 2 / 4 / 8 / 16 / 32 / 64 / 128 sectors (this size is defined in the Boot Sector of the Volume).
  * One sector = 512 / 1024 / 2048 / 4096 bytes (A lot of code are assuming 512 bytes per sectors)
  * 
  * A FAT16 partition may not have less than 4085 clusters or more than 65524 clusters (maximum SD Card
  * size is 4 Go - minimum is 2 Mo).
  * 
  * A FAT32 partition uses a 32-bit number to identify each cluster, and this gives it a total of 
  * 4.294.967.296 clusters.
  * 
  * The two informations below are localized in the Partition Entry:
  * The CHS (Cylinder / Head / Sector) address mode has a limit of 1024(C) x 252(H) x 64(S) = 16.515.072 sectors 
  * (16.515.072 x 512 bytes = 8 Go Max).
  * The LBA address mode has a limit of 2^32 = 4.294.967.296 sectors (4.294.967.296 x 512 bytes = 2 To Max). 
  
  * Memory Description of a SD CARD formated in a FAT File System:
  * -------------------------------------------------------------
  *     FAT16               FAT32
  *  ---------------     ---------------    --> Sector 0: Master Boot Record (sd card boot sector)  !! This is sector 0 in LBA mode, and sector 0:0:1 in CHS mode. !!
  * | MASTER BOOT   |   | MASTER BOOT   |   MBR is common to FAT16 and FAT32
  * | RECORD        |   | RECORD        |
  *  ---------------     --------------- 
  * |   ...         |   |   ...         |
  *  ---------------     ---------------    --> Boot sector of a partition (up to 4 - possible - partitions)
  * | BOOT SECTOR   |   | BOOT SECTOR   |
  * |   FAT16       |   |   FAT32       |
  *  ---------------     ---------------
  * |   ...         |   |   ...         |   reserved sector (can include the FSInfo - File System Information - in this area)
  *  ---------------     ---------------    --> FAT 1 starts at BOOT SECTOR + Number of reserved sector
  * | FAT 1         |   | FAT 1         |   File Allocation Table is Number of FAT * Number of sectors per FAT (FAT for FAT16 is 16-bit length & FAT for FAT32 is 32-bit length)
  *  ---------------     ---------------
  * | FAT N         |   | FAT N         |
  *  ---------------     ---------------    --> ROOT Dir. (ONLY FAT16) starts at FAT 1 SECTOR + Size of FATs
  * |               |   |               |
  * | ROOT DIR.     |   | DATA SPACE    |   Root Directory (ONLY FAT16) is (Number of possible ROOT Dir. * 32 / Number of bytes per sector) length (length = number of sectors)
  * |               |   |   files...    |
  *  ---------------    |   folders...  |   --> DATA SPACE always starts at CLUSTER 2 (ROOT Dir. SECTOR + Size of ROOT Dir in case of FAT16)
  * |               |   |               |
  * | DATA SPACE    |   |       &       |
  * |   files...    |   |               |
  * |   folders...  |   |   ROOT DIR.   |
  * |               |   |               |
  *  ---------------     ---------------
  * 
*********************************************************************/

#include "../PLIB.h"

#define sd_card_get_cid(var)                    sd_card_get_packet(var, SD_CARD_CMD_10, 0x00000000, SD_CARD_RET_R1, SD_CARD_CID_LENGTH)
#define sd_card_get_csd(var)                    sd_card_get_packet(var, SD_CARD_CMD_9, 0x00000000, SD_CARD_RET_R1, SD_CARD_CSD_LENGTH)
#define sd_card_read_single_block(var, sector)  sd_card_get_packet(var, SD_CARD_CMD_17, (var->args_type == SD_CARD_ARGS_TYPE_SECTOR) ? sector : (sector * 512), SD_CARD_RET_R1, SD_CARD_DATA_BLOCK_LENGTH)

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

static uint8_t _get_last_entry_name(uint8_t *p_lfn_number_of_entry, uint8_t *p_lfn_buffer, uint8_t *p_ram_rx_of_last_entry, char *p_last_entry_name)
{
    uint8_t i, j, k;
    uint8_t name_length = 0;
    static const uint8_t lfn_entry_offset[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
    
    memset(p_last_entry_name, 0, 255);
                    
    if (*p_lfn_number_of_entry > 0)
    {    
        for (i = 1, k = 0 ; i <= *p_lfn_number_of_entry ; i++)
        {
            for (j = 0 ; j < 13 ; j++)
            {
                uint8_t character = p_lfn_buffer[*p_lfn_number_of_entry * 32 - i * 32 + lfn_entry_offset[j]];
                if (character == 0)
                {
                    break;
                }
                else
                {
                    p_last_entry_name[k++] = character;
                }
            }                                
        }
        name_length = k;
        memset(p_lfn_buffer, 0, 32*19);
        *p_lfn_number_of_entry = 0;   
    }
    else
    {
        //  Get Name without space between 'name' and 'extension' and remove space in extension if extension_length < 3 chars (example: "MOVIE   IO " becomes "MOVIE.IO"
        //  Set all characters to lower case "MOVIE.IO" becomes "movie.io"
        for (i = 7 ; i > 0 ; i--)
        {
            if (p_ram_rx_of_last_entry[i] != ' ')
            {
                break;
            }
        }
        for (j = 0 ; j <= i ; j++)
        {
            p_last_entry_name[j] = p_ram_rx_of_last_entry[j];
            p_last_entry_name[j] = ((p_last_entry_name[j] >= 65) && (p_last_entry_name[j] <= 90)) ? (p_last_entry_name[j] + 32) : p_last_entry_name[j];
        }
        p_last_entry_name[j] = '.';
        for (i = 0 ; i < 3 ; i++)
        {
            if (p_ram_rx_of_last_entry[8 + i] != ' ')
            {
                p_last_entry_name[++j] = p_ram_rx_of_last_entry[8 + i];
                p_last_entry_name[j] = ((p_last_entry_name[j] >= 65) && (p_last_entry_name[j] <= 90)) ? (p_last_entry_name[j] + 32) : p_last_entry_name[j];
                name_length = j + 1;
            }
            else
            {
                break;
            }
        }
    }
    
    return name_length;
}

static void _sort_data_array_to_cid_structure(sd_card_params_t *var)
{
    var->cid.manufacturer_id = var->_p_ram_rx[0];
    var->cid.oem_id[0] = var->_p_ram_rx[1];
    var->cid.oem_id[1] = var->_p_ram_rx[2];
    var->cid.product_name[0] = var->_p_ram_rx[3];
    var->cid.product_name[1] = var->_p_ram_rx[4];
    var->cid.product_name[2] = var->_p_ram_rx[5];
    var->cid.product_name[3] = var->_p_ram_rx[6];
    var->cid.product_name[4] = var->_p_ram_rx[7];
    var->cid.product_revision = var->_p_ram_rx[8];
    var->cid.serial_number = (uint32_t) ((var->_p_ram_rx[9] << 24) | (var->_p_ram_rx[10] << 16) | (var->_p_ram_rx[11] << 8) | (var->_p_ram_rx[12] << 0));
    var->cid.manufacturer_data_code = (uint16_t) ((var->_p_ram_rx[13] << 8) | (var->_p_ram_rx[14] << 0)) & 0x0fff;
    var->cid.crc = (var->_p_ram_rx[15] >> 1) & 0x7f;
    
    if (var->is_log_enable) 
    { 
        LOG_BLANCK("\nSD Card get Identifications (CID):\n    Manufacturer ID: %2x", var->cid.manufacturer_id); 
        LOG_BLANCK("    OEM ID: %s", p_string(var->cid.oem_id)); 
        LOG_BLANCK("    Product Name: %s", p_string(var->cid.product_name)); 
        LOG_BLANCK("    Product Revision: %2d.%2d", ((var->cid.product_revision >> 4) & 0x0f), ((var->cid.product_revision >> 0) & 0x0f));
        LOG_BLANCK("    Serial Number: %8x", var->cid.serial_number); 
        LOG_BLANCK("    Manufacturer Data Code: %2d/%4d", (var->cid.manufacturer_data_code >> 0)&0x0f, (((var->cid.manufacturer_data_code >> 4) & 0xff) + 2000)); 
    }
}

static void _sort_data_array_to_csd_structure(sd_card_params_t *var)
{
    var->csd.csd_structure = (var->_p_ram_rx[0] >> 6) & 0x03;
    var->csd.taac = var->_p_ram_rx[1];
    var->csd.nsac = var->_p_ram_rx[2];
    var->csd.transfer_rate = var->_p_ram_rx[3];
    var->csd.command_classes = (var->_p_ram_rx[4] << 4) | ((var->_p_ram_rx[5] >> 4) & 0x0f);
    var->csd.max_read_data_block_length = (var->_p_ram_rx[5] & 0x0f);  
    var->csd.read_block_partial = GET_BIT(var->_p_ram_rx[6], 7);
    var->csd.write_block_misalignement = GET_BIT(var->_p_ram_rx[6], 6);
    var->csd.read_block_misalignement = GET_BIT(var->_p_ram_rx[6], 5);
    var->csd.dsr_implemented = GET_BIT(var->_p_ram_rx[6], 4);
    var->csd.device_size = ((var->_p_ram_rx[6] & 0x03) << 10) | (var->_p_ram_rx[7] << 2) | ((var->_p_ram_rx[8] >> 6) & 0x03);
    var->csd.max_read_current_at_vdd_min = ((var->_p_ram_rx[8] >> 3) & 0x07);
    var->csd.max_read_current_at_vdd_max = ((var->_p_ram_rx[8] >> 0) & 0x07);
    var->csd.max_write_current_at_vdd_min = ((var->_p_ram_rx[9] >> 5) & 0x07);
    var->csd.max_write_current_at_vdd_max = ((var->_p_ram_rx[9] >> 2) & 0x07);    
    var->csd.device_size_mult = ((var->_p_ram_rx[9] & 0x03) << 1) | ((var->_p_ram_rx[10] & 0x80) >> 7);    
    var->csd.erase_single_block_enable = GET_BIT(var->_p_ram_rx[10], 6);
    var->csd.erase_sector_size = ((var->_p_ram_rx[10] & 0x3f) << 1) | ((var->_p_ram_rx[11] & 0x80) >> 7);
    var->csd.write_protect_group_size = (var->_p_ram_rx[11] & 0x7f);
    var->csd.write_protect_group_enable = GET_BIT(var->_p_ram_rx[12], 7);
    var->csd.write_speed_factor = ((var->_p_ram_rx[12] >> 2) & 0x07);    
    var->csd.max_write_data_block_length = ((var->_p_ram_rx[12] & 0x03) << 2) | ((var->_p_ram_rx[13] & 0xc0) >> 6);    
    var->csd.write_partial_blocks_enable = GET_BIT(var->_p_ram_rx[13], 5);
    var->csd.file_format_group = GET_BIT(var->_p_ram_rx[14], 7);
    var->csd.copy_flag = GET_BIT(var->_p_ram_rx[14], 6);
    var->csd.permanent_write_protection = GET_BIT(var->_p_ram_rx[14], 5);
    var->csd.temporary_write_protection = GET_BIT(var->_p_ram_rx[14], 4);
    var->csd.file_format = (var->_p_ram_rx[14] >> 2) & 0x03;
    var->csd.crc = (var->_p_ram_rx[15] >> 1) & 0x7f;

    if (var->is_log_enable) 
    { 
        uint32_t capacity = (var->csd.device_size + 1) * (pow(2, var->csd.device_size_mult + 2)) * pow(2, var->csd.max_read_data_block_length) / (1024 * 1024);
        
        LOG_BLANCK("\nSD Card get Specific Data (CSD):\n    TAAC (time unit = %d / time value = %d) - NSAC (%d)", (var->csd.taac & 0x03), ((var->csd.taac >> 3) & 0x0f), var->csd.nsac);        
        LOG_BLANCK("    Transfer Rate: time unit = %d / time value = %d (always tu=2 and tv = 6 to give 25MHz)", (var->csd.transfer_rate & 0x03), ((var->csd.transfer_rate >> 3) & 0x0f));
        LOG_BLANCK("    Command Classes (CCC): %12b", var->csd.command_classes);
        LOG_BLANCK("    Maximum Read Data Block Length: %d bytes", pow(2, var->csd.max_read_data_block_length));
        LOG_BLANCK("    Maximum Write Data Block Length: %d bytes", pow(2, var->csd.max_write_data_block_length));
        if (!capacity)
        {
            // Impossible to have more than 4 GB in the CSD structure (maximum = 4096 (device size + 1) x 512 (MULT) x 2048 (BLOCK LEN) = 4 GB)
            LOG_BLANCK("    Memory Capacity (without protected security area): > 4 GB", capacity);
            var->args_type = SD_CARD_ARGS_TYPE_SECTOR;
        }
        else
        {
            LOG_BLANCK("    Memory Capacity (without protected security area): %d MB", capacity);
            var->args_type = SD_CARD_ARGS_TYPE_ADDRESS;
        }
        LOG_BLANCK("    Max. Read / Write Current @Vdd min: %d / %d and @Vdd max: %d / %d", var->csd.max_read_current_at_vdd_min, var->csd.max_write_current_at_vdd_min, var->csd.max_read_current_at_vdd_max, var->csd.max_write_current_at_vdd_max);        
        LOG_BLANCK("    Erase Single Block Enable: %d - Erase Sector Size: %d write blocks", var->csd.erase_single_block_enable, (var->csd.erase_sector_size + 1));        
        LOG_BLANCK("    File Format Group: %d / File Format: %d", var->csd.file_format_group, var->csd.file_format);
        
        if (var->args_type == SD_CARD_ARGS_TYPE_ADDRESS)
        {
            LOG_BLANCK("    SD CARD ARGS type: ADDRESS (sector x 512)");
        }
        else
        {
            LOG_BLANCK("    SD CARD ARGS type: SECTOR");
        }
    }
}

static void _sort_data_array_to_master_boot_record_structure(sd_card_params_t *var)
{
    uint16_t end_of_data_block = (var->_p_ram_rx[510] << 0) | (var->_p_ram_rx[511] << 8);                        
    if (end_of_data_block == SD_CARD_END_OF_DATA_BLOCK)
    {
        uint8_t i;
        for (i = 0 ; i < 4 ; i++)
        {
            var->master_boot_record.partition_entry[i]._is_fat_32_partition = false;
            var->master_boot_record.partition_entry[i].boot_descriptor = var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 0];
            var->master_boot_record.partition_entry[i].start_partition_head = var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 1];
            var->master_boot_record.partition_entry[i].start_partition_cyl_sec.value = (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 2] << 0) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 3] << 8);
            var->master_boot_record.partition_entry[i].file_system_descriptor = var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 4];
            var->master_boot_record.partition_entry[i].last_partition_head = var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 5];
            var->master_boot_record.partition_entry[i].last_partition_cyl_sec.value = (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 6] << 0) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 7] << 8);
            var->master_boot_record.partition_entry[i].first_sector_of_the_partition = (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 8] << 0) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 9] << 8) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 10] << 16) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 11] << 24);    
            var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition = (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 12] << 0) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 13] << 8) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 14] << 16) | (var->_p_ram_rx[FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET + i*16 + 15] << 24);    
            
            if (var->is_log_enable)
            {
                if (var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition > 0)
                {
                    LOG_BLANCK("\nPartition %d entry (%d MB):", i, (var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition / 1024 * 512 / 1024));
                    LOG_BLANCK("    Boot descriptor: %2x", var->master_boot_record.partition_entry[i].boot_descriptor);
                    switch (var->master_boot_record.partition_entry[i].file_system_descriptor)
                    {
                        case FAT_FILE_SYSTEM_DESCRIPTOR_UNKNOWN:
                            LOG_BLANCK("    File System Descriptor: Unknown or nothing");
                            break;
                            
                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT12:
                            LOG_BLANCK("    File System Descriptor: FAT12");
                            break;
                            
                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_LOW:
                            LOG_BLANCK("    File System Descriptor: FAT16 CHS mode with partition < 32M");
                            break;
                            
                        case FAT_FILE_SYSTEM_DESCRIPTOR_EXTENDED_MSDOS_CHS:
                            LOG_BLANCK("    File System Descriptor: Extended MS-DOS partition CHS mode");
                            break;

                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_HIGH:
                            LOG_BLANCK("    File System Descriptor: FAT16 CHS mode with partition >= 32M");
                            break;

                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_CHS:
                            LOG_BLANCK("    File System Descriptor: FAT32 CHS mode");
                            break;

                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_LBA:
                            LOG_BLANCK("    File System Descriptor: FAT32 LBA mode");
                            break;

                        case FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_LBA:
                            LOG_BLANCK("    File System Descriptor: FAT16 LBA mode");
                            break;
                            
                        case FAT_FILE_SYSTEM_DESCRIPTOR_EXTENDED_MSDOS_LBA:
                            LOG_BLANCK("    File System Descriptor: Extended MS-DOS partition LBA mode");
                            break;

                        default:
                            LOG_BLANCK("    File System Descriptor: %2x", var->master_boot_record.partition_entry[i].file_system_descriptor);
                            break;
                    }
                    LOG_BLANCK("    Start Partition CHS (cylinders: %d / head: %d / sectors: %d)", var->master_boot_record.partition_entry[i].start_partition_cyl_sec.cylinders, var->master_boot_record.partition_entry[i].start_partition_head, var->master_boot_record.partition_entry[i].start_partition_cyl_sec.sectors);
                    LOG_BLANCK("    Last Partition CHS (cylinders: %d / head: %d / sectors: %d)", var->master_boot_record.partition_entry[i].last_partition_cyl_sec.cylinders, var->master_boot_record.partition_entry[i].last_partition_head, var->master_boot_record.partition_entry[i].last_partition_cyl_sec.sectors);
                    LOG_BLANCK("    First Sector Of The Partition (Boot Sector): %d", var->master_boot_record.partition_entry[i].first_sector_of_the_partition);
                    LOG_BLANCK("    Number Of Sector In The Partition: %d", var->master_boot_record.partition_entry[i].number_of_sector_in_the_partition);                    
                }
                else
                {
                    LOG_BLANCK("Partition %d entry not existing...", i);
                }
            }
        }
        
        if (var->master_boot_record.partition_entry[0].number_of_sector_in_the_partition > 0)
        {
            if (    (var->master_boot_record.partition_entry[0].file_system_descriptor != FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_LOW) &&
                    (var->master_boot_record.partition_entry[0].file_system_descriptor != FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_HIGH) &&
                    (var->master_boot_record.partition_entry[0].file_system_descriptor != FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_LBA) &&
                    (var->master_boot_record.partition_entry[0].file_system_descriptor != FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_CHS) &&
                    (var->master_boot_record.partition_entry[0].file_system_descriptor != FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_LBA))
            {
                if (var->is_log_enable)
                {
                    LOG_BLANCK("FAT File System Driver not compatible (not FAT 16 nor FAT32 CHS mode)");
                }
            }
        }
        
    }
    else
    {
        if (var->is_log_enable)
        {
            LOG_BLANCK("\nRead Master Boot Record Fail (0xAA55 mismatch)");
        }
    }
}

static void _sort_data_array_to_boot_sector_structure(sd_card_params_t *var)
{
    uint16_t end_of_data_block = (var->_p_ram_rx[510] << 0) | (var->_p_ram_rx[511] << 8);                        
    if (end_of_data_block == SD_CARD_END_OF_DATA_BLOCK)
    {
        if (    (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_CHS) ||
                (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_LBA))
        {
            var->master_boot_record.partition_entry[0]._is_fat_32_partition = true;
            var->boot_sector.jump_boot_code = (var->_p_ram_rx[0] << 0) | (var->_p_ram_rx[1] << 8) | (var->_p_ram_rx[2] << 16);
            memcpy((void *) &var->boot_sector.oem_name, &var->_p_ram_rx[3], 8);
            var->boot_sector.number_of_bytes_per_sector = (var->_p_ram_rx[11] << 0) | (var->_p_ram_rx[12] << 8);
            var->boot_sector.number_of_sectors_per_cluster = var->_p_ram_rx[13];
            var->boot_sector.number_of_reserved_sectors = (var->_p_ram_rx[14] << 0) | (var->_p_ram_rx[15] << 8);
            var->boot_sector.number_of_file_allocation_tables = var->_p_ram_rx[16];            
            var->boot_sector.media_descriptor = var->_p_ram_rx[21];            
            var->boot_sector.number_of_sectors_per_track = (var->_p_ram_rx[24] << 0) | (var->_p_ram_rx[25] << 8);
            var->boot_sector.number_of_heads = (var->_p_ram_rx[26] << 0) | (var->_p_ram_rx[27] << 8);
            var->boot_sector.number_of_hidden_sectors = (var->_p_ram_rx[28] << 0) | (var->_p_ram_rx[29] << 8) | (var->_p_ram_rx[30] << 16) | (var->_p_ram_rx[31] << 24);
            var->boot_sector.number_of_sectors_in_the_partition = (var->_p_ram_rx[32] << 0) | (var->_p_ram_rx[33] << 8) | (var->_p_ram_rx[34] << 16) | (var->_p_ram_rx[35] << 24);            
            var->boot_sector.number_of_sectors_per_fat = (var->_p_ram_rx[36] << 0) | (var->_p_ram_rx[37] << 8) | (var->_p_ram_rx[38] << 16) | (var->_p_ram_rx[39] << 24);
            var->boot_sector.fat_handling_flags.value = (var->_p_ram_rx[40] << 0) | (var->_p_ram_rx[41] << 8);
            var->boot_sector.fat32_version.value = (var->_p_ram_rx[42] << 0) | (var->_p_ram_rx[43] << 8);
            var->boot_sector.cluster_number_of_root_directory_table = (var->_p_ram_rx[44] << 0) | (var->_p_ram_rx[45] << 8) | (var->_p_ram_rx[46] << 16) | (var->_p_ram_rx[47] << 24);
            var->boot_sector.file_system_information_region_start = (var->_p_ram_rx[48] << 0) | (var->_p_ram_rx[49] << 8);
            var->boot_sector.backup_boot_record_region_start = (var->_p_ram_rx[50] << 0) | (var->_p_ram_rx[51] << 8);            
            var->boot_sector.physical_drive_number = var->_p_ram_rx[64];
            var->boot_sector.current_head = var->_p_ram_rx[65];
            var->boot_sector.boot_signature = var->_p_ram_rx[66];
            var->boot_sector.volume_id = (var->_p_ram_rx[67] << 0) | (var->_p_ram_rx[68] << 8) | (var->_p_ram_rx[69] << 16) | (var->_p_ram_rx[70] << 24);
            memcpy((void *) &var->boot_sector.volume_label, &var->_p_ram_rx[71], 11);
            memcpy((void *) &var->boot_sector.file_system_type, &var->_p_ram_rx[82], 8);
            
            var->boot_sector.reserved_region_start = var->master_boot_record.partition_entry[0].first_sector_of_the_partition;
            var->boot_sector.fat_region_start = var->boot_sector.reserved_region_start + var->boot_sector.number_of_reserved_sectors;
            var->boot_sector.data_space_region_start = var->boot_sector.fat_region_start + (var->boot_sector.number_of_file_allocation_tables * var->boot_sector.number_of_sectors_per_fat);
            var->boot_sector.root_directory_region_start = fat_file_system_get_first_sector_of_cluster_N(var->boot_sector.cluster_number_of_root_directory_table);
            var->boot_sector.file_system_information_region_start += var->boot_sector.reserved_region_start;
            var->boot_sector.backup_boot_record_region_start += var->boot_sector.reserved_region_start;
            var->boot_sector.backup_file_system_information_region_start = var->boot_sector.backup_boot_record_region_start + var->boot_sector.file_system_information_region_start - var->boot_sector.reserved_region_start;
                    
            if (var->is_log_enable)
            {
                LOG_BLANCK("\nBoot Sector Partition 0 (FAT32):");
                LOG_BLANCK("        Boot region sector: %d", var->boot_sector.reserved_region_start);
                LOG_BLANCK("            FSInfo sector: %d", var->boot_sector.file_system_information_region_start);
                LOG_BLANCK("            Backup Boot Record sector: %d", var->boot_sector.backup_boot_record_region_start);
                LOG_BLANCK("            Backup FSInfo sector: %d", var->boot_sector.backup_file_system_information_region_start);
                LOG_BLANCK("        FAT region sector: %d", var->boot_sector.fat_region_start);                
                LOG_BLANCK("        Data Space region sector: %d (start at Cluster 2)", var->boot_sector.data_space_region_start);
                LOG_BLANCK("        Root Directory region sector: %d", var->boot_sector.root_directory_region_start);
                LOG_BLANCK("    Capacity Of The Partition: %d MB", var->boot_sector.number_of_sectors_in_the_partition/1024*var->boot_sector.number_of_bytes_per_sector/1024);
                LOG_BLANCK("    Jump Command: %6x", var->boot_sector.jump_boot_code);
                LOG_BLANCK("    %d Sectors in the partition - %d Bytes Per Sector - %d Sectors Per Cluster - %d Sectors Per Track - %d Sector(s) Reserved - %d Sector(s) Hidden", var->boot_sector.number_of_sectors_in_the_partition, var->boot_sector.number_of_bytes_per_sector, var->boot_sector.number_of_sectors_per_cluster, var->boot_sector.number_of_sectors_per_track, var->boot_sector.number_of_reserved_sectors, var->boot_sector.number_of_hidden_sectors);            
                LOG_BLANCK("    %d File Allocation Tables (%d Sectors per FAT)", var->boot_sector.number_of_file_allocation_tables, var->boot_sector.number_of_sectors_per_fat);
                LOG_BLANCK("    Media Descriptor: %x - Number of Heads: %d (Current Head: %d) - Physical Drive Number: %x", var->boot_sector.media_descriptor, var->boot_sector.number_of_heads, var->boot_sector.current_head, var->boot_sector.physical_drive_number);
                LOG_BLANCK("    Boot Signature: %2x - Volume ID: %8x", var->boot_sector.boot_signature, var->boot_sector.volume_id);
                LOG_BLANCK("    Handling flags enable: %1d (active FAT: %d)", var->boot_sector.fat_handling_flags.enable, var->boot_sector.fat_handling_flags.active_fat);
                LOG_BLANCK("    FAT32 version: %1d.%1d", var->boot_sector.fat32_version.major, var->boot_sector.fat32_version.minor);
                LOG_BLANCK("    OEM Name: %s - Volume Label: %s", p_string(var->boot_sector.oem_name), p_string(var->boot_sector.volume_label));            
                LOG_BLANCK("    File System Type: %s", p_string(var->boot_sector.file_system_type));
            }
        }
        else // FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_HIGH or FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_LOW or FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_LBA
        {
            uint16_t small_number_of_sectors_in_the_partition;
            
            var->boot_sector.jump_boot_code = (var->_p_ram_rx[0] << 0) | (var->_p_ram_rx[1] << 8) | (var->_p_ram_rx[2] << 16);
            memcpy((void *) &var->boot_sector.oem_name, &var->_p_ram_rx[3], 8);
            var->boot_sector.number_of_bytes_per_sector = (var->_p_ram_rx[11] << 0) | (var->_p_ram_rx[12] << 8);
            var->boot_sector.number_of_sectors_per_cluster = var->_p_ram_rx[13];
            var->boot_sector.number_of_reserved_sectors = (var->_p_ram_rx[14] << 0) | (var->_p_ram_rx[15] << 8);
            var->boot_sector.number_of_file_allocation_tables = var->_p_ram_rx[16];
            var->boot_sector.number_of_possible_root_directory_entries = (var->_p_ram_rx[17] << 0) | (var->_p_ram_rx[18] << 8);
            small_number_of_sectors_in_the_partition = (var->_p_ram_rx[19] << 0) | (var->_p_ram_rx[20] << 8);            
            var->boot_sector.media_descriptor = var->_p_ram_rx[21];
            var->boot_sector.number_of_sectors_per_fat = (var->_p_ram_rx[22] << 0) | (var->_p_ram_rx[23] << 8);
            var->boot_sector.number_of_sectors_per_track = (var->_p_ram_rx[24] << 0) | (var->_p_ram_rx[25] << 8);
            var->boot_sector.number_of_heads = (var->_p_ram_rx[26] << 0) | (var->_p_ram_rx[27] << 8);
            var->boot_sector.number_of_hidden_sectors = (var->_p_ram_rx[28] << 0) | (var->_p_ram_rx[29] << 8) | (var->_p_ram_rx[30] << 16) | (var->_p_ram_rx[31] << 24);
            var->boot_sector.number_of_sectors_in_the_partition = (small_number_of_sectors_in_the_partition == 0) ? ((var->_p_ram_rx[32] << 0) | (var->_p_ram_rx[33] << 8) | (var->_p_ram_rx[34] << 16) | (var->_p_ram_rx[35] << 24)) : small_number_of_sectors_in_the_partition;
            var->boot_sector.physical_drive_number = var->_p_ram_rx[36];
            var->boot_sector.current_head = var->_p_ram_rx[37];
            var->boot_sector.boot_signature = var->_p_ram_rx[38];
            var->boot_sector.volume_id = (var->_p_ram_rx[39] << 0) | (var->_p_ram_rx[40] << 8) | (var->_p_ram_rx[41] << 16) | (var->_p_ram_rx[42] << 24);
            memcpy((void *) &var->boot_sector.volume_label, &var->_p_ram_rx[43], 11);
            memcpy((void *) &var->boot_sector.file_system_type, &var->_p_ram_rx[54], 8);

            var->boot_sector.reserved_region_start = var->master_boot_record.partition_entry[0].first_sector_of_the_partition;
            var->boot_sector.fat_region_start = var->boot_sector.reserved_region_start + var->boot_sector.number_of_reserved_sectors;
            var->boot_sector.root_directory_region_start = var->boot_sector.fat_region_start + (var->boot_sector.number_of_file_allocation_tables * var->boot_sector.number_of_sectors_per_fat);
            var->boot_sector.data_space_region_start = var->boot_sector.root_directory_region_start + (var->boot_sector.number_of_possible_root_directory_entries * 32 / var->boot_sector.number_of_bytes_per_sector);
        
            if (var->is_log_enable)
            {
                LOG_BLANCK("\nBoot Sector Partition 0 (FAT16):");
                LOG_BLANCK("        Boot region sector: %d", var->boot_sector.reserved_region_start);
                LOG_BLANCK("        FAT region sector: %d", var->boot_sector.fat_region_start);
                LOG_BLANCK("        Root Directory region sector: %d", var->boot_sector.root_directory_region_start);
                LOG_BLANCK("        Data Space region sector: %d (start at Cluster 2)", var->boot_sector.data_space_region_start);
                LOG_BLANCK("    Capacity Of The Partition: %d MB", var->boot_sector.number_of_sectors_in_the_partition/1024*var->boot_sector.number_of_bytes_per_sector/1024);
                LOG_BLANCK("    Jump Command: %6x", var->boot_sector.jump_boot_code);
                LOG_BLANCK("    %d Sectors in the partition - %d Bytes Per Sector - %d Sectors Per Cluster - %d Sectors Per Track - %d Sector(s) Reserved - %d Sector(s) Hidden", var->boot_sector.number_of_sectors_in_the_partition, var->boot_sector.number_of_bytes_per_sector, var->boot_sector.number_of_sectors_per_cluster, var->boot_sector.number_of_sectors_per_track, var->boot_sector.number_of_reserved_sectors, var->boot_sector.number_of_hidden_sectors);            
                LOG_BLANCK("    %d File Allocation Tables (%d Sectors per FAT) - %d Possible Root Directory Entries", var->boot_sector.number_of_file_allocation_tables, var->boot_sector.number_of_sectors_per_fat, var->boot_sector.number_of_possible_root_directory_entries);
                LOG_BLANCK("    Media Descriptor: %x - Number of Heads: %d (Current Head: %d) - Physical Drive Number: %x", var->boot_sector.media_descriptor, var->boot_sector.number_of_heads, var->boot_sector.current_head, var->boot_sector.physical_drive_number);
                LOG_BLANCK("    Boot Signature: %2x - Volume ID: %8x", var->boot_sector.boot_signature, var->boot_sector.volume_id);
                LOG_BLANCK("    OEM Name: %s - Volume Label: %s", p_string(var->boot_sector.oem_name), p_string(var->boot_sector.volume_label));            
                LOG_BLANCK("    File System Type: %s", p_string(var->boot_sector.file_system_type));
            }
        }
    }
    else
    {
        if (var->is_log_enable)
        {
            LOG_BLANCK("\nBoot Sector Fail (0xAA55 mismatch)");
        }
    }    
}

static bool _fat16_search_and_sort_files(sd_card_params_t *var, uint32_t *current_sector)
{
    uint8_t i;
    static uint8_t last_entry_index = 0;  
    static uint32_t save_address[10] = {0};    
    static uint8_t save_path_name_length[10] = {0};
    static uint8_t save_index = 0;
    
    uint8_t first_byte = 0;
    uint8_t file_attributes = 0;
    
    static uint8_t lfn_number_of_entry = 0;
    static uint8_t lfn_data_entries[32*19] = {0};        
    
    char last_entry_name[255] = {0};
    uint8_t last_entry_name_length = 0;
    
    static char path_name[255] = {0};
    char file_name[255] = {0};
    
    while (1)
    {
        first_byte = var->_p_ram_rx[last_entry_index * 32];
        file_attributes = var->_p_ram_rx[last_entry_index * 32 + 0x0b];
                        
        if (first_byte != 0x00)
        {
            if ((first_byte != 0xe5) && (first_byte != 0x2e))
            {   
                if (file_attributes != FAT_FILE_SYSTEME_FA_LFN)    
                {
                    if (!(file_attributes & FAT_FILE_SYSTEME_FA_VOLUME_NAME))
                    {
                    
                        last_entry_name_length = _get_last_entry_name(&lfn_number_of_entry, lfn_data_entries, &var->_p_ram_rx[last_entry_index * 32], last_entry_name);

                        if (!(file_attributes & FAT_FILE_SYSTEME_FA_DIRECTORY))
                        {
                            if (var->is_log_enable)
                            {
                                uint32_t first_cluster = (var->_p_ram_rx[last_entry_index * 32 + 0x1a] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x1b] << 8) | (var->_p_ram_rx[last_entry_index * 32 + 0x14] << 16) | (var->_p_ram_rx[last_entry_index * 32 + 0x15] << 24);
                                uint32_t first_sector = fat_file_system_get_first_sector_of_cluster_N(first_cluster);
                                uint32_t size = (var->_p_ram_rx[last_entry_index * 32 + 0x1c] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x1d] << 8) | (var->_p_ram_rx[last_entry_index * 32 + 0x1e] << 16) | (var->_p_ram_rx[last_entry_index * 32 + 0x1f] << 24);
                                if (save_index > 0)
                                {
                                    LOG_BLANCK("    \\%s%s (first cluster: %d / first sector: %d / size: %d bytes)", p_string(path_name), p_string(last_entry_name), first_cluster, first_sector, size);
                                }
                                else
                                {
                                    LOG_BLANCK("    \\%s (first cluster: %d / first sector: %d / size: %d bytes)", p_string(last_entry_name), first_cluster, first_sector, size);
                                }
                            }
                            memset(file_name, 0, 255);
                            memcpy(file_name, path_name, save_path_name_length[save_index]);
                            memcpy(&file_name[save_path_name_length[save_index]], last_entry_name, last_entry_name_length); 

                            var->number_of_file++;                            
                            for (i = 0 ; i < var->number_of_p_file ; i++)
                            {    
                                if (!strcmp(file_name, var->p_file[i]->file_name))
                                {
                                    var->p_file[i]->flags.is_found = true;
                                    var->p_file[i]->file_attributes.value = file_attributes;
                                    var->p_file[i]->last_write_time.value = (var->_p_ram_rx[last_entry_index * 32 + 0x16] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x17] << 8);
                                    var->p_file[i]->last_write_date.value = (var->_p_ram_rx[last_entry_index * 32 + 0x18] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x18] << 8);
                                    var->p_file[i]->first_cluster_of_the_file = (var->_p_ram_rx[last_entry_index * 32 + 0x1a] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x1b] << 8) | (var->_p_ram_rx[last_entry_index * 32 + 0x14] << 16) | (var->_p_ram_rx[last_entry_index * 32 + 0x15] << 24);
                                    var->p_file[i]->file_size = (var->_p_ram_rx[last_entry_index * 32 + 0x1c] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x1d] << 8) | (var->_p_ram_rx[last_entry_index * 32 + 0x1e] << 16) | (var->_p_ram_rx[last_entry_index * 32 + 0x1f] << 24);
                                }
                            }
                        }
                        else
                        {
                            uint32_t first_cluster_of_the_folder = (var->_p_ram_rx[last_entry_index * 32 + 0x1a] << 0) | (var->_p_ram_rx[last_entry_index * 32 + 0x1b] << 8) | (var->_p_ram_rx[last_entry_index * 32 + 0x14] << 16) | (var->_p_ram_rx[last_entry_index * 32 + 0x15] << 24);
                            var->number_of_folder++;  
                            memcpy(&path_name[save_path_name_length[save_index]], last_entry_name, last_entry_name_length);
                            path_name[save_path_name_length[save_index] + last_entry_name_length] = '\\';
                            save_path_name_length[save_index + 1] = save_path_name_length[save_index] + last_entry_name_length + 1;
                            save_address[save_index] = (*current_sector * var->boot_sector.number_of_bytes_per_sector) + ((last_entry_index + 1) * 32);
                            save_index++;
                            *current_sector = fat_file_system_get_first_sector_of_cluster_N(first_cluster_of_the_folder);                      
                            last_entry_index = 0;

                            if (var->is_log_enable)
                            {
                                LOG_BLANCK("D   \\%s", p_string(path_name));
                            }

                            return 1;
                        }
                    }
                }
                else 
                {
                    if (GET_BIT(first_byte, 6))
                    {
                        // Long File Name detected (get the number of entry useful - start with 0x4...)
                        lfn_number_of_entry = first_byte & 0x3f;
                    }    
                    memcpy(&lfn_data_entries[(lfn_number_of_entry - (first_byte & 0x3f)) * 32], &var->_p_ram_rx[last_entry_index * 32], 32);
                }
            }
            
            if (++last_entry_index >= 16)
            {
                last_entry_index = 0;
                (*current_sector)++;
                return 1;
            }  
        }
        else
        {
            if (save_index > 0)
            {
                save_index--;
                *current_sector = save_address[save_index] / var->boot_sector.number_of_bytes_per_sector;
                last_entry_index = (save_address[save_index] % var->boot_sector.number_of_bytes_per_sector) / 32;
                save_address[save_index] = 0;
                memset(&path_name[save_path_name_length[save_index]], 0, save_path_name_length[save_index+1]);
                return 1;
            }
            else
            {
                if (var->is_log_enable)
                {
                    LOG_BLANCK("%d folders / %d files\n", var->number_of_folder, var->number_of_file); 
                }
                return 0;
            }
        }
    }
}

static bool _fat32_search_and_sort_files(sd_card_params_t *var, uint32_t *current_sector)
{
    return 0;
}

static uint8_t sd_card_send_command(sd_card_params_t *var, SD_CARD_COMMAND_TYPE cde_type, uint32_t args, SD_CARD_RESPONSE_COMMAND ret, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_GET_RESPONSE     
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
            
            // Send one dummy byte (essential for some SD CARD - but not for all...)
            while (spi_write_and_read_8(var->spi_id, 0xff, var->_p_ram_rx));
               
            var->response_command.is_response_returned = false;
            
            var->dma_tx_params.src_size = 6;
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;
            
            var->_p_ram_tx[0] = cde_type;
            var->_p_ram_tx[1] = (args >> 24) & 0xff;
            var->_p_ram_tx[2] = (args >> 16) & 0xff;
            var->_p_ram_tx[3] = (args >> 8) & 0xff;
            var->_p_ram_tx[4] = (args >> 0) & 0xff;
            var->_p_ram_tx[5] = sd_card_crc7(&var->_p_ram_tx[0], 5);  
            
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, false, OFF);   // Do not force the transfer (it occurs automatically when data is received - SPI Rx generates the transfer)
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, OFF);    // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).            
            dma_channel_enable(var->dma_rx_id, ON);
            dma_channel_enable(var->dma_tx_id, ON);
            
            functionState = SM_GET_RESPONSE;
            break;
            
        case SM_GET_RESPONSE:
            
            if ((dma_get_flags(var->dma_rx_id) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(var->dma_rx_id, DMA_FLAG_BLOCK_TRANSFER_DONE);  
                
                uint8_t number_of_retransmission = 8;
                
                do
                {
                    if (!spi_write_and_read_8(var->spi_id, 0xff, var->_p_ram_rx))
                    {
                        if (var->_p_ram_rx[0] != 0xff)
                        {
                            var->response_command.R1.value = var->_p_ram_rx[0];
                            var->response_command.is_response_returned = true;  

                            if (ret == SD_CARD_RET_R1B)
                            {

                            }
                            else if (ret > SD_CARD_RET_R1B)
                            {
                                uint8_t i = 0;

                                do
                                {
                                    if (!spi_write_and_read_8(var->spi_id, 0xff, &var->_p_ram_rx[i]))
                                    {
                                        i++;
                                    }
                                }
                                while (i < 4);

                                if (ret == SD_CARD_RET_R3)
                                {
                                    var->response_command.R3.value = (uint32_t) ((var->_p_ram_rx[0] << 24) | (var->_p_ram_rx[1] << 16) | (var->_p_ram_rx[2] << 8) | (var->_p_ram_rx[3] << 0));
                                }
                                else if (ret == SD_CARD_RET_R7)
                                {
                                    var->response_command.R7.value = (uint32_t) ((var->_p_ram_rx[0] << 24) | (var->_p_ram_rx[1] << 16) | (var->_p_ram_rx[2] << 8) | (var->_p_ram_rx[3] << 0));
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
                            break;
                        }
                        else
                        {
                            number_of_retransmission--;
                        }
                    }
                }
                while (number_of_retransmission > 0);
                
                functionState = SM_FREE;
                
            }
            break;
    }
    
    return functionState;
}

static uint8_t sd_card_read_data(sd_card_params_t *var, uint16_t length, SPI_CS_CDE cs_at_begining_of_transmission, SPI_CS_CDE cs_at_end_of_transmission)
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
            memset((void *) var->_p_ram_tx, 0xff, length);
            var->dma_tx_params.src_size = length;
            var->dma_rx_params.dst_size = var->dma_tx_params.src_size;     
            
            dma_set_transfer(var->dma_rx_id, &var->dma_rx_params, false, OFF);   // Do not force the transfer (it occurs automatically when data is received - SPI Rx generates the transfer)
            dma_set_transfer(var->dma_tx_id, &var->dma_tx_params, true, OFF);    // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).            
            dma_channel_enable(var->dma_rx_id, ON);
            dma_channel_enable(var->dma_tx_id, ON);
            
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
    static uint8_t fail_index = 0;
    
    switch (functionState)
    {
        case SM_FREE:      
            
            if (var->is_log_enable) 
            { 
                LOG_BLANCK("\nSD Card Initialization..."); 
            }
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
            
            if (!sd_card_read_data(var, 10, SPI_CS_SET, SPI_CS_DO_NOTHING))
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
                    fail_index = 1;
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
                                fail_index = 2;
                                functionState = SM_FAIL;
                            }
                        }
                        else
                        {
                            fail_index = 3;
                            functionState = SM_FAIL;
                        }
                    }
                }
                else
                {
                    fail_index = 4;
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
                    fail_index = 5;
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
                    fail_index = 6;
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
                    fail_index = 7;
                    functionState = SM_FAIL;
                }
                
                if (mTickCompare(functionTick) >= TICK_1S)
                {
                    fail_index = 8;
                    functionState = SM_FAIL;        // Timeout: SD Card stay in Idle mode
                }
            }
            break;
            
        case SM_CMD_58_SECOND:
            
            if (!sd_card_send_command(var, SD_CARD_CMD_58, 0x00000000, SD_CARD_RET_R3, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {                
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_NORMAL_STATE) && var->response_command.is_response_returned)
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
                    fail_index = 9;
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_16:
            
            // Set 512 bytes per sector (case if the SD card is Ver2.X SDSC - Standard Capacity or Ver1.X)
            if (!sd_card_send_command(var, SD_CARD_CMD_16, 0x00000200, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_NORMAL_STATE) && var->response_command.is_response_returned)
                {
                    functionState = SM_CMD_59;
                }
                else
                {
                    fail_index = 10;
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_CMD_59:
            
            // Disable CRC
            if (!sd_card_send_command(var, SD_CARD_CMD_59, 0x00000000, SD_CARD_RET_R1, SPI_CS_DO_NOTHING, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_NORMAL_STATE) && var->response_command.is_response_returned)
                {
                    functionState = SM_END_OF_INIT;
                }
                else
                {
                    fail_index = 11;
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_END_OF_INIT:
            
            if (var->is_log_enable)
            {
                if (var->card_version == SD_CARD_VER_1_X)
                {
                    LOG_BLANCK("SD Card version: Ver1.X SDSC (Standard Capacity)\nSD Card is ready !");
                }
                else if (var->card_version == SD_CARD_VER_2_X_SDSC)
                {
                    LOG_BLANCK("SD Card version: Ver2.00 or upper - SDSC (Standard Capacity)\nSD Card is ready !");
                }
                else if (var->card_version == SD_CARD_VER_2_X_SDHC)
                {
                    LOG_BLANCK("SD Card version: Ver2.00 or upper - SDHC / SDXC (High or Extended Capacity)\nSD Card is ready !");
                }
            }
            functionState = SM_FREE;
            ports_set_bit(var->spi_cs);
            spi_set_frequency(var->spi_id, SD_CARD_FREQ);
            break;
            
        case SM_FAIL:
            
            ports_set_bit(var->spi_cs);
            mUpdateTick(functionTick);
            functionState++;
            if (var->is_log_enable) 
            { 
                LOG_BLANCK("    SD Card Fail: Retry... (code: %d)", fail_index); 
            }
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

static uint8_t sd_card_get_packet(sd_card_params_t *var, SD_CARD_COMMAND_TYPE cde_type, uint32_t args, SD_CARD_RESPONSE_COMMAND ret, uint16_t packet_length)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_CMD,
        SM_WAIT_START_TOKEN,
        SM_READ_DATA_PACKET,
        SM_FAIL
    } functionState = 0;
    static uint64_t functionTick = 0;
    static uint8_t fail_count = 0;
    
    switch (functionState)
    {
        case SM_FREE:      
            
            fail_count = 0;
            functionState = SM_CMD;   
            
        case SM_CMD:
            
            if (!sd_card_send_command(var, cde_type, args, ret, SPI_CS_CLR, SPI_CS_DO_NOTHING))
            {
                if (!(var->response_command.R1.value & R1_RESPONSE_MASK_NORMAL_STATE) && var->response_command.is_response_returned)
                {                    
                    functionState = SM_WAIT_START_TOKEN;
                }
                else
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_WAIT_START_TOKEN:
            
            if (!spi_write_and_read_8(var->spi_id, 0xff, var->_p_ram_rx))
            {
                if (var->_p_ram_rx[0] == SD_CARD_DATA_TOKEN)
                {
                    functionState = SM_READ_DATA_PACKET;
                }
                else if (!(var->_p_ram_rx[0] & SD_CARD_MASK_ERROR_TOKEN))
                {
                    functionState = SM_FAIL;
                }
            }
            break;
            
        case SM_READ_DATA_PACKET:
            
            if (!sd_card_read_data(var, packet_length, SPI_CS_DO_NOTHING, SPI_CS_SET))
            {      
                functionState = SM_FREE;
            }
            break;
            
        case SM_FAIL:
            
            if (++fail_count >= 10)
            {
                functionState = SM_FREE;
                SET_BIT(var->_flags, SM_SD_CARD_INITIALIZATION);
            }
            else
            {
                mUpdateTick(functionTick);
                functionState++;
            }
            break;
            
        default:
            
            if (mTickCompare(functionTick) >= TICK_1MS)
            {
                functionState = SM_CMD;
            }
            break;
    }
    
    return functionState;
}

static uint8_t sd_card_search_files(sd_card_params_t *var)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_FAT16_SEARCH_FILES,
        SM_FAT32_SEARCH_FILES,
        SM_END
    } functionState = 0;
    static uint32_t current_sector = 0;
    uint8_t i;
    
    switch (functionState)
    {
        case SM_FREE:      
            
            if (var->is_log_enable)
            {
                LOG_BLANCK("\nRoot Directories:"); 
            }
            current_sector = var->boot_sector.root_directory_region_start;
            functionState = var->master_boot_record.partition_entry[0]._is_fat_32_partition ? SM_FAT32_SEARCH_FILES : SM_FAT16_SEARCH_FILES;   
            
        case SM_FAT16_SEARCH_FILES:
            
            if (!sd_card_read_single_block(var, current_sector))
            {
                if (!_fat16_search_and_sort_files(var, &current_sector))
                {                                                            
                    functionState = SM_END;
                }
            }
            break;
            
        case SM_FAT32_SEARCH_FILES:
            
            if (!sd_card_read_single_block(var, current_sector))
            {
                if (!_fat32_search_and_sort_files(var, &current_sector))
                {                                                            
                    functionState = SM_END;
                }
            }
            break;
            
        case SM_END:
            
            if (var->is_log_enable) 
            {            
                for (i = 0 ; i < var->number_of_p_file ; i++)
                {
                    if(var->p_file[i]->flags.is_found)
                    {
                        LOG_BLANCK("File open: %s (%d bytes) - Starting Cluster: %d - Attributes: %2x - Last write: %2d/%2d/%4d at %2dh:%2dm:%2ds", p_string(&var->p_file[i]->file_name), var->p_file[i]->file_size, var->p_file[i]->first_cluster_of_the_file, var->p_file[i]->file_attributes.value, var->p_file[i]->last_write_date.day, var->p_file[i]->last_write_date.month, (var->p_file[i]->last_write_date.year + 1980), var->p_file[i]->last_write_time.hours, var->p_file[i]->last_write_time.minutes, var->p_file[i]->last_write_time.seconds);
                    }
                    else
                    {
                        LOG_BLANCK("File Not Found: %s", p_string(&var->p_file[i]->file_name));
                    }
                }
            }
            functionState = SM_FREE;
            break;
            
        default:
            break;
    }
        
    return functionState;
}

static uint8_t sd_card_read_file_data(sd_card_params_t *var)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_READ_FAT_TABLE,
        SM_GET_FILE_DATA
    } functionState = 0;
        
    static uint32_t __data_address = 0;               // [ 0 .. data_address .. (length_file - 1) ]
    static uint32_t __data_length = 0;                // 1 .. length_file        
    
    switch (functionState)
    {
        case SM_FREE:      
            
            __data_address = var->p_file[var->current_selected_file]->_data_address;
            __data_length = var->p_file[var->current_selected_file]->_data_length;
            var->p_file[var->current_selected_file]->buffer.index = 0;
            
            if (__data_address < (var->boot_sector.number_of_bytes_per_sector * var->boot_sector.number_of_sectors_per_cluster))
            {
                var->p_file[var->current_selected_file]->_current_jump_index = 0;
                var->p_file[var->current_selected_file]->current_cluster_of_the_file = var->p_file[var->current_selected_file]->first_cluster_of_the_file;
            }
            
            if (__data_address <= (var->p_file[var->current_selected_file]->file_size - 1))
            {                
                uint32_t jump_index = __data_address / (var->boot_sector.number_of_bytes_per_sector * var->boot_sector.number_of_sectors_per_cluster);     // Number of "jump" (for a same file) to reach the good cluster in the FAT table (the FIRST cluster - start point in the FAT table - is always "var->p_file[var->current_selected_file]->first_cluster_of_the_file".                                
                if (var->p_file[var->current_selected_file]->_current_jump_index != jump_index)
                {   
                    // In which sector of the FAT table the expected cluster is localized ?
                    //      FAT16: ((uint32_t) (fat_region_start + current_cluster_value * 2 / bytes_per_sector))   // Cluster is 16-bit length
                    //      FAT32: ((uint32_t) (fat_region_start + current_cluster_value * 4 / bytes_per_sector))   // Cluster is 32-bit length
                    var->p_file[var->current_selected_file]->_current_fat_sector = ((uint32_t) (var->boot_sector.fat_region_start + var->p_file[var->current_selected_file]->current_cluster_of_the_file * ((var->master_boot_record.partition_entry[0]._is_fat_32_partition) ? 4 : 2) / var->boot_sector.number_of_bytes_per_sector));
                    var->p_file[var->current_selected_file]->_current_jump_index = jump_index;
                    functionState = SM_READ_FAT_TABLE;
                }
                else
                {                    
                    uint8_t sector_index_in_cluster = (__data_address / var->boot_sector.number_of_bytes_per_sector) % var->boot_sector.number_of_sectors_per_cluster;     // Value between [0..Sectors Per Cluster]
                    var->p_file[var->current_selected_file]->_current_data_sector = sector_index_in_cluster + fat_file_system_get_first_sector_of_cluster_N(var->p_file[var->current_selected_file]->current_cluster_of_the_file);
                    var->p_file[var->current_selected_file]->_index_data_in_sector = __data_address % var->boot_sector.number_of_bytes_per_sector;
                    functionState = SM_GET_FILE_DATA;
                }
            }
            else
            {
                functionState = SM_FREE;
            }
            break;
            
        case SM_READ_FAT_TABLE:
            
            // Read the FAT table from the last cluster value.             
            if (!sd_card_read_single_block(var, var->p_file[var->current_selected_file]->_current_fat_sector))
            {  
                uint16_t index_fat_in_sector = ((var->master_boot_record.partition_entry[0]._is_fat_32_partition) ? ((var->p_file[var->current_selected_file]->current_cluster_of_the_file % 128) * 4) : ((var->p_file[var->current_selected_file]->current_cluster_of_the_file % 256) * 2));                
                var->p_file[var->current_selected_file]->current_cluster_of_the_file = (var->_p_ram_rx[index_fat_in_sector + 0] << 0) | (var->_p_ram_rx[index_fat_in_sector + 1] << 8) | ((var->master_boot_record.partition_entry[0]._is_fat_32_partition) ? ((var->_p_ram_rx[index_fat_in_sector + 2] << 16) | (var->_p_ram_rx[index_fat_in_sector + 3] << 24)) : 0);
                
                uint8_t sector_index_in_cluster = (__data_address / var->boot_sector.number_of_bytes_per_sector) % var->boot_sector.number_of_sectors_per_cluster;     // Value between [0..Sectors Per Cluster]
                var->p_file[var->current_selected_file]->_current_data_sector = sector_index_in_cluster + fat_file_system_get_first_sector_of_cluster_N(var->p_file[var->current_selected_file]->current_cluster_of_the_file);
                var->p_file[var->current_selected_file]->_index_data_in_sector = __data_address % var->boot_sector.number_of_bytes_per_sector;
                functionState = SM_GET_FILE_DATA;
            }
            break;
            
        case SM_GET_FILE_DATA:
            
            if (!sd_card_read_single_block(var, var->p_file[var->current_selected_file]->_current_data_sector))
            {   
                uint16_t max_read_byte_in_sector = var->boot_sector.number_of_bytes_per_sector - var->p_file[var->current_selected_file]->_index_data_in_sector;
                
                if (__data_length >= max_read_byte_in_sector)
                {
                    memcpy(&var->p_file[var->current_selected_file]->buffer.p[var->p_file[var->current_selected_file]->buffer.index], &var->_p_ram_rx[var->p_file[var->current_selected_file]->_index_data_in_sector], max_read_byte_in_sector);
                    __data_length -= max_read_byte_in_sector;
                    
                    var->p_file[var->current_selected_file]->buffer.index += max_read_byte_in_sector;
                    __data_address += max_read_byte_in_sector;                    
                }                
                else
                {
                    memcpy(&var->p_file[var->current_selected_file]->buffer.p[var->p_file[var->current_selected_file]->buffer.index], &var->_p_ram_rx[var->p_file[var->current_selected_file]->_index_data_in_sector], __data_length);
                    __data_length = 0;
                }
                
                if (!__data_length)
                {
                    functionState = SM_FREE;
                }
                else
                {
                    uint32_t jump_index = __data_address / (var->boot_sector.number_of_bytes_per_sector * var->boot_sector.number_of_sectors_per_cluster);     // Number of "jump" (for a same file) to reach the good cluster in the FAT table (the FIRST cluster - start point in the FAT table - is always "var->p_file[var->current_selected_file]->first_cluster_of_the_file".                                
                    if (var->p_file[var->current_selected_file]->_current_jump_index != jump_index)
                    {   
                        // In which sector of the FAT table the expected cluster is localized ?
                        //      FAT16: ((uint32_t) (fat_region_start + current_cluster_value * 2 / bytes_per_sector))   // Cluster is 16-bit length
                        //      FAT32: ((uint32_t) (fat_region_start + current_cluster_value * 4 / bytes_per_sector))   // Cluster is 32-bit length
                        var->p_file[var->current_selected_file]->_current_fat_sector = ((uint32_t) (var->boot_sector.fat_region_start + var->p_file[var->current_selected_file]->current_cluster_of_the_file * ((var->master_boot_record.partition_entry[0]._is_fat_32_partition) ? 4 : 2) / var->boot_sector.number_of_bytes_per_sector));
                        var->p_file[var->current_selected_file]->_current_jump_index = jump_index;
                        functionState = SM_READ_FAT_TABLE;
                    }
                    else
                    {                    
                        uint8_t sector_index_in_cluster = (__data_address / var->boot_sector.number_of_bytes_per_sector) % var->boot_sector.number_of_sectors_per_cluster;     // Value between [0..Sectors Per Cluster]
                        var->p_file[var->current_selected_file]->_current_data_sector = sector_index_in_cluster + fat_file_system_get_first_sector_of_cluster_N(var->p_file[var->current_selected_file]->current_cluster_of_the_file);
                        var->p_file[var->current_selected_file]->_index_data_in_sector = __data_address % var->boot_sector.number_of_bytes_per_sector;
                        functionState = SM_GET_FILE_DATA;
                    }
                }
            }
            break;
            
        default:
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
                    SPI_STD_MASTER_CONFIG);
                
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
        
        if (var->is_log_enable)
        {
            LOG_BLANCK("PIC32 initialization done.");
        }
        
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
                SET_BIT(var->_flags, SM_SD_CARD_GET_CID);
                SET_BIT(var->_flags, SM_SD_CARD_GET_CSD);
                SET_BIT(var->_flags, SM_SD_CARD_MASTER_BOOT_RECORD);
                SET_BIT(var->_flags, SM_SD_CARD_PARTITION_BOOT_SECTOR);
                SET_BIT(var->_flags, SM_SD_CARD_ROOT_DIRECTORY);
                var->_sm.index = SM_SD_CARD_HOME;   
            }
            break;
            
        case SM_SD_CARD_GET_CID:
            
            if (!sd_card_get_cid(var))
            {    
                _sort_data_array_to_cid_structure(var);
                CLR_BIT(var->_flags, SM_SD_CARD_GET_CID);
                var->_sm.index = SM_SD_CARD_HOME;   
            }
            break;
            
        case SM_SD_CARD_GET_CSD:
            
            if (!sd_card_get_csd(var))
            {            
                _sort_data_array_to_csd_structure(var);
                CLR_BIT(var->_flags, SM_SD_CARD_GET_CSD);
                var->_sm.index = SM_SD_CARD_HOME;   
            }
            break;
            
        case SM_SD_CARD_MASTER_BOOT_RECORD:
                    
            if (!sd_card_read_single_block(var, 0))
            {
                _sort_data_array_to_master_boot_record_structure(var);                
                CLR_BIT(var->_flags, SM_SD_CARD_MASTER_BOOT_RECORD);
                var->_sm.index = SM_SD_CARD_HOME;
            }
            break;
            
        case SM_SD_CARD_PARTITION_BOOT_SECTOR:
               
            if (var->master_boot_record.partition_entry[0].number_of_sector_in_the_partition > 0)
            {
                if (    (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_LOW) ||
                        (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_HIGH) ||
                        (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_LBA) ||
                        (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_CHS) ||
                        (var->master_boot_record.partition_entry[0].file_system_descriptor == FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_LBA))
                {
                    if (!sd_card_read_single_block(var, var->master_boot_record.partition_entry[0].first_sector_of_the_partition))
                    {
                        _sort_data_array_to_boot_sector_structure(var);
                        CLR_BIT(var->_flags, SM_SD_CARD_PARTITION_BOOT_SECTOR);
                        var->_sm.index = SM_SD_CARD_HOME;
                    }
                }
            }
            break;
            
        case SM_SD_CARD_ROOT_DIRECTORY:
            
            if (!sd_card_search_files(var))
            {
                CLR_BIT(var->_flags, SM_SD_CARD_ROOT_DIRECTORY);
                var->_sm.index = SM_SD_CARD_HOME;
            }
            break;
            
        case SM_SD_CARD_READ_OPERATION_PREPARATION:
            
//            for (i = (var->current_selected_file == 0xff) ? 0 : (var->current_selected_file + 1) ; i < var->number_of_p_file ; i++)              
            for (i = 0 ; i < var->number_of_p_file ; i++)                     
            {
                if (var->p_file[i]->flags.is_read_block_op == FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED)
                {
                    var->current_selected_file = i;
                    var->p_file[i]->flags.is_read_block_op = FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_ON_GOING;
                    var->_sm.index = SM_SD_CARD_READ_OPERATION;
                    break;
                }
            }
            if (i >= var->number_of_p_file)
            {
                var->current_selected_file = 0;
//                var->current_selected_file = 0xff;
//                CLR_BIT(var->_flags, SM_SD_CARD_READ_OPERATION_PREPARATION);
//                var->_sm.index = SM_SD_CARD_HOME;
            }
            break;
            
        case SM_SD_CARD_READ_OPERATION:
            
            if (!sd_card_read_file_data(var))
            {
                var->p_file[var->current_selected_file]->flags.is_read_block_op = FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_TERMINATED;
                var->_sm.index = SM_SD_CARD_READ_OPERATION_PREPARATION;
            }
            break;
                        
    }
}

void sd_card_open(fat_file_system_entry_t *file)
{
    sd_card_params_t *var = (sd_card_params_t *) file->p_sd_card;
    
    var->p_file[var->number_of_p_file++] = file;    
    SET_BIT(var->_flags, SM_SD_CARD_ROOT_DIRECTORY);
}

uint8_t sd_card_read_block_file(fat_file_system_entry_t *file, uint8_t *p_dst, uint32_t data_address, uint32_t block_length)
{
    sd_card_params_t *var = (sd_card_params_t *) file->p_sd_card;
    
    if (file->flags.is_found && !file->flags.is_read_file_stopped && !file->flags.is_read_file_terminated)
    {
        switch (file->sm_read.index)
        {
            case 0:

                file->buffer.p = p_dst;
                file->_data_address = (data_address > file->file_size) ? file->file_size : data_address;
                file->_data_length = ((file->_data_address + block_length) > file->file_size) ? (file->file_size - file->_data_address) : block_length;
                file->flags.is_read_block_op = FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED;
                SET_BIT(var->_flags, SM_SD_CARD_READ_OPERATION_PREPARATION);
                file->sm_read.index = 1;
                break;

            case 1:

                if (file->flags.is_read_block_op == FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_TERMINATED)
                {
                    file->sm_read.index = 0;
                    file->flags.is_read_file_stopped = true;
                    file->flags.is_read_file_terminated = true;
                }
                break;
        }   
        
        return file->sm_read.index;
    }
        
    return 0xff;
}

uint8_t sd_card_read_play_file(fat_file_system_entry_t *file, uint8_t *p_dst, uint16_t block_length, uint32_t period, uint8_t *progression)
{    
    sd_card_params_t *var = (sd_card_params_t *) file->p_sd_card;
        
    if (file->flags.is_found && !file->flags.is_read_file_stopped && !file->flags.is_read_file_terminated)
    {
        switch (file->sm_read.index)
        {
            case 0:

                file->buffer.p = p_dst;
                file->_data_address = 0;
                file->_data_length = (block_length > file->file_size) ? file->file_size : block_length;
                file->flags.is_read_block_op = FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED;
                SET_BIT(var->_flags, SM_SD_CARD_READ_OPERATION_PREPARATION);
                file->sm_read.index = 1;
                break;

            case 1:

                if (file->flags.is_read_block_op == FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_TERMINATED)
                {
                    if (mTickCompare(file->sm_read.tick) >= period)
                    {
                        mUpdateTick(file->sm_read.tick);
                        file->sm_read.index = 2;                
                    }
                }
                break;

            case 2:
                
                file->_data_address += file->_data_length;
                file->_data_length = ((file->_data_address + block_length) > file->file_size) ? (file->file_size - file->_data_address) : block_length;

                if (!file->_data_length)
                {                    
                    file->sm_read.index = 0;
                    file->flags.is_read_file_stopped = true;
                    file->flags.is_read_file_terminated = true;
                    if (progression != NULL)
                    {
                        *progression = 100;
                    }
                }
                else
                {
                    file->sm_read.index = 1;
                    file->flags.is_read_block_op = FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED;
                    SET_BIT(var->_flags, SM_SD_CARD_READ_OPERATION_PREPARATION);
                }                                
                
                break;

        }
        
        if (progression != NULL)
        {
            *progression = file->_data_address * 100 / file->file_size;
        }
        
        return file->sm_read.index;
    }     
    
    return 0xff;
}
