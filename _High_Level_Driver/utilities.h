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
    _io_t                   io;
    _IO_ACTIVE_STATE        active_state;
    _SWITCH_TYPE_OF_PUSH    type_of_push;
    uint8_t                 indice;
    bool                    is_updated;
    bool                    is_initialization_done;
    bool                    is_debounce_protection_active;
    uint64_t                tick_debounce;
    uint64_t                tick_longpush;
} switch_params_t;

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
static switch_params_t _name = SWITCH_INSTANCE(__PORT(_io), __INDICE(_io), _active_state)

// ------------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE ENCODER FUNCTION ****
typedef struct
{
    _io_t               io[2];
    _IO_ACTIVE_STATE    active_state;
    int32_t             indice;
    int8_t              last_direction;
    int8_t              add;
    bool                a0;
    bool                b0;
    bool                is_initialization_done;
} encoder_params_t;

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
static encoder_params_t _name = ENCODER_INSTANCE(__PORT(_io_a), __INDICE(_io_a), __PORT(_io_b), __INDICE(_io_b), _active_state)

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
} led_params_t;

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
static led_params_t _name = LED_INSTANCE(_p_out, _enable, _intensity, _t_up, _t_down)

#define RGBW_COLOR_OFF          (rgbw_color_t)  {0, 0, 0, 0}
#define RGBW_COLOR_WHITE        (rgbw_color_t)  {0, 0, 0, 255}
#define RGBW_COLOR_WHITE_MIX    (rgbw_color_t)  {255, 255, 255, 0}
#define RGBW_COLOR_WHITE_ALL    (rgbw_color_t)  {255, 255, 255, 255}
#define RGBW_COLOR_RED          (rgbw_color_t)  {255, 0, 0, 0}
#define RGBW_COLOR_GREEN        (rgbw_color_t)  {0, 255, 0, 0}
#define RGBW_COLOR_BLUE         (rgbw_color_t)  {0, 0, 255, 0}
#define RGBW_COLOR_YELLOW       (rgbw_color_t)  {255, 255, 0, 0}
#define RGBW_COLOR_CYAN         (rgbw_color_t)  {0, 255, 255, 0}
#define RGBW_COLOR_PURPLE       (rgbw_color_t)  {255, 0, 255, 0}

#define RGB_COLOR_OFF           (rgb_color_t)   {0, 0, 0}
#define RGB_COLOR_WHITE         (rgb_color_t)   {255, 255, 255}
#define RGB_COLOR_RED           (rgb_color_t)   {255, 0, 0}
#define RGB_COLOR_GREEN         (rgb_color_t)   {0, 255, 0}
#define RGB_COLOR_BLUE          (rgb_color_t)   {0, 0, 255}
#define RGB_COLOR_YELLOW        (rgb_color_t)   {255, 255, 0}
#define RGB_COLOR_CYAN          (rgb_color_t)   {0, 255, 255}
#define RGB_COLOR_PURPLE        (rgb_color_t)   {255, 0, 255}

#define HSV_COLOR_OFF           (hsv_color_t)   {0, 0, 0}
#define HSV_COLOR_WHITE         (hsv_color_t)   {0, 0, 255}
#define HSV_COLOR_RED           (hsv_color_t)   {0, 255, 255}
#define HSV_COLOR_GREEN         (hsv_color_t)   {510, 255, 255}
#define HSV_COLOR_BLUE          (hsv_color_t)   {1020, 255, 255}
#define HSV_COLOR_YELLOW        (hsv_color_t)   {255, 255, 255}
#define HSV_COLOR_CYAN          (hsv_color_t)   {765, 255, 255}
#define HSV_COLOR_PURPLE        (hsv_color_t)   {1275, 255, 255}

typedef struct
{
    uint8_t                     red;
    uint8_t                     green;
    uint8_t                     blue;
    uint8_t                     white;
} rgbw_color_t;

typedef struct
{
    uint8_t                     red;
    uint8_t                     green;
    uint8_t                     blue;
} rgb_color_t;

typedef struct
{
    uint16_t                    hue;
    uint8_t                     saturation;
    uint8_t                     value;
} hsv_color_t;

// ----------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE SLIDER ROUTINE ****

typedef enum
{
    SLIDER_START_TO_END     = 1,    // |------->
    SLIDER_END_TO_START     = 2,    // <-------|
    SLIDER_CENTER_TO_ENDS   = 4,    // <---|--->
    SLIDER_ENDS_TO_CENTER   = 8     // |--> <--|
} SLIDER_MODES;

typedef enum
{
    SLIDER_SUCCESS = 0,
    SLIDER_TRANSITION_WAIT,
    SLIDER_TRANSITION_UPDATE
} SLIDER_STATUS;

typedef struct
{
    bool                    enable;
    uint8_t                 mode_on;
    uint8_t                 mode_off;
    uint32_t                time_on;
    uint32_t                time_off;
    DYNAMIC_TAB_BOOL        p_output;
    uint8_t                 _mode;
    uint32_t                _time;
    uint8_t                 save_state;
    state_machine_t         state_machine;
} slider_params_t;

#define SLIDER_INSTANCE(_p_output, _size, _mode_on, _mode_off, _time_on, _time_off)         \
{                                                                                           \
    .enable = false,                                                                        \
    .mode_on = _mode_on,                                                                    \
    .mode_off = _mode_off,                                                                  \
    .time_on = (_time_on),                                                                  \
    .time_off = (_time_off),                                                                \
    .p_output = {_p_output, _size, 0},                                                      \
    ._mode = 0,                                                                             \
    ._time = 0,                                                                             \
    .save_state = 2,                                                                        \
    .state_machine = {0}                                                                    \
}

#define SLIDER_DEF(_name, _size, _mode_on, _mode_off, _time_on, _time_off)                  \
static bool _name ## _ram_allocation[_size] = {0};                                       \
static slider_params_t _name = SLIDER_INSTANCE(_name ## _ram_allocation, _size, _mode_on, _mode_off, _time_on, _time_off)

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
} average_params_t;

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
static average_params_t _name = AVERAGE_INSTANCE(_adc_module, _name ## _buffer_ram_allocation, _period)

// ---------------------------------------------------
// ********** STRUCTURE FOR THE NTC ROUTINE **********
typedef enum
{
    NTC_SUCCESS                         = 0,
    NTC_WAIT,
    NTC_FAIL_SHORT_CIRCUIT_GND,
    NTC_FAIL_SHORT_CIRCUIT_VREF
} NTC_STATUS;

typedef struct
{
    uint8_t             t0;
    uint32_t            r0;
    uint16_t            b;
} ntc_settings_t;

typedef struct
{
    average_params_t    average;
    ntc_settings_t      ntc_params;
    uint32_t            pull_up_value;
    float               temperature;
} ntc_params_t;

#define NTC_INSTANCE(_adc_module, _buffer, _t0, _r0, _b, _r_pull_up)        \
{                                                                           \
    .average = AVERAGE_INSTANCE(_adc_module, _buffer, TICK_10MS),           \
    .ntc_params = { _t0, _r0, _b },                                         \
    .pull_up_value = _r_pull_up,                                            \
    .temperature = 0.0,                                                     \
}
#define NTC_DEF(_name, _adc_module, _t0, _r0, _b, _r_pull_up)           \
static float _name ## _buffer_ram_allocation[20] = {0.0};                   \
static ntc_params_t _name = NTC_INSTANCE(_adc_module, _name ## _buffer_ram_allocation, _t0, _r0, _b, _r_pull_up)

// ------------------------------------------------
// ***** STRUCTURE FOR THE HYSTERESIS ROUTINE *****
typedef struct
{
    bool                is_updated;
    uint8_t             current_threshold;    
    uint8_t             hysteresis_gap;
    uint8_t             *p_input_value;
    uint8_t             *p_tipping_threshold;
    uint8_t             number_of_tipping_threshold;
} hysteresis_params_t;

#define HYSTERESIS_INSTANCE(_p_input_value, _hysteresis_gap, _p_tipping_threshold, _number_of_tipping_threshold)    \
{                                                                                                                   \
    .is_updated = 0,                                                                                                \
    .current_threshold = 0,                                                                                         \
    .hysteresis_gap = _hysteresis_gap,                                                                              \
    .p_input_value = _p_input_value,                                                                                \
    .p_tipping_threshold = _p_tipping_threshold,                                                                    \
    .number_of_tipping_threshold = _number_of_tipping_threshold                                                     \
}
#define HYSTERESIS_DEF(_name, _p_input_value, _hysteresis_gap, ...)                             \
static uint8_t _name ## _ram_allocation[1 + COUNT_ARGUMENTS( __VA_ARGS__ )] = { 0, __VA_ARGS__ };      \
static hysteresis_params_t _name = HYSTERESIS_INSTANCE(_p_input_value, _hysteresis_gap, _name ## _ram_allocation, COUNT_ARGUMENTS( __VA_ARGS__ ));

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
    ntc_params_t        ntc;                // .temperature : -40.0 .. 200.0
    average_params_t    current;            // .average: (e.i) 17.48
    average_params_t    voltage;            // .average: (e.i) 12.56
    average_params_t    an15;               // .average: 0 .. 1023
    float               power_consumption;  // voltage x current (@ t time)
    uint64_t            speed;              // UC Speed define in uS
} acquisitions_params_t;

#define ACQUISITIONS_INSTANCE(_buffer_ntc, _buffer_current, _buffer_voltage, _buffer_an15)    \
{                                                                               \
    .ntc = NTC_INSTANCE(AN15, _buffer_ntc, 25, 10000, 3380, 10000),             \
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
static acquisitions_params_t _name = ACQUISITIONS_INSTANCE(_name ## _buffer_ntc_ram_allocation, _name ## _buffer_current_ram_allocation, _name ## _buffer_voltage_ram_allocation, _name ## _buffer_an15_ram_allocation)

// ----------------------------------------------------
// ************** PROTOTYPES OF FUNCTIONS *************

SLIDER_STATUS   fu_slider(slider_params_t *var);

void            fu_switch(switch_params_t *var);
void            fu_encoder(encoder_params_t *config);
bool            fu_turn_indicator(bool enable, uint32_t time_on, uint32_t time_off);

void            fu_led(led_params_t *var);
hsv_color_t     fu_rgb_to_hsv(rgb_color_t rgb_color);
rgb_color_t     fu_hsv_to_rgb(hsv_color_t hsv_color);

bool            fu_adc_average(average_params_t *var);
NTC_STATUS      fu_adc_ntc(ntc_params_t *var);
NTC_STATUS      fu_calc_ntc(ntc_settings_t ntc_params, uint32_t ntc_pull_up, uint16_t v_adc, uint8_t adc_resolution, float *p_temperature);
void            fu_hysteresis(hysteresis_params_t *var);

void            fu_bus_management_task(BUS_MANAGEMENT_VAR *dp);
uint16_t        fu_crc_16_ibm(uint8_t *buffer, uint16_t length);

uint32_t        fu_get_integer_value(float v);
uint32_t        fu_get_decimal_value(float v, uint8_t numbers_after_coma);
float           fu_get_float_value(uint32_t integer, uint8_t decimal);

void            background_tasks(acquisitions_params_t *var);

#endif
