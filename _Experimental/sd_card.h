#ifndef __DEF_SD_CARD
#define __DEF_SD_CARD

#define SD_CARD_FREQ_INIT       250000
#define SD_CARD_FREQ            25000000

typedef enum
{
    SM_SD_CARD_HOME             = 0,
            
    SM_SD_CARD_INITIALIZATION,  // Upper priority
    SM_SD_CARD_READ_SECTOR_0,
    SM_SD_CARD_READ_ONE_BLOCK,  // Lower priority
            
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
    SD_CARD_CMD_58      = 0x7a
} SD_CARD_COMMAND_TYPE;

typedef enum
{
    SD_CARD_RET_R1      = 1,
    SD_CARD_RET_R3_R7   = 5,
} SD_CARD_COMMAND_RETURN;

typedef struct
{
    bool                        is_init_done;
    SPI_MODULE                  spi_id;
    _IO                         spi_cs;
    DMA_MODULE                  dma_tx_id;
    DMA_MODULE                  dma_rx_id;
    DMA_CHANNEL_TRANSFER        dma_tx_params;
    DMA_CHANNEL_TRANSFER        dma_rx_params;
    
    uint32_t                    _flags;
    state_machine_t             _sm;
} sd_card_params_t;

#define SD_CARD_INSTANCE(_spi_module, _io_port, _io_indice, _tx_buffer_ram, _rx_buffer_ram)     \
{                                                                                               \
    .is_init_done = false,                                                                      \
    .spi_id = _spi_module,                                                                      \
    .spi_cs = { _io_port, _io_indice },                                                         \
    .dma_tx_id = DMA_NUMBER_OF_MODULES,                                                         \
    .dma_rx_id = DMA_NUMBER_OF_MODULES,                                                         \
    .dma_tx_params = {_tx_buffer_ram, NULL, 0, 1, 1, 0x0000},                                   \
    .dma_rx_params = {NULL, _rx_buffer_ram, 1, 0, 1, 0x0000},                                   \
    ._flags = 0,                                                                                \
    ._sm = {0}                                                                                  \
}

#define SD_CARD_DEF(_name, _spi_module, _cs_pin)                                                \
static uint8_t _name ## _tx_buffer_ram_allocation[1+2048+2];                                    \
static uint8_t _name ## _rx_buffer_ram_allocation[1+2048+2];                                    \
static sd_card_params_t _name = SD_CARD_INSTANCE(_spi_module, __PORT(_cs_pin), __INDICE(_cs_pin), _name ## _tx_buffer_ram_allocation, _name ## _rx_buffer_ram_allocation)

void sd_card_deamon(sd_card_params_t *var);

#endif
