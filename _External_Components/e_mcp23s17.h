#ifndef __DEF_MCP23S17
#define	__DEF_MCP23S17

#define MCP23S17_FREQ       10000000        // Can be set up to 10 MHz

typedef enum
{
    MCP23S17_IOCON_INTPOL_ACTIVE_HIGH       = 0x02,
    MCP23S17_IOCON_INTPOL_ACTIVE_LOW        = 0x00,
            
    MCP23S17_IOCON_ODR_OPEN_DRAIN           = 0x04,
    MCP23S17_IOCON_ODR_ACTIVE_DRIVER        = 0x00,
            
    MCP23S17_IOCON_ADDRESS_PINS_ENABLE      = 0x08,
    MCP23S17_IOCON_ADDRESS_PINS_DISABLE     = 0x00,
            
    MCP23S17_IOCON_SLEW_RATE_ENABLE         = 0x00,
    MCP23S17_IOCON_SLEW_RATE_DISABLE        = 0x10,
            
    MCP23S17_IOCON_SEQUENTIAL_OP_ENABLE     = 0x00,
    MCP23S17_IOCON_SEQUENTIAL_OP_DISABLE    = 0x20,
            
    MCP23S17_IOCON_INT_PINS_ARE_CONNECTED   = 0x40,
    MCP23S17_IOCON_INT_PINS_ARE_INDEPENDANT = 0x00,
            
    MCP23S17_IOCON_SAME_BANK                = 0x00,
    MCP23S17_IOCON_SEPARATED_BANK           = 0x80
} MCP23S17_IOCON_REG;

// IMPORTANT: Registers order with BANK=0
typedef struct 
{
    uint16_t                _header;        // Used only by the driver (do not modify))
    
    uint8_t                 IODIRA;         // @=0x00   // I/O Direction (1: input / 0: output)
    uint8_t                 IODIRB;
    uint8_t                 IPOLA;          // @=0x02   // Input polarity port (1: GPIO register bit will reflect the opposite logic state of the input pin / 0: same logic state)
    uint8_t                 IPOLB;
    uint8_t                 GPINTENA;       // @=0x04   // IRQ on change (1: enable input pin IRQ / 0: disable input pin IRQ)
    uint8_t                 GPINTENB;
    uint8_t                 DEFVALA;        // @=0x06   // Default compare for IRQ
    uint8_t                 DEFVALB;
    uint8_t                 INTCONA;        // @=0x08   // Interrupt control register (1: Controls how the associated pin value is compared for IRQ / 0: pin value is compared against the previous pin value)
    uint8_t                 INTCONB;
    uint8_t                 IOCON;          // @=0x0A   // Configuration register (|BANK|MIRROR|SEQOP|DISSLW|HAEN|ODR|INTPOL|-|)
                                            //  INTPOL: This bit sets the polarity of the INT output pin:
                                            //      1 = Active-high
                                            //      0 = Active-low
                                            //  ODR: Configures the INT pin as an open-drain output:
                                            //      1 = Open-drain output (overrides the INTPOL bit).
                                            //      0 = Active driver output (INTPOL bit sets the polarity).
                                            //  HAEN: Hardware Address Enable bit (MCP23S17 only):
                                            //      1 = Enables the MCP23S17 address pins.
                                            //      0 = Disables the MCP23S17 address pins
                                            //  DISSLW: Slew Rate control bit for SDA output:
                                            //      1 = Slew rate disabled
                                            //      0 = Slew rate enabled
                                            //  SEQOP: Sequential Operation mode bit:
                                            //      1 = Sequential operation disabled, address pointer does not increment.
                                            //      0 = Sequential operation enabled, address pointer increments.
                                            //  MIRROR: INT Pins Mirror bit
                                            //      1 = The INT pins are internally connected:
                                            //      0 = The INT pins are not connected. INTA is associated with PORTA and INTB is associated with PORTB
                                            //  BANK: Controls how the registers are addressed:
                                            //      1 = The registers associated with each port are separated into different banks.
                                            //      0 = The registers are in the same bank (addresses are sequential). 
    uint8_t                 _iocon_copy;
    uint8_t                 GPPUA;        // @=0x0C   // Pull-up resistor configuration (1: enable / 0: disable)
    uint8_t                 GPPUB;
    uint8_t                 INTFA;        // @=0x0E   // Interrupt flag (1: pin caused interrupt / 0: interrupt not pending)
    uint8_t                 INTFB;
    uint8_t                 INTCAPA;      // @=0x10   // Interrupt capture (1: logic high / 0: logic low)
    uint8_t                 INTCAPB;
    uint8_t                 GPIOA;        // @=0x12   // Port register (1: logic high / 0: logic low)
    uint8_t                 GPIOB;
    uint8_t                 OLATA;        // @=0x14   // Output latch (1: logic high / 0: logic low)
    uint8_t                 OLATB;
} mcp23s17_registers_t;

typedef struct
{
    bool                    is_init_done;
    SPI_MODULE              spi_id;
    _io_t                   spi_cs;
    DMA_MODULE              dma_tx_id;
    DMA_MODULE              dma_rx_id;
    dma_channel_transfer_t  dma_tx_params;
    dma_channel_transfer_t  dma_rx_params;        
    
    mcp23s17_registers_t    *read;
    mcp23s17_registers_t    *write;
    uint8_t                 *__p_device_addresses;
    uint8_t                 __current_selected_device;
    uint8_t                 __number_of_device;
} mcp23s17_params_t;

#define MCP23S17_INSTANCE(_spi_module, _io_port, _io_indice, _device_addresses, _read_ram, _write_ram, _number_of_device) \
{                                                                               \
    .is_init_done = false,                                                      \
    .spi_id = _spi_module,                                                      \
    .spi_cs = { _io_port, _io_indice },                                         \
    .dma_tx_id = DMA_NUMBER_OF_MODULES,                                         \
    .dma_rx_id = DMA_NUMBER_OF_MODULES,                                         \
    .dma_tx_params = {NULL, NULL, sizeof(mcp23s17_registers_t), 1, 1, 0x0000},  \
    .dma_rx_params = {NULL, NULL, 1, sizeof(mcp23s17_registers_t), 1, 0x0000},  \
    .read = _read_ram,                                                          \
    .write = _write_ram,                                                        \
    .__p_device_addresses = _device_addresses,                                  \
    .__current_selected_device = 0,                                             \
    .__number_of_device = _number_of_device                                     \
}

#define MCP23S17_DEF(_name, _spi_module, _cs_pin, ...)                                          \
static uint8_t _name ## _device_addresses[COUNT_ARGUMENTS( __VA_ARGS__ )] = { __VA_ARGS__ };    \
static mcp23s17_registers_t _name ##_read_ram_allocation[COUNT_ARGUMENTS( __VA_ARGS__ )];       \
static mcp23s17_registers_t _name ##_write_ram_allocation[COUNT_ARGUMENTS( __VA_ARGS__ )];      \
static mcp23s17_params_t _name = MCP23S17_INSTANCE(_spi_module, __PORT(_cs_pin), __INDICE(_cs_pin), _name ## _device_addresses, _name ##_read_ram_allocation, _name ##_write_ram_allocation, COUNT_ARGUMENTS( __VA_ARGS__ ))

uint8_t e_mcp23s17_deamon(mcp23s17_params_t *var);

#endif
