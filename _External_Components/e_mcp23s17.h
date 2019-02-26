#ifndef __DEF_MCP23S17
#define	__DEF_MCP23S17

typedef enum
{
    SM_MCP23S17_HOME = 0,
    SM_MCP23S17_WRITE,
    SM_MCP23S17_READ,
    SM_MCP23S17_END
} MCP23S17_SM;

// Ordre registre avec BANK=0
typedef struct 
{
    unsigned header:16;     // Use only by driver (do not modify))
    
    unsigned IODIRA:8;      // @=0x00   // I/O Direction (1: input / 0: output)
    unsigned IODIRB:8;
    unsigned IPOLA:8;       // @=0x02   // Input polarity port (1: GPIO register bit will reflect the opposite logic state of the input pin / 0: same logic state)
    unsigned IPOLB:8;
    unsigned GPINTENA:8;    // @=0x04   // IRQ on change (1: enable input pin IRQ / 0: disable input pin IRQ)
    unsigned GPINTENB:8;
    unsigned DEFVALA:8;     // @=0x06   // Default compare for IRQ
    unsigned DEFVALB:8;
    unsigned INTCONA:8;     // @=0x08   // Interrupt control register (1: Controls how the associated pin value is compared for IRQ / 0: pin value is compared against the previous pin value)
    unsigned INTCONB:8;
    unsigned IOCON:8;       // @=0x0A   // Configuration register (|BANK|MIRROR|SEQOP|DISSLW|HAEN|ODR|INTPOL|-|)
    unsigned IOCONBIS:8;
    unsigned GPPUA:8;       // @=0x0C   // Pull-up resistor configuration (1: enable / 0: disable)
    unsigned GPPUB:8;
    unsigned INTFA:8;       // @=0x0E   // Interrupt flag (1: pin caused interrupt / 0: interrupt not pending)
    unsigned INTFB:8;
    unsigned INTCAPA:8;     // @=0x10   // Interrupt capture (1: logic high / 0: logic low)
    unsigned INTCAPB:8;
    unsigned GPIOA:8;       // @=0x12   // Port register (1: logic high / 0: logic low)
    unsigned GPIOB:8;
    unsigned OLATA:8;       // @=0x14   // Output latch (1: logic high / 0: logic low)
    unsigned OLATB:8;
} MCP23S17_REGISTERS;

typedef struct
{
    SPI_PARAMS              spi_params;
    MCP23S17_REGISTERS      read_registers;
    MCP23S17_REGISTERS      write_registers;

} MCP23S17_CONFIG;

#define MCP23S17_REGISTERS_INSTANCE(_header)    \
{                                               \
    .header = _header,                          \
    .IODIRA = 0xff,                             \
    .IODIRB = 0xff,                             \
    .IPOLA = 0,                                 \
    .IPOLB = 0,                                 \
    .GPINTENA = 0,                              \
    .GPINTENB = 0,                              \
    .DEFVALA = 0,                               \
    .DEFVALB = 0,                               \
    .INTCONA = 0,                               \
    .INTCONB = 0,                               \
    .IOCON = 0,                                 \
    .IOCONBIS = 0,                              \
    .GPPUA = 0,                                 \
    .GPPUB = 0,                                 \
    .INTFA = 0,                                 \
    .INTFB = 0,                                 \
    .INTCAPA = 0,                               \
    .INTCAPB = 0,                               \
    .GPIOA = 0,                                 \
    .GPIOB = 0,                                 \
    .OLATA = 0,                                 \
    .OLATB = 0,                                 \
}

#define MCP23S17_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time)                    \
{                                                                                               \
    .spi_params = SPI_PARAMS_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time, 0),    \
    .read_registers = MCP23S17_REGISTERS_INSTANCE(0x0041),                                      \
    .write_registers = MCP23S17_REGISTERS_INSTANCE(0x0040)                                      \
}

#define MCP23S17_DEF(_name, _spi_module, _cs_pin, _periodic_time)   \
static MCP23S17_CONFIG _name = MCP23S17_INSTANCE(_spi_module, _XBR(_cs_pin), _IND(_cs_pin), _periodic_time)

BYTE eMCP23S17Deamon(MCP23S17_CONFIG *var);

#endif
