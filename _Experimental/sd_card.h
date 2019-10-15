#ifndef __DEF_SD_CARD
#define __DEF_SD_CARD

#define SD_CARD_FREQ_INIT                       250000      // DO NOT MODIFY
#define SD_CARD_FREQ                            10000000    // Can be set up to 25 MHz

#define SD_CARD_MAXIMUM_FILE                    20

typedef enum
{
    SM_SD_CARD_HOME                             = 0,
            
    SM_SD_CARD_INITIALIZATION,                  // Upper priority
    SM_SD_CARD_GET_CID,
    SM_SD_CARD_GET_CSD,
    SM_SD_CARD_MASTER_BOOT_RECORD,
    SM_SD_CARD_PARTITION_BOOT_SECTOR,
    SM_SD_CARD_ROOT_DIRECTORY,
    SM_SD_CARD_READ_OPERATION_PREPARATION,
    SM_SD_CARD_READ_OPERATION,                  // Lower priority
            
    SM_SD_CARD_MAX_FLAGS,
    SM_SD_CARD_END
} SD_CARD_SM;

// ACMD<n> means a command sequence of CMD55 follow by the command CMD<n>
typedef enum
{
    SD_CARD_CMD_0                               = 0x40,
    SD_CARD_CMD_1                               = 0x41,    
    SD_CARD_CMD_8                               = 0x48,
    SD_CARD_CMD_9                               = 0x49,
    SD_CARD_CMD_10                              = 0x4a,
    SD_CARD_CMD_12                              = 0x4c,           
    SD_CARD_CMD_16                              = 0x50,
    SD_CARD_CMD_17                              = 0x51,
    SD_CARD_CMD_18                              = 0x52,   
    SD_CARD_ACMD_23                             = 0x57,
    SD_CARD_CMD_24                              = 0x58,
    SD_CARD_CMD_25                              = 0x59,
    SD_CARD_ACMD_41                             = 0x69,
    SD_CARD_CMD_55                              = 0x77,
    SD_CARD_CMD_58                              = 0x7a,
    SD_CARD_CMD_59                              = 0x7b
} SD_CARD_COMMAND_TYPE;

#define R1_RESPONSE_MASK_ERRORS                 0x78
#define R1_RESPONSE_MASK_NORMAL_STATE           0x7f

typedef enum
{
    SD_CARD_RET_R1                              = 1,
    SD_CARD_RET_R1B,
    SD_CARD_RET_R3,
    SD_CARD_RET_R7,
    SD_CARD_RET_CID,
    SD_CARD_RET_CSD
} SD_CARD_RESPONSE_COMMAND;

typedef enum
{
    SD_CARD_VER_1_X                             = 1,                                // SD Card Ver1.X Standard Capacity        
    SD_CARD_VER_2_X_SDSC                        = 2,                                // SD Card Ver2.X or Later Standard Capacity
    SD_CARD_VER_2_X_SDHC                        = 3,                                // SD Card Ver2.X or Later SDHC / SDXC
} SD_CARD_VERSION;

#define SD_CARD_DATA_TOKEN                      0xfe                                // DATA TOKEN for CMD9/10/17/18/19
#define SD_CARD_MASK_ERROR_TOKEN                0x1f
#define SD_CARD_DATA_BLOCK_LENGTH               514                                 // 512 bytes of usefull data + 2 CRC bytes
#define SD_CARD_END_OF_DATA_BLOCK               0xaa55                              // 2 characters to ends a data packet (510 bytes + 2 characters byte EoP - End of Packet)
#define SD_CARD_CID_LENGTH                      18                                  // 16 bytes of usedfull data + 2 CRC bytes
#define SD_CARD_CSD_LENGTH                      18                                  // 16 bytes of usedfull data + 2 CRC bytes

// Manufacturer ID is present in CID register (get CID just after the Initialization)
typedef enum
{
    SD_CARD_PANASONIC                           = 0x01,
    SD_CARD_TOSHIBA                             = 0x02,
    SD_CARD_SANDISK                             = 0x03,
    SD_CARD_SAMSUNG                             = 0x1b,
    SD_CARD_PROGRADE                            = 0x1b,
    SD_CARD_ADATA                               = 0x1d,
    SD_CARD_PHISON                              = 0x27,                             // AgfaPhoto, Delkin, Integral, Lexar, Patriot, PNY, Polaroid, Sony, Verbatim
    SD_CARD_LEXAR                               = 0x28,                             // Lexar, PNY, ProGrade
    SD_CARD_SILICON_POWER                       = 0x31,
    SD_CARD_KINGSTON                            = 0x41,
    SD_CARD_TRANSCEND                           = 0x74,
    SD_CARD_PATRIOT                             = 0x76,
    SD_CARD_SONY                                = 0x82,
    SD_CARD_ANGELBIRD                           = 0x9c,
    SD_CARD_HOODMAN                             = 0x9c
} SD_CARD_MANUFACTURER_ID;

typedef struct
{
    SD_CARD_MANUFACTURER_ID                     manufacturer_id;                    // An 8 bit binary number that identifies the card manufacturer (cf. SD_CARD_MANUFACTURER_ID enumeration).
    char                                        oem_id[3];                          // A 2 ASCII string characters (the third char is \0) that identifies the card OEM and/or the card contents when used as a distribution media either on ROM or FLASH cards.
    char                                        product_name[6];                    // A 5 ASCII string characters (the sixth char is \0). 
    uint8_t                                     product_revision;                   // The product revision is composed of two Binary Coded Decimal digits (4 bit each). (Example revision 6.2 will be coded as follow: 0110 0010).
    uint32_t                                    serial_number;                      // The serial number is a 32 bit binary number.
    uint16_t                                    manufacturer_data_code;             // The manufacturing date is composed of two hexadecimal digits, one for the year (y) and one for the month (m). The "m" field is the month code (1 = January .. 12 = December). The "y" field is the year code (0 = 2000 .. 19 = 2019 ...).
    uint8_t                                     crc;                                // A 7 bit CRC (bit 0 always '1').
} sd_card_command_cid_t;

typedef struct
{
    unsigned                                    csd_structure:2;                    // [7:6] 0: CSD version No. 1.0 (1-3 reserved).
    unsigned                                    taac:8;                             // [2:0] time unit (0=1ns, 1=10ns, 2=100ns, 3=1us, 4=10us, 5=100us, 6=1ms, 7=10ms) [6:3] time value (0=reserved, 1=1.0, 2=1.2, 3=1.3, 4=1.5, 5=2.0, 6=2.5, 7=3.0, 8=3.5, 9=4.0, 10=4.5, 11=5.0, 12=5.5, 13=6.0, 14=7.0, 15=8.0) [7] reserved bit.
    unsigned                                    nsac:8;                             // Defines the worst case for the clock dependent factor of the data time. The unit for NSAC is 100 clock cycles.
    unsigned                                    transfer_rate:8;                    // [2:0] transfer rate unit (0=100kbit/s, 1=1Mbit/s, 2=10Mbit/s, 3=100Mbit/s, 4..7 are reserved) [6:3] time value (0=reserved, 1=1.0, 2=1.2, 3=1.3, 4=1.5, 5=2.0, 6=2.5, 7=3.0, 8=3.5, 9=4.0, 10=4.5, 11=5.0, 12=5.5, 13=6.0, 14=7.0, 15=8.0) [7] reserved bit. Note that for current SD Cards that field must be always 0 0110 010 which is equal to 25MHz.
    unsigned                                    command_classes:12;                 // The Card Command Class (CCC) defines which command classes are supported by this card. ([0] 1=Class 0 supported / 0=Class 0 not supported, .. , [11] 1=Class 11 supported / 0= Class 11 not supported).
    unsigned                                    max_read_data_block_length:4;       // The maximum read data block length is computed as pow(2, value) (0-8 & 12-15=reserved, 9=pow(2, 9)=512 bytes ...).
    unsigned                                    read_block_partial:1;               // Always '1'.
    unsigned                                    write_block_misalignement:1;        // Defines if the data block to be written by one command can be spread over more than one physical block of the memory device. The size of the memory block is defined in "max_write_data_block_length" (0=invalid, 1=allowed).
    unsigned                                    read_block_misalignement:1;         // Defines if the data block to be read by one command can be spread over more than one physical block of the memory device. The size of the memory block is defined in "max_read_data_block_length" (0=invalid, 1=allowed).
    unsigned                                    dsr_implemented:1;                  // Defines if the configurable driver stage is integrated on the card.
    unsigned                                    device_size:12;                     // This parameter is used to compute the user?s data card capacity (not include the security protected area). The memory capacity of the card is computed from the entries "device_size", "device_size_mult" and "max_read_data_block_length" as follows: 
                                                                                    // Memory capacity = BLOCKNR * BLOCK_LEN 
                                                                                    //      BLOCKNR = (device_size + 1) * MULT
                                                                                    //      MULT = pow(2, device_size_mult + 2)
                                                                                    //      BLOCK_LEN = pow(2, max_read_data_block_length)
    unsigned                                    max_read_current_at_vdd_min:3;      // The maximum values for read and write currents at the minimal power supply VDD are coded as follows: [2:0] value (0=0.5mA, 1=1mA, 2=5mA, 3=10mA, 4=25mA, 5=35mA, 6=60mA, 7=100mA)
    unsigned                                    max_read_current_at_vdd_max:3;      // The maximum values for read and write currents at the maximal power supply VDD are coded as follows: [2:0] value (0=1mA, 1=5mA, 2=10mA, 3=25mA, 4=35mA, 5=45mA, 6=80mA, 7=200mA)
    unsigned                                    max_write_current_at_vdd_min:3;
    unsigned                                    max_write_current_at_vdd_max:3;
    unsigned                                    device_size_mult:3;                 // The factor MULT is defined as pow(2, device_size_mult + 2).
    unsigned                                    erase_single_block_enable:1;        // If "erase_single_block_enable" = ?0?, the host can erase one or multiple units of SECTOR_SIZE. The erase will start from the
                                                                                    // beginning of the sector that contains the start address to the end of the sector that contains the end address. For
                                                                                    // example, if SECTOR_SIZE=31 and the host sets the Erase Start Address to 5 and the Erase End Address to 40. 
                                                                                    // If "erase_single_block_enable" = ?1? the host can erase one or multiple units of 512 bytes. All blocks that contain data from start
                                                                                    // address to end address are erased. For example, if the host sets the Erase Start Address to 5 and the Erase End
                                                                                    // Address to 40. 
    unsigned                                    erase_sector_size:7;                // The size of an erasable sector. The contents of this register is a 7 bit binary coded value, defining the number of write blocks (see "max_write_data_block_length"). The actual size is computed by increasing this number by one. A value of zero means 1 write block, 127 means 128 write blocks. 
    unsigned                                    write_protect_group_size:7;         // The size of a write protected group. The contents of this register is a 7 bit binary coded value, defining the number of erase sectors (see "erase_sector_size"). The actual size is computed by increasing this number by one. A value of zero means 1 erase sector, 127 means 128 erase sectors. 
    unsigned                                    write_protect_group_enable:1;       // A value of ?0? means no group write protection possible. 
    unsigned                                    write_speed_factor:3;               // Defines the typical block program time as a multiple of the read access time.
    unsigned                                    max_write_data_block_length:4;      // The maximum write data block length is computed as pow(2, max_write_data_block_length). The maximum block length might therefore be in the range from 512 up to 2048 bytes. Write Block Length of 512 bytes is always supported. Note that in SD Memory Card the WRITE_BL_LEN is always equal to READ_BL_LEN.
    unsigned                                    write_partial_blocks_enable:1;
    unsigned                                    file_format_group:1;                // Indicates the selected group of file formats. 
    unsigned                                    copy_flag:1;
    unsigned                                    permanent_write_protection:1;       // Permanently protects the whole card content against overwriting or erasing (all write and erase commands for this card are permanently disabled). The default value is ?0?, i.e. not permanently write protected. 
    unsigned                                    temporary_write_protection:1;       // Temporarily protects the whole card content from being overwritten or erased (all write and erase commands for this card are temporarily disabled). This bit can be set and reset. The default value is ?0?, i.e. not write protected. 
    unsigned                                    file_format:2;                      // Indicates the file format on the card. This field is read-only for ROM. 
                                                                                    // If "file_format_group = 1" then values 0..3 are reserved
                                                                                    // If "file_format_group = 0" then (0=Hard disk-like file system with partition table, 1=DOS FAT (floppy-like) with boost sector only (no partition table), 2=Universal File Format, 3=Others / Unknown).
    unsigned                                    crc:7;                              // A 7 bit CRC (bit 0 always '1').
} sd_card_command_csd_t;

typedef union 
{
    struct 
    {
        unsigned                                idle_state:1;
        unsigned                                erase_reset:1;
        unsigned                                illegal_command:1;
        unsigned                                command_crc_error:1;
        unsigned                                erase_sequence_error:1;
        unsigned                                address_error:1;
        unsigned                                parameter_error:1;
        unsigned                                :1;
    };
    struct
    {
        uint8_t                                 value;
    };
} sd_card_command_R1_response_t;    // This response is the "Normal Response Command" for most commands

typedef union
{
    struct 
    {
        unsigned                                :15;                    // Reserved (read as 0)
        unsigned                                voltage_windows:9;      // VDD Voltage Window (bit 15: 2,7-2,8 / bit 16: 2,8-2,9 / .. / bit 23: 3,5-3,6) (example: if supported 2,7-3,6V then voltage_window = 0x1ff)
        unsigned                                s18a:1;                 // Switching to 1,8V Accepted (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                                :4;
        unsigned                                UHS_II:1;               // UHS-II Card Status (Only UHS-I card supports this bit.) (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                                CCS:1;                  // Card Capacity Status (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                                busy:1;                 // Card Power-up status bit (This bit is set to LOW if the card has not finished the power up routine.)
    };
    struct
    {
        uint32_t                                value;
    };
} sd_card_command_R3_response_t;    // This response is the "OCR register" for CMD58 & ACMD41 commands

typedef union
{
    struct 
    {
        unsigned                                check_pattern:8;        // Echo back of check pattern (argument in CMD8 command)
        unsigned                                voltage_accepted:4;     // 0: Not Defined / 1: 2,7-3,6V / 2: Reserved for low voltage range / 4: Reserved / 8: Reserved / Others: Not Defined
        unsigned                                :20;                    // Reserved bits
    };
    struct
    {
        uint32_t                                value;
    };
} sd_card_command_R7_response_t;    // This response is the "Card Interface Condition" for CMD8 command

typedef struct
{
    bool                                        is_response_returned;
    sd_card_command_R1_response_t               R1;
    sd_card_command_R3_response_t               R3;
    sd_card_command_R7_response_t               R7;
} sd_card_command_responses_t;

typedef struct
{
    bool                                        is_init_done;
    SPI_MODULE                                  spi_id;
    _IO                                         spi_cs;
    DMA_MODULE                                  dma_tx_id;
    DMA_MODULE                                  dma_rx_id;
    DMA_CHANNEL_TRANSFER                        dma_tx_params;
    DMA_CHANNEL_TRANSFER                        dma_rx_params;
    bool                                        is_log_enable;
    
    SD_CARD_VERSION                             card_version;  
    sd_card_command_cid_t                       cid;
    sd_card_command_csd_t                       csd;
    sd_card_command_responses_t                 response_command;
    
    fat16_file_system_master_boot_record_t      master_boot_record;
    fat16_file_system_boot_sector_t             boot_sector;
    uint16_t                                    number_of_file;   
    uint16_t                                    number_of_folder;   
    fat16_file_system_entry_t                   *p_file[SD_CARD_MAXIMUM_FILE];
    uint8_t                                     number_of_p_file;
    uint8_t                                     current_selected_file;
    
    uint8_t                                     *_p_ram_tx;
    uint8_t                                     *_p_ram_rx;
    
    uint32_t                                    _flags;
    state_machine_t                             _sm;
} sd_card_params_t;

#define SD_CARD_INSTANCE(_spi_module, _io_port, _io_indice, _enable_log, _tx_buffer_ram, _rx_buffer_ram)     \
{                                                                                               \
    .is_init_done = false,                                                                      \
    .spi_id = _spi_module,                                                                      \
    .spi_cs = { _io_port, _io_indice },                                                         \
    .dma_tx_id = DMA_NUMBER_OF_MODULES,                                                         \
    .dma_rx_id = DMA_NUMBER_OF_MODULES,                                                         \
    .dma_tx_params = {_tx_buffer_ram, NULL, 0, 1, 1, 0x0000},                                   \
    .dma_rx_params = {NULL, _rx_buffer_ram, 1, 0, 1, 0xfffe},                                   \
    .is_log_enable = _enable_log,                                                               \
    .card_version = 0,                                                                          \
    .cid = {0},                                                                                 \
    .csd = {0},                                                                                 \
    .response_command = {0},                                                                    \
    .master_boot_record = {0},                                                                  \
    .boot_sector = {0},                                                                         \
    .number_of_file = 0,                                                                        \
    .number_of_folder = 0,                                                                      \
    .p_file = {NULL},                                                                           \
    .number_of_p_file = 0,                                                                      \
    .current_selected_file = 0xff,                                                              \
    ._p_ram_tx = _tx_buffer_ram,                                                                \
    ._p_ram_rx = _rx_buffer_ram,                                                                \
    ._flags = 0,                                                                                \
    ._sm = {0}                                                                                  \
}

#define SD_CARD_DEF(_name, _spi_module, _cs_pin, _enable_log)                                   \
static uint8_t _name ## _tx_buffer_ram_allocation[512+2];                                       \
static uint8_t _name ## _rx_buffer_ram_allocation[512+2];                                       \
static sd_card_params_t _name = SD_CARD_INSTANCE(_spi_module, __PORT(_cs_pin), __INDICE(_cs_pin), _enable_log, _name ## _tx_buffer_ram_allocation, _name ## _rx_buffer_ram_allocation)

void sd_card_deamon(sd_card_params_t *var);
void sd_card_open(fat16_file_system_entry_t *file);
uint8_t sd_card_read_block_file(fat16_file_system_entry_t *file, uint8_t *p_dst, uint32_t data_address, uint32_t block_length);
uint8_t sd_card_read_play_file(fat16_file_system_entry_t *file, uint8_t *p_dst, uint16_t block_length, uint32_t period, uint8_t *progression);

#endif
