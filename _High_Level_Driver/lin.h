#ifndef __DEF_LIN2
#define	__DEF_LIN2

typedef enum
{
    LIN_VERSION_1_X                 = 0,
    LIN_VERSION_2_X
} LIN_VERSION;

typedef enum
{
    LIN_READ_REQUEST                = 0,
    LIN_WRITE_REQUEST               = 1
} LIN_READ_WRITE_TYPE;

typedef enum
{
    LIN_BUS_TIMING_MAX_8_BYTES      = 9044 * TICK_1US,  // 1,4 x (14 + 10 + 10 + 80 + 10) / 19200
    LIN_BUS_TIMING_MAX_4_BYTES      = 6127 * TICK_1US,  // 1,4 x (14 + 10 + 10 + 40 + 10) / 19200
    LIN_BUS_TIMING_MAX_2_BYTES      = 4668 * TICK_1US,  // 1,4 x (14 + 10 + 10 + 20 + 10) / 19200
    LIN_BUS_TIMING_SLEEP            = 130 * TICK_100MS
} LIN_TIMINGS;

typedef enum
{
    _LIN_HOME   = 0,
    
    _LIN_WAKE_UP,
    _LIN_WAKE_UP_WAIT_100MS,
            
    _LIN_HEADER_BREAK,
    _LIN_HEADER_SYNC,
    _LIN_HEADER_ID,
            
    _LIN_TX_DATA,
    _LIN_RX_DATA,
    _LIN_TX_CHKSM,
    _LIN_RX_CHKSM,
            
    _LIN_WAIT_AND_READBACK,
            
    _LIN_RX_CHKSM_FAIL,
    _LIN_READBACK_FAIL,
    _LIN_TIMING_FAIL,
    _LIN_END,
            
    _LIN_BUS_FRAME_NULL,
    _LIN_BUS_INIT                   = 0xff
} LIN_STATE_MACHINE;

typedef struct
{
    bool                        is_data_receive;
    uint8_t                     data;
} LIN_EVENT;

typedef struct
{
    uint8_t                     data_readback;
    uint8_t                     current_index;
    uint8_t                     next_index;
    uint64_t                    tick;
} LIN_STATE_LACHINE_PARAMS;

typedef struct
{
    uint16_t                    rx_chksm;
    uint16_t                    readback;
    uint16_t                    timing;
} LIN_FAILS;

typedef struct
{    
    bool                        read_write_type;
    uint8_t                     data_index;
    LIN_FAILS                   errors;
    
    uint8_t                     id;
    uint8_t                     length;
    uint8_t                     data[8];
    uint16_t                    checksum;
} LIN_FRAME_PARAMS;

typedef struct
{
    bool                        is_init_done;
    UART_MODULE                 uart_module;
    _IO                         chip_enable;
    LIN_VERSION                 lin_version;
    LIN_FRAME_PARAMS            *frame;
    LIN_STATE_LACHINE_PARAMS    state_machine;
    LIN_FAILS                   errors;
} LIN_PARAMS;

#define LIN_PARAMS_INSTANCE(_uart_module, _version, _io_port, _io_indice)   \
{                                                                           \
	.is_init_done = false,                                                  \
	.uart_module = _uart_module,                                            \
    .chip_enable = { _io_port, _io_indice },                                \
	.lin_version = _version,                                                \
	.frame = NULL,                                                          \
    .state_machine =                                                        \
    {                                                                       \
        .data_readback = 0,                                                 \
        .current_index = 0,                                                 \
        .next_index = 0,                                                    \
        .tick = 0                                                           \
    },                                                                      \
    .errors = {0}                                                           \
}

#define LIN_DEF(_name, _uart_module, _chip_enable_pin, _version)            \
static LIN_PARAMS _name = LIN_PARAMS_INSTANCE(_uart_module, _version, _XBR(_chip_enable_pin), _IND(_chip_enable_pin))

uint8_t lin_master_deamon(LIN_PARAMS *var);

#endif
