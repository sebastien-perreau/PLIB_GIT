#ifndef __DEF_SD_CARD
#define __DEF_SD_CARD

#define SD_CARD_FREQ_INIT       250000
#define SD_CARD_FREQ            10000000

typedef enum
{
    SM_SD_CARD_HOME             = 0,
            
    SM_SD_CARD_INITIALIZATION,  // Upper priority
    SM_SD_CARD_GET_CID,
    SM_SD_CARD_GET_CSD,
    SM_SD_CARD_MASTER_BOOT_RECORD,
    SM_SD_CARD_PARTITION_BOOT_SECTOR,
    SM_SD_CARD_FAT1,
    SM_SD_CARD_ROOT_DIRECTORY,
    SM_SD_CARD_DATA_SPACE,      // Lower priority
            
    SM_SD_CARD_MAX_FLAGS,
    SM_SD_CARD_END
} SD_CARD_SM;

// ACMD<n> means a command sequence of CMD55 follow by the command CMD<n>
typedef enum
{
    SD_CARD_CMD_0       = 0x40,
    SD_CARD_CMD_1       = 0x41,    
    SD_CARD_CMD_8       = 0x48,
    SD_CARD_CMD_9       = 0x49,
    SD_CARD_CMD_10      = 0x4a,
    SD_CARD_CMD_12      = 0x4c,           
    SD_CARD_CMD_16      = 0x50,
    SD_CARD_CMD_17      = 0x51,
    SD_CARD_CMD_18      = 0x52,   
    SD_CARD_ACMD_23     = 0x57,
    SD_CARD_CMD_24      = 0x58,
    SD_CARD_CMD_25      = 0x59,
    SD_CARD_ACMD_41     = 0x69,
    SD_CARD_CMD_55      = 0x77,
    SD_CARD_CMD_58      = 0x7a,
    SD_CARD_CMD_59      = 0x7b
} SD_CARD_COMMAND_TYPE;

typedef enum
{
    SD_CARD_RET_R1      = 0x01,                                         // 5 LSB bits are the response command size and the 3 MSB bits are the ID of the response command
    SD_CARD_RET_R1B     = 0x21,
    SD_CARD_RET_R3      = 0x65,
    SD_CARD_RET_R7      = 0x85,
    SD_CARD_RET_CID     = 0xbf,
    SD_CARD_RET_CSD     = 0xdf,
} SD_CARD_RESPONSE_COMMAND;

typedef enum
{
    SD_CARD_VER_1_X                 = 1,                                // SD Card Ver1.X Standard Capacity        
    SD_CARD_VER_2_X_SDSC            = 2,                                // SD Card Ver2.X or Later Standard Capacity
    SD_CARD_VER_2_X_SDHC            = 3,                                // SD Card Ver2.X or Later SDHC / SDXC
} SD_CARD_VERSION;

// Manufacturer ID is present in CID register (get CID just after the Initialization)
typedef enum
{
    SD_CARD_PANASONIC               = 0x01,
    SD_CARD_TOSHIBA                 = 0x02,
    SD_CARD_SANDISK                 = 0x03,
    SD_CARD_SAMSUNG                 = 0x1b,
    SD_CARD_PROGRADE                = 0x1b,
    SD_CARD_ADATA                   = 0x1d,
    SD_CARD_PHISON                  = 0x27,                             // AgfaPhoto, Delkin, Integral, Lexar, Patriot, PNY, Polaroid, Sony, Verbatim
    SD_CARD_LEXAR                   = 0x28,                             // Lexar, PNY, ProGrade
    SD_CARD_SILICON_POWER           = 0x31,
    SD_CARD_KINGSTON                = 0x41,
    SD_CARD_TRANSCEND               = 0x74,
    SD_CARD_PATRIOT                 = 0x76,
    SD_CARD_SONY                    = 0x82,
    SD_CARD_ANGELBIRD               = 0x9c,
    SD_CARD_HOODMAN                 = 0x9c
} SD_CARD_MANUFACTURER_ID;

typedef union 
{
    struct 
    {
        unsigned                    idle_state:1;
        unsigned                    erase_reset:1;
        unsigned                    illegal_command:1;
        unsigned                    command_crc_error:1;
        unsigned                    erase_sequence_error:1;
        unsigned                    address_error:1;
        unsigned                    parameter_error:1;
        unsigned                    :1;
    };
    struct
    {
        uint8_t                     value;
    };
} sd_card_command_R1_response_t;                            // This response is the "Normal Response Command" for most commands
#define R1_RESPONSE_MASK_ERRORS     0x78

typedef union
{
    struct 
    {
        unsigned                    :15;                    // Reserved (read as 0)
        unsigned                    voltage_windows:9;      // VDD Voltage Window (bit 15: 2,7-2,8 / bit 16: 2,8-2,9 / .. / bit 23: 3,5-3,6) (example: if supported 2,7-3,6V then voltage_window = 0x1ff)
        unsigned                    s18a:1;                 // Switching to 1,8V Accepted (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                    :4;
        unsigned                    UHS_II:1;               // UHS-II Card Status (Only UHS-I card supports this bit.) (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                    CCS:1;                  // Card Capacity Status (This bit is valid only when Busy (Bit 31) is set to 1.)
        unsigned                    busy:1;                 // Card Power-up status bit (This bit is set to LOW if the card has not finished the power up routine.)
    };
    struct
    {
        uint32_t                    value;
    };
} sd_card_command_R3_response_t;                            // This response is the "OCR register" for CMD58 & ACMD41 commands

typedef union
{
    struct 
    {
        unsigned                    check_pattern:8;        // Echo back of check pattern (argument in CMD8 command)
        unsigned                    voltage_accepted:4;     // 0: Not Defined / 1: 2,7-3,6V / 2: Reserved for low voltage range / 4: Reserved / 8: Reserved / Others: Not Defined
        unsigned                    :20;                    // Reserved bits
    };
    struct
    {
        uint32_t                    value;
    };
} sd_card_command_R7_response_t;                            // This response is the "Card Interface Condition" for CMD8 command

typedef struct
{
    uint8_t                         manufacturer_id;
    uint16_t                        oem_id;
    char                            product_name[5];
    uint8_t                         product_revision;
    uint32_t                        serial_number;
    uint16_t                        manufacturer_data_code;
} sd_card_command_cid_t;

typedef struct
{
    bool                            is_response_returned;
    sd_card_command_R1_response_t   R1;
    sd_card_command_R3_response_t   R3;
    sd_card_command_R7_response_t   R7;
    sd_card_command_cid_t           CID;
} sd_card_command_responses_t;

#define SD_CARD_DATA_TOKEN          0xfe                                // DATA TOKEN for CMD9/10/17/18/19
#define SD_CARD_ERROR_TOKEN_MASK    0x1f
#define SD_CARD_DATA_BLOCK_LENGTH   514                                 // 512 bytes of usefull data + 2 CRC bytes
#define SD_CARD_END_OF_DATA_BLOCK   0xaa55                              // 2 characters to ends a data packet (510 bytes + 2 characters byte EoP - End of Packet)
#define SD_CARD_MBR_PARTITION_ENTRY_1_OFFSET    0x1be

typedef struct
{
    bool                            is_existing;                        //              true: if datas present / false: all is cleared (0x00)
    uint8_t                         boot_descriptor;                    // [1 byte]     0x80: if active partition / 0x00 if inactive
    uint32_t                        first_partition_sector;             // [3 bytes]    CHS address of first absolute sector in partition (ignore cos LBA used these days)
    uint8_t                         file_system_descriptor;             // [1 byte]     0x04: 16-bit FAT < 32M / 0x06: 16-bit FAT >= 32M / 0x0e: DOS CHS mapped
    uint32_t                        last_partition_sector;              // [3 bytes]    CHS address of last absolute sector in partition (ignore cos LBA used these days)
    uint32_t                        first_sector_of_the_partition;      // [4 bytes]    Number of sector between the MBR (sector 0) and the first sector of the partition (called Boot Sector)
    uint32_t                        number_of_sector_in_the_partition;  // [4 bytes]    Number of sector in the partition x 512 bytes per sector = total size of the partition (in bytes)
} sd_card_partition_entry_t;

typedef struct
{
    sd_card_partition_entry_t       partition_entry[4];
} sd_card_master_boot_record_t;

typedef struct
{
    bool                            is_init_done;
    SPI_MODULE                      spi_id;
    _IO                             spi_cs;
    DMA_MODULE                      dma_tx_id;
    DMA_MODULE                      dma_rx_id;
    DMA_CHANNEL_TRANSFER            dma_tx_params;
    DMA_CHANNEL_TRANSFER            dma_rx_params;
    bool                            is_log_enable;
    
    SD_CARD_VERSION                 card_version;          
    sd_card_command_responses_t     response_command;
    
    sd_card_master_boot_record_t    master_boot_record;
    
    uint8_t                         *_p_ram_tx;
    uint8_t                         *_p_ram_rx;
    
    uint32_t                        _flags;
    state_machine_t                 _sm;
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
    .response_command = {0},                                                                    \
    .master_boot_record = {0},                                                                  \
    ._p_ram_tx = _tx_buffer_ram,                                                                \
    ._p_ram_rx = _rx_buffer_ram,                                                                \
    ._flags = 0,                                                                                \
    ._sm = {0}                                                                                  \
}

#define SD_CARD_DEF(_name, _spi_module, _cs_pin, _enable_log)                                   \
static uint8_t _name ## _tx_buffer_ram_allocation[1+2048+2];                                    \
static uint8_t _name ## _rx_buffer_ram_allocation[1+2048+2];                                    \
static sd_card_params_t _name = SD_CARD_INSTANCE(_spi_module, __PORT(_cs_pin), __INDICE(_cs_pin), _enable_log, _name ## _tx_buffer_ram_allocation, _name ## _rx_buffer_ram_allocation)

void sd_card_deamon(sd_card_params_t *var);

#endif
