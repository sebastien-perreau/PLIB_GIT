#ifndef __DEF_E_25LC512
#define	__DEF_E_25LC512

typedef enum
{
    SM_25LC512_HOME = 0,
            
    SM_25LC512_WRITE_STATUS_REGISTER,  // Upper priority
    SM_25LC512_PAGE_ERASE, 
    SM_25LC512_SECTOR_ERASE,
    SM_25LC512_CHIP_ERASE,
    SM_25LC512_BYTES_ERASE,
    SM_25LC512_READ, 
    SM_25LC512_WRITE,                  // Lower priority
            
    SM_25LC512_MAX_FLAGS,
    SM_25LC512_END
} _25LC512_SM;

typedef enum
{
    _25LC512_INST_WRSR          = 0x01,
    _25LC512_INST_RDSR          = 0x05,   
    _25LC512_INST_READ          = 0x03,
    _25LC512_INST_WRITE         = 0x02, 
    _25LC512_INST_WREN          = 0x06, 
    _25LC512_INST_WRDI          = 0x04,  
    _25LC512_INST_PAGE_ERASE    = 0x42,   
    _25LC512_INST_SECTOR_ERASE  = 0xD8, 
    _25LC512_INST_CHIP_ERASE    = 0xC7, 
    _25LC512_INST_RDIP          = 0xAB, 
    _25LC512_INST_DIP           = 0xB9
} _25LC512_INSTRUCTION;

typedef enum
{
    _25LC512_DISABLE_ALL_SECTOR_PROTECTION  = 0x00,
    _25LC512_ENABLE_SECTOR3_PROTECTION      = 0x04,
    _25LC512_ENABLE_SECTOR23_PROTECTION     = 0x08,
    _25LC512_ENABLE_SECTOR0123_PROTECTION   = 0x0C
} _25LC512_BLOCK_PROTECTION;

typedef union
{
    struct
    {
        unsigned    WIP:1;      // Write in process (indicates whether the 25LC512 is busy) - Read only
        unsigned    WEL:1;      // Write enable latch (indicates the status of the write enable latch) - Read only
        unsigned    BP:2;       // Block protection - Read/Write
        unsigned    reserved:3;
        unsigned    WPEN:1;     // Write protect enable - Read/Write
    };
    
    struct 
    {
        unsigned    w:8;
    };
} __25LC512_STATUS_REGISTERbits;

typedef struct
{
    DYNAMIC_TAB_BYTE                    dW;
    DYNAMIC_TAB_BYTE                    dR;
    uint16_t                            aW;
    uint16_t                            aR;
    __25LC512_STATUS_REGISTERbits       status_bit;
} _25LC512_REGISTERS;

typedef struct
{
    SPI_PARAMS                          spi_params;
    _25LC512_REGISTERS                  registers;
} _25LC512_CONFIG;

#define _25LC512_REGISTERS_INSTANCE(a, b)           \
{                                                   \
    .dW = {a, 0, 0},                                \
    .dR = {b, 0, 0},                                \
    .aW = 0,                                        \
    .aR = 0,                                        \
    .status_bit = {0}                               \
}

#define _25LC512_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time, _buffer_tx, _buffer_rx)\
{                                                                                                   \
    .spi_params = SPI_PARAMS_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time, 0),        \
    .registers = _25LC512_REGISTERS_INSTANCE(_buffer_tx, _buffer_rx)                                \
}

#define _25LC512_DEF(_name, _spi_module, _cs_pin, _periodic_time, _size_tx, _size_rx)               \
static uint8_t _name ## _buffer_tx_ram_allocation[3+_size_tx] = {0xff};                             \
static uint8_t _name ## _buffer_rx_ram_allocation[3+_size_rx] = {0xff};                             \
static _25LC512_CONFIG _name = _25LC512_INSTANCE(_spi_module, _XBR(_cs_pin), _IND(_cs_pin), _periodic_time, _name ## _buffer_tx_ram_allocation, _name ## _buffer_rx_ram_allocation)

void e_25lc512_deamon(_25LC512_CONFIG *var);
static char e_25lc512_read_sequences(_25LC512_CONFIG *var);
static char e_25lc512_erase_sequences(_25LC512_CONFIG *var, uint8_t typeOfErase);
static char e_25lc512_write_sequences(_25LC512_CONFIG *var, bool writeOrErase);
void e_25lc512_check_for_erasing_memory(_25LC512_CONFIG *var, BUS_MANAGEMENT_VAR *bm);

/*
 * STANDARD VERSION
 */
#define e_25lc512_page_erase(var, adress)                   (var.registers.aW = adress, SET_BIT(var.spi_params.flags, SM_25LC512_PAGE_ERASE))
#define e_25lc512_sector_erase(var, adress)                 (var.registers.aW = adress, SET_BIT(var.spi_params.flags, SM_25LC512_SECTOR_ERASE))
#define e_25lc512_chip_erase(var)                           (SET_BIT(var.spi_params.flags, SM_25LC512_CHIP_ERASE))
#define e_25lc512_bytes_erase(var, adress, length)          (var.registers.dW.size = length, var.registers.aW = adress, SET_BIT(var.spi_params.flags, SM_25LC512_BYTES_ERASE))
#define e_25lc512_read_bytes(var, adress, length)           (var.registers.dR.size = length, var.registers.aR = adress, SET_BIT(var.spi_params.flags, SM_25LC512_READ))
#define e_25lc512_write_bytes(var, adress)                  (var.registers.aW = adress, SET_BIT(var.spi_params.flags, SM_25LC512_WRITE))

#define e_25lc512_is_read_in_progress(var)                  GET_BIT(var.spi_params.flags, SM_25LC512_READ)
#define e_25lc512_is_write_in_progress(var)                 GET_BIT(var.spi_params.flags, SM_25LC512_WRITE)

/*
 * POINTER VERSION
 */
#define e_25lc512_page_erase_ptr(var, adress)               (var->registers.aW = adress, SET_BIT(var->spi_params.flags, SM_25LC512_PAGE_ERASE))
#define e_25lc512_sector_erase_ptr(var, adress)             (var->registers.aW = adress, SET_BIT(var->spi_params.flags, SM_25LC512_SECTOR_ERASE))
#define e_25lc512_chip_erase_ptr(var)                       (SET_BIT(var->spi_params.flags, SM_25LC512_CHIP_ERASE))
#define e_25lc512_bytes_erase_ptr(var, adress, length)      (var->registers.dW.size = length, var->registers.aW = adress, SET_BIT(var->spi_params.flags, SM_25LC512_BYTES_ERASE))
#define e_25lc512_read_bytes_ptr(var, adress, length)       (var->registers.dR.size = length, var->registers.aR = adress, SET_BIT(var->spi_params.flags, SM_25LC512_READ))
#define e_25lc512_write_bytes_ptr(var, adress)              (var->registers.aW = adress, SET_BIT(var->spi_params.flags, SM_25LC512_WRITE))

#define e_25lc512_is_read_in_progress_ptr(var)              GET_BIT(var->spi_params.flags, SM_25LC512_READ)
#define e_25lc512_is_write_in_progress_ptr(var)             GET_BIT(var->spi_params.flags, SM_25LC512_WRITE)

#endif

