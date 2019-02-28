#ifndef __DEF_E_PCA9685
#define	__DEF_E_PCA9685

#define PCA9685_FREQ_MIN                                255
#define PCA9685_FREQ_50HZ                               121
#define PCA9685_FREQ_100HZ                              60 
#define PCA9685_FREQ_200HZ                              30
#define PCA9685_FREQ_300HZ                              19
#define PCA9685_FREQ_400HZ                              14
#define PCA9685_FREQ_500HZ                              11
#define PCA9685_FREQ_600HZ                              9
#define PCA9685_FREQ_750HZ                              7
#define PCA9685_FREQ_850HZ                              6
#define PCA9685_FREQ_1000HZ                             5
#define PCA9685_FREQ_1200HZ                             4
#define PCA9685_FREQ_1500HZ                             3
#define PCA9685_FREQ_MAX                                3

typedef enum
{
    PCA9685_FLAG_RESET                                  = 0,
    PCA9685_FLAG_SLEEP_MODE,
    PCA9685_FLAG_SET_PRESCALE,
    PCA9685_FLAG_NORMAL_MODE,
    PCA9685_FLAG_SET_MODES,    
    PCA9685_FLAG_CONFIG_SUB_ADDRESS,
    PCA9685_FLAG_SET_OUTPUTS,
    PCA9685_FLAG_GET_MODES,
            
    PCA9685_FLAG_NUMBERS
} PCA9685_FLAGS;

typedef enum
{
    PCA9685_ADDR_RESET                                  = 256,
    PCA9685_ADDR_SLEEP_MODE                             = 0,
    PCA9685_ADDR_SET_PRESCALE                           = 254,
    PCA9685_ADDR_NORMAL_MODE                            = 0,
    PCA9685_ADDR_SET_MODES                              = 0,    
    PCA9685_ADDR_CONFIG_SUB_ADDRESS                     = 2,
    PCA9685_ADDR_SET_OUTPUTS                            = 6,
    PCA9685_ADDR_GET_MODES                              = 0
} PCA9685_REGISTERS_ADDRESS;

typedef enum
{
    PCA9685_SIZE_RESET                                  = 1,
    PCA9685_SIZE_SLEEP_MODE                             = 1,
    PCA9685_SIZE_SET_PRESCALE                           = 1,
    PCA9685_SIZE_NORMAL_MODE                            = 1,
    PCA9685_SIZE_SET_MODES                              = 2,    
    PCA9685_SIZE_CONFIG_SUB_ADDRESS                     = 3,
    PCA9685_SIZE_SET_OUTPUTS                            = 64,
    PCA9685_SIZE_GET_MODES                              = 2
} PCA9685_REGISTERS_SIZE;

typedef enum
{
    PCA9685_RESTART                                     = 0x80,
            
    PCA9685_USE_INTERNAL_CLOCK                          = 0x00,
    PCA9685_USE_EXTERNAL_CLOCK                          = 0x40,
            
    PCA9685_ENABLE_AUTO_INCREMENT_REG                   = 0x20,
    PCA9685_DISABLE_AUTO_INCREMENT_REG                  = 0x00,
            
    PCA9685_PUT_IN_NORMAL_MODE                          = 0x00,
    PCA9685_PUT_IN_SLEEP_MODE                           = 0x10,
            
    PCA9685_ENABLE_SUBADR1_RESPONSE                     = 0x08,
    PCA9685_ENABLE_SUBADR2_RESPONSE                     = 0x04,
    PCA9685_ENABLE_SUBADR3_RESPONSE                     = 0x02,
            
    PCA9685_ENABLE_LED_ALL_CALL_ADDR                    = 0x01,
    PCA9685_DISABLE_LED_ALL_CALL_ADDR                   = 0x00
} PCA9685_MODE1_PARAMS;

typedef enum
{    
    PCA9685_OUTPUT_LOGIC_STATE_NORMAL                   = 0x00,     // Output logic state not inverted.
    PCA9685_OUTPUT_LOGIC_STATE_INV                      = 0x10,     // Output logic state inverted.
            
    PCA9685_UPDATE_OUTPUT_ON_STOP_CMD                   = 0x00,     // Outputs change on STOP command.
    PCA9685_UPDATE_OUTPUT_ON_ACK                        = 0x08,     // Outputs change on ACK.
            
    PCA9685_OUTPUTS_ARE_OPEN_DRAIN                      = 0x00,     // The 16 LEDn outputs are configured with an open-drain structure
    PCA9685_OUTPUTS_ARE_DRIVE                           = 0x04      // The 16 LEDn outputs are configured with a totem pole structure.
} PCA9685_MODE2_PARAMS;

typedef struct
{
    uint16_t                phase;
    uint16_t                duty_cycle;
} PCA9685_PWM;

typedef struct
{
    uint8_t                 mode1;
    uint8_t                 mode2;
    uint8_t                 sub_address_1;
    uint8_t                 sub_address_2;
    uint8_t                 sub_address_3;
    uint8_t                 all_call_address;
    PCA9685_PWM             output[16];
    uint8_t                 __reserved_do_not_used__[180];
    PCA9685_PWM             output_all;
    uint8_t                 prescale;
    uint8_t                 test_mode;
    const uint8_t           _internal_register_reset;
} PCA9685_REGS;

typedef struct
{
    bool                    is_init_done;
    I2C_PARAMS              i2c_params;
    I2C_FUNCTIONS           i2c_functions;
    PCA9685_REGS            registers;
} PCA9685_CONFIG;

#define PCA9685_INSTANCE(_name, _i2c_module, _i2c_address, _periodic_time)                  \
{                                                                                           \
    .is_init_done = false,                                                                  \
    .i2c_params = I2C_PARAMS_INSTANCE(_i2c_module, _i2c_address, false, (uint8_t*) &_name.registers.mode1, _periodic_time, 0), \
    .i2c_functions =                                                                        \
    {                                                                                       \
        .flags = 0,                                                                         \
        .active_function = 0xff,                                                            \
        .maximum_functions = PCA9685_FLAG_NUMBERS,                                          \
        .functions_tab =                                                                    \
        {                                                                                   \
            {I2C_GENERAL_CALL_ADDRESS, I2C_WRITE, PCA9685_ADDR_RESET, PCA9685_SIZE_RESET},                      \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_SLEEP_MODE, PCA9685_SIZE_SLEEP_MODE},                  \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_SET_PRESCALE, PCA9685_SIZE_SET_PRESCALE},              \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_NORMAL_MODE, PCA9685_SIZE_NORMAL_MODE},                \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_SET_MODES, PCA9685_SIZE_SET_MODES},                    \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_CONFIG_SUB_ADDRESS, PCA9685_SIZE_CONFIG_SUB_ADDRESS},  \
            {I2C_DEVICE_ADDRESS, I2C_WRITE, PCA9685_ADDR_SET_OUTPUTS, PCA9685_SIZE_SET_OUTPUTS},                \
            {I2C_DEVICE_ADDRESS, I2C_READ, PCA9685_ADDR_GET_MODES, PCA9685_SIZE_GET_MODES}                      \
        }                                                                                   \
    },                                                                                      \
    .registers =                                                                            \
    {                                                                                       \
        .mode1 = 0x11,                                                                      \
        .mode2 = 0x04,                                                                      \
        .sub_address_1 = 0xe2,                                                              \
        .sub_address_2 =  0xe4,                                                             \
        .sub_address_3 =  0xe8,                                                             \
        .all_call_address = 0xe0,                                                           \
        .output = {0},                                                                      \
        .__reserved_do_not_used__ = {0},                                                    \
        .output_all = {0},                                                                  \
        .prescale = 0x1e,                                                                   \
        .test_mode = 0x00,                                                                  \
        ._internal_register_reset = 0x06                                                    \
    }                                                                                       \
}

#define PCA9685_DEF(_name, _i2c_module, _i2c_address, _periodic_time)                       \
static PCA9685_CONFIG _name = PCA9685_INSTANCE(_name, _i2c_module, _i2c_address, _periodic_time)

uint8_t e_pca9685_deamon(PCA9685_CONFIG *var);

// Reset command should use a General Call Address (0x00) + Write request follow by a data = 0x06 and then a stop condition (reset occurs after stop condition)
// Be careful the reset command does not reinitialize the data 'registers' in the PIC32.
#define e_pca9685_reset(var)                                            (SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_RESET))                       
// Change frequency: 1. Put in sleep mode then 2. Change PRESCALE then 3. Put in normal mode
#define e_pca9685_set_frequency(var, val)                               (var.registers.prescale = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SLEEP_MODE), SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_PRESCALE), SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_NORMAL_MODE))

#define e_pca9685_config_mode1(var, val)                                (var.registers.mode1 = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_MODES))
#define e_pca9685_config_mode2(var, val)                                (var.registers.mode2 = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_MODES))
#define e_pca9685_read_modes(var)                                       (SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_GET_MODES))

#define e_pca9685_config_sub_address_1(var, val)                        (var.registers.sub_address_1 = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_CONFIG_SUB_ADDRESS))
#define e_pca9685_config_sub_address_2(var, val)                        (var.registers.sub_address_2 = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_CONFIG_SUB_ADDRESS))
#define e_pca9685_config_sub_address_3(var, val)                        (var.registers.sub_address_3 = val, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_CONFIG_SUB_ADDRESS))

#define e_pca9685_set_pwm(var, id, _duty_cycle)                         (var.registers.output[id].phase = 0, var.registers.output[id].duty_cycle = (_duty_cycle & 0x0fff), SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_OUTPUTS))
#define e_pca9685_set_pwm_with_phase(var, id, _phase, _duty_cycle)      (var.registers.output[id].phase = (_phase & 0x0fff), var.registers.output[id].duty_cycle = ((_phase + _duty_cycle)&0x0fff), SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_OUTPUTS))

#define e_pca9685_output_off(var, id)                                   (var.registers.output[id].phase = 0x0000, var.registers.output[id].duty_cycle = 0x1000, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_OUTPUTS))
#define e_pca9685_output_on(var, id)                                    (var.registers.output[id].phase = 0x1000, var.registers.output[id].duty_cycle = 0x0000, SET_BIT(var.i2c_functions.flags, PCA9685_FLAG_SET_OUTPUTS))

#endif

