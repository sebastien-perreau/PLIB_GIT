#ifndef __DEF_FUTILITIES
#define __DEF_FUTILITIES

// -----------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE SWITCH FUNCTION ****
typedef enum
{
    SIMPLE_PUSH = 0,
    LONG_PUSH
} _SWITCH_TYPE_OF_PUSH;

typedef struct
{
    _IO                     io;
    _IO_ACTIVE_STATE        active_state;
    _SWITCH_TYPE_OF_PUSH    type_of_push;
    uint8_t                 indice;
    bool                    is_updated;
    bool                    is_initialization_done;
    bool                    is_debounce_protection_active;
    uint64_t                tick_debounce;
    uint64_t                tick_longpush;
} SWITCH_PARAMS;

#define SWITCH_INSTANCE(_io_port, _io_indice, _active_state) \
{                                                           \
    .io = { _io_port, _io_indice },                         \
    .active_state = _active_state,                          \
    .type_of_push = SIMPLE_PUSH,                            \
    .indice = 0,                                            \
    .is_updated = false,                                    \
    .is_initialization_done = false,                        \
    .is_debounce_protection_active = false,                 \
    .tick_debounce = 0,                                     \
    .tick_longpush = 0,                                     \
}

#define SWITCH_DEF(_name, _io, _active_state)               \
static SWITCH_PARAMS _name = SWITCH_INSTANCE(__PORT(_io), __INDICE(_io), _active_state)

// ------------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE ENCODER FUNCTION ****
typedef struct
{
    _IO                 io[2];
    _IO_ACTIVE_STATE    active_state;
    int32_t             indice;
    int8_t              last_direction;
    int8_t              add;
    bool                a0;
    bool                b0;
    bool                is_initialization_done;
} ENCODER_PARAMS;

#define ENCODER_INSTANCE(_io_port_a, _io_indice_a, _io_port_b, _io_indice_b, _active_state) \
{                                                           \
    .io = { { _io_port_a, _io_indice_a },                   \
            { _io_port_b, _io_indice_b }},                  \
    .active_state = _active_state,                          \
    .indice = 0,                                            \
    .last_direction = 0,                                    \
    .add = 0,                                               \
    .a0 = 0,                                                \
    .b0 = 0,                                                \
    .is_initialization_done = false,                        \
}

#define ENCODER_DEF(_name, _io_a, _io_b, _active_state)     \
static ENCODER_PARAMS _name = ENCODER_INSTANCE(__PORT(_io_a), __INDICE(_io_a), __PORT(_io_b), __INDICE(_io_b), _active_state)

// ---------------------------------------------------
// ***** MACRO AND STRUCTURE FOR THE LED ROUTINE *****
typedef struct
{
    bool            enable;
    uint8_t         intensity;
    uint8_t         *p_out;
    uint32_t        t_up;
    uint32_t        t_down;
    uint64_t        tick;
} LED_PARAMS;

#define LED_INSTANCE(_p_out, _enable, _intensity, _t_up, _t_down)       \
{                                                                       \
    .enable = _enable,                                                  \
	.intensity = _intensity,                                            \
    .p_out = (uint8_t *) _p_out,                                        \
    .t_up = _t_up,                                                      \
    .t_down = _t_down,                                                  \
    .tick = 0                                                           \
}

#define LED_DEF(_name, _p_out, _enable, _intensity, _t_up, _t_down)     \
static LED_PARAMS _name = LED_INSTANCE(_p_out, _enable, _intensity, _t_up, _t_down)

typedef struct
{
    uint8_t                     red;
    uint8_t                     green;
    uint8_t                     blue;
    uint8_t                     white;
} RGBW_COLOR;

typedef struct
{
    uint8_t                     red;
    uint8_t                     green;
    uint8_t                     blue;
} RGB_COLOR;

typedef struct
{
    uint16_t                    hue;
    uint8_t                     saturation;
    uint8_t                     value;
} HSV_COLOR;

// ----------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE SLIDER ROUTINE ****
//#define DIR_NONE        0
//#define DIR_RIGHT       1   //  *------->
//#define DIR_LEFT        2   //  <-------*
//#define DIR_EXTERNAL    3   //  <-- * -->
//#define DIR_CENTER      4   //  *--> <--*
//#define INIT_SLIDER(ptr, modeOn, modeOff, tOn, tOff)      {OFF, 0, modeOn, modeOff, 0, ptr, sizeof(ptr)/sizeof(LED_CONFIG), tOn, tOff, TICK_INIT}
//
//typedef struct
//{
//    BOOL enable;
//    BOOL previousState;
//    BYTE modeSlidingOn;
//    BYTE modeSlidingOff;
//    BYTE currentIndice;
//    LED_CONFIG *ptrLed;
//    BYTE sizeTab;
//    QWORD tSliderOn;
//    QWORD tSliderOff;
//    QWORD tickSlider;
//}LED_SLIDER_CONFIG;

// ---------------------------------------------------
// ******** STRUCTURE FOR THE AVERAGE ROUTINE ********
typedef struct
{
    uint16_t                adc_module;
    DYNAMIC_TAB_FLOAT       buffer;
    float                   average;
    float                   sum_of_buffer;
    uint16_t                index_buffer;
    uint64_t                period;
    uint64_t                tick;
} AVERAGE_PARAMS;

#define AVERAGE_INSTANCE(_adc_module, _buffer, _period)         \
{                                                               \
    .adc_module = _adc_module,                                  \
    .buffer = { _buffer, sizeof(_buffer)/sizeof(float), 0 },    \
    .average = 0.0,                                             \
    .sum_of_buffer = 0.0,                                       \
    .index_buffer = 0,                                          \
    .period = _period,                                          \
    .tick = 0,                                                  \
}

#define AVERAGE_DEF(_name, _adc_module, _number_of_acquisition, _period)        \
static float _name ## _buffer_ram_allocation[_number_of_acquisition] = {0.0};   \
static AVERAGE_PARAMS _name = AVERAGE_INSTANCE(_adc_module, _name ## _buffer_ram_allocation, _period)

// ---------------------------------------------------
// ********** STRUCTURE FOR THE NTC ROUTINE **********
typedef struct
{
    uint8_t     t0;
    uint16_t    r0;
    uint16_t    b;
} NTC_PARAMS;

typedef struct
{
    AVERAGE_PARAMS average;
    NTC_PARAMS  ntc_params;
    float       temperature;
} NTC_VAR;

#define NTC_INSTANCE(_adc_module, _buffer, _t0, _r0, _b)            \
{                                                                   \
    .average = AVERAGE_INSTANCE(_adc_module, _buffer, TICK_10MS),   \
    .ntc_params = { _t0, _r0, _b },                                 \
    .temperature = 0.0,                                             \
}
#define NTC_DEF(_name, _adc_module, _t0, _r0, _b)                   \
static float _name ## _buffer_ram_allocation[20] = {0.0};           \
static NTC_VAR _name = NTC_INSTANCE(_adc_module, _name ## _buffer_ram_allocation, _t0, _r0, _b)

// ---------------------------------------------------
// ***** STRUCTURE FOR THE DEAMON PARENT ROUTINE *****
typedef struct
{
    bool                    is_running;
    uint64_t                waiting_period;
    uint64_t                tick;
} BUS_MANAGEMENT_PARAMS;

typedef struct
{
    uint8_t                 number_of_params;
    BUS_MANAGEMENT_PARAMS   *params[];
} BUS_MANAGEMENT_VAR;

#define BUS_MANAGEMENT_INSTANCE(...)                    \
{                                                       \
    .number_of_params = COUNT_ARGUMENTS(__VA_ARGS__),   \
    .params = { __VA_ARGS__ },                          \
}
#define BUS_MANAGEMENT_DEF(_name, ...)  \
static BUS_MANAGEMENT_VAR _name = BUS_MANAGEMENT_INSTANCE(__VA_ARGS__)

// ------------------------------------------------------
// ***** STRUCTURE FOR THE BACKGROUND TASKS ROUTINE *****
typedef struct
{    
    NTC_VAR         ntc;                // .temperature : -40.0 .. 200.0
    AVERAGE_PARAMS  current;            // .average: (e.i) 17.48
    AVERAGE_PARAMS  voltage;            // .average: (e.i) 12.56
    AVERAGE_PARAMS  an15;               // .average: 0 .. 1023
    float           power_consumption;  // voltage x current (@ t time)
    uint64_t        speed;              // UC Speed define in uS
} ACQUISITIONS_PARAMS;

#define ACQUISITIONS_INSTANCE(_buffer_ntc, _buffer_current, _buffer_voltage, _buffer_an15)    \
{                                                                               \
    .ntc = NTC_INSTANCE(AN15, _buffer_ntc, 25, 10000, 3380),                    \
    .current = AVERAGE_INSTANCE(AN15, _buffer_current, TICK_1MS),               \
    .voltage = AVERAGE_INSTANCE(AN15, _buffer_voltage, TICK_1MS),               \
    .an15 = AVERAGE_INSTANCE(AN15, _buffer_an15, TICK_1MS),                     \
    .power_consumption = 0.0,                                                   \
    .speed = 0,                                                                 \
}

#define ACQUISITIONS_DEF(_name)                                     \
static float _name ## _buffer_ntc_ram_allocation[20] = {0.0};       \
static float _name ## _buffer_current_ram_allocation[10] = {0.0};   \
static float _name ## _buffer_voltage_ram_allocation[10] = {0.0};   \
static float _name ## _buffer_an15_ram_allocation[1] = {0.0};   \
static ACQUISITIONS_PARAMS _name = ACQUISITIONS_INSTANCE(_name ## _buffer_ntc_ram_allocation, _name ## _buffer_current_ram_allocation, _name ## _buffer_voltage_ram_allocation, _name ## _buffer_an15_ram_allocation)

// ----------------------------------------------------
// ************** PROTOTYPES OF FUNCTIONS *************

//void        fUtilitiesSlider(LED_SLIDER_CONFIG *config);

void        fu_switch(SWITCH_PARAMS *var);
void        fu_encoder(ENCODER_PARAMS *config);
bool        fu_turn_indicator(bool enable, uint32_t time_on, uint32_t time_off);

void        fu_led(LED_PARAMS *var);
HSV_COLOR   fu_rgb_to_hsv(RGB_COLOR rgb_color);
RGB_COLOR   fu_hsv_to_rgb(HSV_COLOR hsv_color);

bool        fu_adc_average(AVERAGE_PARAMS *var);
bool        fu_ntc(NTC_VAR *var);

void        fu_bus_management_task(BUS_MANAGEMENT_VAR *dp);
uint16_t    fu_crc_16_ibm(uint8_t *buffer, uint16_t length);

uint32_t    fu_get_integer_value(float v);
uint32_t    fu_get_decimal_value(float v, uint8_t numbers_after_coma);
float       fu_get_float_value(uint32_t integer, uint8_t decimal);

void        background_tasks(ACQUISITIONS_PARAMS *var);

#endif
