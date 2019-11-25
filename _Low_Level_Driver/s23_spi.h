#ifndef __DEF_SPI
#define	__DEF_SPI

#define SPI1_CLK    __PD10
#define SPI1_SDO    __PD0
#define SPI1_SDI    __PC4
#define SPI1_SS     __PD9

#define SPI2_CLK    __PG6
#define SPI2_SDO    __PG8
#define SPI2_SDI    __PG7
#define SPI2_SS     __PG9

#define SPI3_CLK    __PD15
#define SPI3_SDO    __PF8
#define SPI3_SDI    __PF2
#define SPI3_SS     __PD14

#define SPI4_CLK    __PF13
#define SPI4_SDO    __PF5
#define SPI4_SDI    __PF4
#define SPI4_SS     __PF12

typedef enum
{
    SPI1 = 0,
    SPI2,
    SPI3,
    SPI4,
    SPI_NUMBER_OF_MODULES
} SPI_MODULE;

typedef enum
{
    SPI_CS_SET = 0,
    SPI_CS_CLR,
    SPI_CS_DO_NOTHING
} SPI_CS_CDE;

typedef enum
{
    // master configuration
    SPI_CONF_MSTEN =                    _SPI1CON_MSTEN_MASK,                // set the Master mode
    SPI_CONF_SMP_END =                  _SPI1CON_SMP_MASK,                  // Master Sample Phase for the input bit at the end of the data out time.
    SPI_CONF_SMP_MIDDLE =               0,                                  // Master Sample Phase for the input bit at the middle of the data out time.
    SPI_CONF_MSSEN =                    _SPI1CON_MSSEN_MASK,                // In master mode, SS pin is automatically driven during transmission.
    SPI_CONF_FRMPOL_HIGH =              _SPI1CON_FRMPOL_MASK,               // Master driven SS output active high.
    SPI_CONF_FRMPOL_LOW =               0,                                  // Master driven SS output active low.

    // slave configuration
    SPI_CONF_SLVEN =                    0,                                  // set the Slave mode
    SPI_CONF_SSEN  =                    _SPI1CON_SSEN_MASK,                 // enable the SS input pin.

    // clocking configuration
    SPI_CONF_CKP_HIGH =                 _SPI1CON_CKP_MASK,                  // Idle state for clock is a high level; active state is a low level.
    SPI_CONF_CKP_LOW =                  0,                                  // Idle state for clock is a low level; active state is a high level.
    SPI_CONF_CKE_ON  =                  _SPI1CON_CKE_MASK,                  // Serial output data changes on transition from active clock state to idle clock state (see CKP bit).
    SPI_CONF_CKE_OFF =                  0,                                  // Serial output data changes on transition from idle clock state to active clock state (see CKP bit).

    // data characters configuration
    SPI_CONF_MODE8 =                    0,                                  // set 8 bits/char
    SPI_CONF_MODE16 =                   _SPI1CON_MODE16_MASK,               // set 16 bits/char
    SPI_CONF_MODE32 =                   _SPI1CON_MODE32_MASK,               // set 32 bits/char

    // framed mode configuration
    SPI_CONF_FRMEN =                    _SPI1CON_FRMEN_MASK,                // Enable the Framed SPI support. Otherwise the Framed SPI is disabled.
    SPI_CONF_FSP_IN =                   _SPI1CON_FRMSYNC_MASK,              // Frame Sync Pulse (FSP) direction set to input (Frame Slave). Otherwise the FSP is output and the SPI channel operates as a Frame Master.
    SPI_CONF_FSP_HIGH =                 _SPI1CON_FRMPOL_MASK,               // FSP polarity set active high. Otherwise the FSP is active low.
    SPI_CONF_FSP_CLK1 =                 _SPI1CON_SPIFE_MASK,                // Set the FSP to coincide with the 1st bit clock. Otherwise the FSP precedes the 1st bit clock
    SPI_CONF_FSP_WIDE =                 _SPI1CON_FRMSYPW_MASK,              // set the FSP one character wide. Otherwise the FSP is one clock wide.

    SPI_CONF_FRM_CNT1 =                 (0 << _SPI1CON_FRMCNT_POSITION),    // set the number of characters per frame (Frame Counter) to 1 (default)
    SPI_CONF_FRM_CNT2 =                 (1 << _SPI1CON_FRMCNT_POSITION),    // set the Frame Counter to 2
    SPI_CONF_FRM_CNT4 =                 (2 << _SPI1CON_FRMCNT_POSITION),    // set the Frame Counter to 4
    SPI_CONF_FRM_CNT8 =                 (3 << _SPI1CON_FRMCNT_POSITION),    // set the Frame Counter to 8
    SPI_CONF_FRM_CNT16 =                (4 << _SPI1CON_FRMCNT_POSITION),    // set the Frame Counter to 16
    SPI_CONF_FRM_CNT32 =                (5 << _SPI1CON_FRMCNT_POSITION),    // set the Frame Counter to 32

    // enhanced buffer (FIFO) configuration
    SPI_CONF_ENHBUF =                   _SPI1CON_ENHBUF_MASK,               // enable the enhanced buffer mode

    SPI_CONF_TBE_NOT_FULL =             (3 << _SPI1CON_STXISEL_POSITION),   // Tx Buffer event issued when Tx buffer not full (at least one slot empty)
    SPI_CONF_TBE_HALF_EMPTY =           (2 << _SPI1CON_STXISEL_POSITION),   // Tx Buffer event issued when Tx buffer >= 1/2 empty
    SPI_CONF_TBE_EMPTY =                (1 << _SPI1CON_STXISEL_POSITION),   // Tx Buffer event issued when Tx buffer completely empty
    SPI_CONF_TBE_SR_EMPTY =             (0 << _SPI1CON_STXISEL_POSITION),   // Tx Buffer event issued when the last character is shifted out of the internal Shift Register and the transmit is complete

    SPI_CONF_RBF_FULL =                 (3 << _SPI1CON_SRXISEL_POSITION),   // Rx Buffer event issued when RX buffer is full
    SPI_CONF_RBF_HALF_FULL =            (2 << _SPI1CON_SRXISEL_POSITION),   // Rx Buffer event issued when RX buffer is >= 1/2 full
    SPI_CONF_RBF_NOT_EMPTY =            (1 << _SPI1CON_SRXISEL_POSITION),   // Rx Buffer event issued when RX buffer is not empty
    SPI_CONF_RBF_EMPTY =                (0 << _SPI1CON_SRXISEL_POSITION),   // Rx Buffer event issued when RX buffer is empty (the last character in the buffer is read).

    // general configuration
    SPI_CONF_DISSDO =                   _SPI1CON_DISSDO_MASK,               // disable the usage of the SDO pin by the SPI
    SPI_CONF_SIDL =                     _SPI1CON_SIDL_MASK,                 // enable the Halt in the CPU Idle mode. Otherwise the SPI will be still active when the CPU is in Idle mode.

    SPI_CONF_ON =                       _SPI1CON_ON_MASK,
    SPI_CONF_OFF =                      (0),
} SPI_CONFIG;

#define SPI_STD_MASTER_CONFIG           (SPI_CONF_MSTEN | SPI_CONF_FRMPOL_LOW | SPI_CONF_SMP_MIDDLE | SPI_CONF_MODE8 | SPI_CONF_CKP_HIGH | SPI_CONF_CKE_OFF)

typedef union 
{
    struct 
    {
        unsigned                        SRXISEL:2;
        unsigned                        STXISEL:2;
        unsigned                        DISSDI:1;      // Available only for PIC32MX1XX/2XX
        unsigned                        MSTEN:1;
        unsigned                        CKP:1;
        unsigned                        SSEN:1;
        unsigned                        CKE:1;
        unsigned                        SMP:1;
        unsigned                        MODE16:1;
        unsigned                        MODE32:1;
        unsigned                        DISSDO:1;
        unsigned                        SIDL:1;
        unsigned                        :1;
        unsigned                        SPION:1;
        unsigned                        ENHBUF:1;
        unsigned                        SPIFE:1;
        unsigned                        :5;
        unsigned                        MCLKSEL:1;     // Available only for PIC32MX1XX/2XX
        unsigned                        FRMCNT:3;
        unsigned                        FRMSYPW:1;
        unsigned                        MSSEN:1;
        unsigned                        FRMPOL:1;
        unsigned                        FRMSYNC:1;
        unsigned                        FRMEN:1;
    };
    struct 
    {
        uint32_t                        value;
    };
} spi_con_reg_t;

typedef union 
{
    struct 
    {
        unsigned                        SPIRBF:1;
        unsigned                        SPITBF:1;
        unsigned                        :1;
        unsigned                        SPITBE:1;
        unsigned                        :1;
        unsigned                        SPIRBE:1;
        unsigned                        SPIROV:1;
        unsigned                        SRMT:1;
        unsigned                        SPITUR:1;
        unsigned                        :2;
        unsigned                        SPIBUSY:1;
        unsigned                        :4;
        unsigned                        TXBUFELM:5;
        unsigned                        :3;
        unsigned                        RXBUFELM:5;
    };
    struct 
    {
        uint32_t                        value;
    };
} spi_stat_reg_t;

typedef struct 
{
    volatile spi_con_reg_t              SPICON;
    volatile uint32_t                   SPICONCLR;
    volatile uint32_t                   SPICONSET;
    volatile uint32_t                   SPICONINV;

    volatile spi_stat_reg_t             SPISTAT;
    volatile uint32_t                   SPISTATCLR;
    volatile uint32_t                   SPISTATSET;
    volatile uint32_t                   SPISTATINV;

    volatile uint32_t                   SPIBUF;
    volatile uint32_t                   unused[3];

    volatile uint32_t                   SPIBRG;
    volatile uint32_t                   SPIBRGCLR;
    volatile uint32_t                   SPIBRGSET;
    volatile uint32_t                   SPIBRGINV;
} spi_registers_t;

typedef struct
{
    _io_t SCK;
    _io_t SDO;
    _io_t SDI;
    _io_t SS;
} spi_io_t;

#define SPI_IO_INSTANCE2(_sck_io_port, _sck_io_indice, _sdo_io_port, _sdo_io_indice, _sdi_io_port, _sdi_io_indice, _ss_io_port, _ss_io_indice)             \
{                                               \
    .SCK = { _sck_io_port, _sck_io_indice },    \
    .SDO = { _sdo_io_port, _sdo_io_indice },    \
    .SDI = { _sdi_io_port, _sdi_io_indice },    \
    .SS = { _ss_io_port, _ss_io_indice },       \
}
#define SPI_IO_INSTANCE(_sck, _sdo, _sdi, _ss)         SPI_IO_INSTANCE2(__PORT(_sck), __INDICE(_sck), __PORT(_sdo), __INDICE(_sdo), __PORT(_sdi), __INDICE(_sdi), __PORT(_ss), __INDICE(_ss))
    
typedef void (*spi_event_handler_t)(uint8_t id, IRQ_EVENT_TYPE event_type, uint32_t event_value);

void spi_init(SPI_MODULE id, spi_event_handler_t evt_handler, IRQ_EVENT_TYPE event_type_enable, uint32_t freq_hz, SPI_CONFIG config);
void spi_enable(SPI_MODULE id, bool enable);
void spi_set_mode(SPI_MODULE mSpiModule, SPI_CONFIG mode);
void spi_set_frequency(SPI_MODULE id, uint32_t freq_hz);
bool spi_write_and_read_8(SPI_MODULE id, uint32_t data_w, uint8_t * data_r);
bool spi_write_and_read_16(SPI_MODULE id, uint32_t data_w, uint16_t * data_r);
bool spi_write_and_read_32(SPI_MODULE id, uint32_t data_w, uint32_t * data_r);

const uint8_t spi_get_tx_irq(SPI_MODULE id);
const uint8_t spi_get_rx_irq(SPI_MODULE id);
const void *spi_get_tx_reg(SPI_MODULE id);
const void *spi_get_rx_reg(SPI_MODULE id);

void spi_interrupt_handler(SPI_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data);

#endif
