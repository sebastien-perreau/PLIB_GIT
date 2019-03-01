#ifndef __DEF_E_VEML7700
#define	__DEF_E_VEML7700

typedef enum
{
    VEML7700_FLAG_SET_CONFIG                            = 0,
    VEML7700_FLAG_SET_HIGH_THRESHOLD,
    VEML7700_FLAG_SET_LOW_THRESHOLD,
    VEML7700_FLAG_GET_AMBIENT_LIGHT_SENSOR,
    VEML7700_FLAG_GET_WHITE,
    VEML7700_FLAG_GET_TRIGGER_EVENT,
            
    VEML7700_FLAG_NUMBERS,
    VEML7700_FLAG_NO_FUNCTION                           = 0xff
} VEML7700_FLAGS;

typedef enum
{
    VEML7700_ADDR_ALS_CONFIG                            = 0,
    VEML7700_ADDR_ALS_HIGH_THRESHOLD                    = 1,
    VEML7700_ADDR_ALS_LOW_THRESHOLD                     = 2,
    VEML7700_ADDR_POWER_SAVING                          = 3,
    VEML7700_ADDR_ALS                                   = 4,
    VEML7700_ADDR_WHITE                                 = 5,
    VEML7700_ADDR_TRIGGER_EVENT                         = 6
} VEML7700_ADDRESS_REGISTERS; 

typedef enum
{
    VEML7700_ALS_GAIN_1                                 = (0 << 11),
    VEML7700_ALS_GAIN_2                                 = (1 << 11),
    VEML7700_ALS_GAIN_1_8                               = (2 << 11),
    VEML7700_ALS_GAIN_1_4                               = (3 << 11),
            
    VEML7700_ALS_INTEGRATION_TIME_25MS                  = (12 << 6),
    VEML7700_ALS_INTEGRATION_TIME_50MS                  = (8 << 6),
    VEML7700_ALS_INTEGRATION_TIME_100MS                 = (0 << 6),
    VEML7700_ALS_INTEGRATION_TIME_200MS                 = (1 << 6),
    VEML7700_ALS_INTEGRATION_TIME_400MS                 = (2 << 6),
    VEML7700_ALS_INTEGRATION_TIME_800MS                 = (3 << 6),
            
    VEML7700_ALS_PERSISTANT_PROTECT_NUMBER_1            = (0 << 4),
    VEML7700_ALS_PERSISTANT_PROTECT_NUMBER_2            = (1 << 4),
    VEML7700_ALS_PERSISTANT_PROTECT_NUMBER_4            = (2 << 4),
    VEML7700_ALS_PERSISTANT_PROTECT_NUMBER_8            = (3 << 4),
            
    VEML7700_ALS_INTERRUPT_ENABLE                       = (1 << 1),
    VEML7700_ALS_INTERRUPT_DISABLE                      = (0 << 1),
            
    VEML7700_ALS_SHUT_DOWN                              = (1 << 0),
    VEML7700_ALS_POWER_ON                               = (0 << 0)
}VEML7700_ALS_CONFIG;

typedef struct
{
    // Write only registers (addresses 0..3)
    uint16_t                ambient_light_sensor_config;
    uint16_t                ambient_light_sensor_high_threshold;
    uint16_t                ambient_light_sensor_low_threshold;
    uint16_t                power_saving;
    // Read only registers (addresses 4..6)
    uint16_t                ambient_light_sensor;
    uint16_t                white;
    uint16_t                interrupt_trigger_event;
} VEML7700_REGS;

typedef struct
{
    bool                    is_init_done;
    I2C_PARAMS              i2c_params;
    I2C_FUNCTIONS           i2c_functions;
    VEML7700_REGS           registers;
} VEML7700_CONFIG;

#define VEML7700_INSTANCE(_name, _i2c_module, _periodic_time)                               \
{                                                                                           \
    .is_init_done = false,                                                                  \
    .i2c_params = I2C_PARAMS_INSTANCE(_i2c_module, 0x10, false, (uint8_t*) &_name.registers, _periodic_time, 0), \
    .i2c_functions =                                                                        \
    {                                                                                       \
        .flags = 0,                                                                         \
        .active_function = 0xff,                                                            \
        .maximum_functions = VEML7700_FLAG_NUMBERS,                                         \
        .functions_tab =                                                                    \
        {                                                                                   \
            I2C_DEVICE_ADDRESS_W(_name, VEML7700_ADDR_ALS_CONFIG, ambient_light_sensor_config, 1*sizeof(uint16_t)),                 \
            I2C_DEVICE_ADDRESS_W(_name, VEML7700_ADDR_ALS_HIGH_THRESHOLD, ambient_light_sensor_high_threshold, 1*sizeof(uint16_t)), \
            I2C_DEVICE_ADDRESS_W(_name, VEML7700_ADDR_ALS_LOW_THRESHOLD, ambient_light_sensor_low_threshold, 1*sizeof(uint16_t)),   \
            I2C_DEVICE_ADDRESS_R(_name, VEML7700_ADDR_ALS, ambient_light_sensor, 1*sizeof(uint16_t)),                               \
            I2C_DEVICE_ADDRESS_R(_name, VEML7700_ADDR_WHITE, white, 1*sizeof(uint16_t)),                                            \
            I2C_DEVICE_ADDRESS_R(_name, VEML7700_ADDR_TRIGGER_EVENT, interrupt_trigger_event, 1*sizeof(uint16_t))                   \
        }                                                                                   \
    },                                                                                      \
    .registers =                                                                            \
    {                                                                                       \
        .ambient_light_sensor_config = 0x0001,                                              \
        .ambient_light_sensor_high_threshold = 0,                                           \
        .ambient_light_sensor_low_threshold = 0,                                            \
        .power_saving =  0,                                                                 \
        .ambient_light_sensor =  0,                                                         \
        .white = 0,                                                                         \
        .interrupt_trigger_event = 0                                                        \
    }                                                                                       \
}

#define VEML7700_DEF(_name, _i2c_module, _periodic_time)                                    \
static VEML7700_CONFIG _name = VEML7700_INSTANCE(_name, _i2c_module, _periodic_time)

uint8_t e_veml7700_deamon(VEML7700_CONFIG *var);

#define e_veml7700_is_flag_empty(var)                   ((var.i2c_functions.flags > 0) ? 0 : 1)

#define e_veml7700_set_config(var, val)                 (var.registers.ambient_light_sensor_config = val, SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_SET_CONFIG))
#define e_veml7700_set_high_threshold(var, val)         (var.registers.ambient_light_sensor_high_threshold = val, SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_SET_HIGH_THRESHOLD))
#define e_veml7700_set_low_threshold(var, val)          (var.registers.ambient_light_sensor_low_threshold = val, SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_SET_LOW_THRESHOLD))

#define e_veml7700_read_als(var)                        (SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_GET_AMBIENT_LIGHT_SENSOR))
#define e_veml7700_read_white(var)                      (SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_GET_WHITE))

#define e_veml7700_get_trigger_event(var)               (SET_BIT(var.i2c_functions.flags, VEML7700_FLAG_GET_TRIGGER_EVENT))

#endif
