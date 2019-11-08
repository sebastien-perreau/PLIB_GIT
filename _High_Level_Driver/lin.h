#ifndef __DEF_LIN2
#define	__DEF_LIN2

#define LIN_MAXIMUM_FRAME           50

#define LIN_NOT_PERIODIC            0

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
    LIN_BUS_TIMING_SLEEP            = 130 * TICK_100MS  // 1,3 seconds
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
            
    _LIN_BUS_INIT                   = 0xff
} LIN_STATE_MACHINE;

typedef struct
{
    bool                        is_data_receive;
    uint8_t                     data;
} lin_event_t;

typedef struct
{
    uint8_t                     data_readback;
    uint8_t                     current_index;
    uint8_t                     next_index;
    uint64_t                    tick;
} lin_state_machine_params_t;

typedef struct
{
    uint16_t                    rx_chksm;
    uint16_t                    readback;
    uint16_t                    timing;
} lin_fails_t;

typedef struct
{    
    bool                        read_write_type;
    uint8_t                     data_index;
    lin_fails_t                 errors;
    bool                        is_updated;
    bool                        force_transfer;
    uint32_t                    periodicity;
    uint64_t                    tick;
    
    uint8_t                     id;
    uint8_t                     length;
    uint8_t                     data[8];
    uint16_t                    checksum;
} lin_frame_params_t;

typedef struct
{
    bool                        is_init_done;
    UART_MODULE                 uart_module;
    _IO                         chip_enable;
    LIN_VERSION                 lin_version;
    lin_frame_params_t          *p_frame[LIN_MAXIMUM_FRAME];
    uint8_t                     number_of_p_frame;
    uint8_t                     current_selected_p_frame;
    lin_state_machine_params_t  state_machine;
    lin_fails_t                 errors;
} lin_params_t;

#define LIN_FRAME_INSTANCE(_rw_type, _id, _period)                          \
{                                                                           \
	.read_write_type = _rw_type,                                            \
    .data_index = 0,                                                        \
    .errors = {0},                                                          \
    .is_updated = 0,                                                        \
    .force_transfer = 0,                                                    \
    .periodicity = (uint32_t) _period,                                      \
    .tick = 0,                                                              \
    .id = _id,                                                              \
    .length = 0,                                                            \
    .data = {0},                                                            \
    .checksum = 0                                                           \
}

#define LIN_FRAME_DEF(_name, _rw_type, _id, _period)                        \
static lin_frame_params_t _name = LIN_FRAME_INSTANCE(_rw_type, _id, _period)

#define LIN_PARAMS_INSTANCE(_uart_module, _version, _io_port, _io_indice, _number_of_p_frame, ...)   \
{                                                                           \
	.is_init_done = false,                                                  \
	.uart_module = _uart_module,                                            \
    .chip_enable = { _io_port, _io_indice },                                \
	.lin_version = _version,                                                \
	.p_frame = { __VA_ARGS__ },                                             \
    .number_of_p_frame = _number_of_p_frame,                                \
    .current_selected_p_frame = 0xff,                                       \
    .state_machine ={0},                                                    \
    .errors = {0}                                                           \
}

#define LIN_DEF(_name, _uart_module, _chip_enable_pin, _version, ...)       \
static lin_params_t _name = LIN_PARAMS_INSTANCE(_uart_module, _version, __PORT(_chip_enable_pin), __INDICE(_chip_enable_pin), COUNT_ARGUMENTS( __VA_ARGS__ ), __VA_ARGS__)

LIN_STATE_MACHINE lin_master_deamon(lin_params_t *var);

#endif
