#ifndef __DEF_SOFTWARE_PWM
#define	__DEF_SOFTWARE_PWM

#define SOFTWARE_PWM_MAX        30

typedef enum
{
    SOFTWARE_PWM_FREQ_50HZ      = 50,
    SOFTWARE_PWM_FREQ_100HZ     = 100,
    SOFTWARE_PWM_FREQ_150HZ     = 150,
    SOFTWARE_PWM_FREQ_200HZ     = 200,
    SOFTWARE_PWM_FREQ_250HZ     = 250          
} SOFTWARE_PWM_FREQUENCY;

typedef enum
{
    SOFTWARE_PWM_RESO_1         = 1,
    SOFTWARE_PWM_RESO_2         = 2,
    SOFTWARE_PWM_RESO_5         = 5,
    SOFTWARE_PWM_RESO_10        = 10,
    SOFTWARE_PWM_RESO_20        = 20
} SOFTWARE_PWM_RESOLUTION;

typedef struct
{
    TIMER_MODULE                timer_module;
    _io_t                       io[SOFTWARE_PWM_MAX];
    uint8_t                     pwm[SOFTWARE_PWM_MAX];
    uint8_t                     number_of_pwm_used;
    uint8_t                     resolution;
    uint16_t                    frequency_hz;
    
    uint8_t                     counter;
} SOFTWAPRE_PWM_PARAMS;

#define SOFTWARE_PWM_PARAMS_INSTANCE(_timer_module, _frequency, _resolution, N, ...)    \
{                                                                                       \
    .timer_module = _timer_module,                                                      \
	.io = { __VA_ARGS__ },                                                              \
    .pwm = {0},                                                                         \
    .number_of_pwm_used = N/2,                                                          \
    .resolution = _resolution,                                                          \
    .frequency_hz = _frequency,                                                         \
    .counter = 0                                                                        \
}

#define SOFTWARE_PWM_DEF(_name, _timer_module, _frequency, _resolution, ...)            \
static SOFTWAPRE_PWM_PARAMS _name = SOFTWARE_PWM_PARAMS_INSTANCE(_timer_module, _frequency, _resolution, COUNT_ARGUMENTS( __VA_ARGS__ ), __VA_ARGS__)

#endif
