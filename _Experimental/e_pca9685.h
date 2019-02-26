#ifndef __DEF_E_PCA9685
#define	__DEF_E_PCA9685

typedef enum
{
    PCA9685_RESTART                     = 0x80,     // *
            
    PCA9685_USE_INTERNAL_CLOCK          = 0x00,     // *
    PCA9685_USE_EXTERNAL_CLOCK          = 0x40,
            
    PCA9685_ENABLE_AUTO_INCREMENT_REG   = 0x20,
    PCA9685_DISABLE_AUTO_INCREMENT_REG  = 0x00,
            
    PCA9685_PUT_IN_NORMAL_MODE          = 0x00,
    PCA9685_PUT_IN_SLEEP_MODE           = 0x10,     // *
            
    PCA9685_ENABLE_SUBADR1_RESPONSE     = 0x08,
    PCA9685_ENABLE_SUBADR2_RESPONSE     = 0x04,
    PCA9685_ENABLE_SUBADR3_RESPONSE     = 0x02,
    PCA9685_ENABLE_LED_ALL_CALL_ADR     = 0x01      // *
} PCA9685_MODE1_PARAMS;

typedef enum
{    
    PCA9685_OUTPUT_LOGIC_STATE_NORMAL   = 0x00,
    PCA9685_OUTPUT_LOGIC_STATE_INV      = 0x10,
            
    PCA9685_UPDATE_OUTPUT_ON_STOP_CMD   = 0x00,
    PCA9685_UPDATE_OUTPUT_ON_ACK        = 0x08,
            
    PCA9685_OUTPUTS_ARE_OPEN_DRAIN      = 0x00,
    PCA9685_OUTPUTS_ARE_DRIVE           = 0x04
} PCA9685_MODE2_PARAMS;

typedef struct
{
    uint16_t    phase;
    uint16_t    dc;
} PCA9685_PWM;

typedef struct
{
    uint8_t     MODE1;
    uint8_t     MODE2;
    uint8_t     SUBADR1;
    uint8_t     SUBADR2;
    uint8_t     SUBADR3;
    uint8_t     ALLCALLADR;
    PCA9685_PWM PWM[16];
    uint8_t     PRESCALE;
    uint8_t     TEST_MODE;
} PCA9685_REGS;

typedef struct
{
    bool                    is_init_done;
    I2C_PARAMS              i2c_params;
    PCA9685_REGS            registers;
} PCA9685_CONFIG;

#define PCA9685_INSTANCE(_name, _i2c_module, _i2c_address, _periodic_time)                  \
{                                                                                           \
    .is_init_done = false,                                                                  \
    .i2c_params = I2C_PARAMS_INSTANCE(_i2c_module, _i2c_address, false, (uint8_t*) &_name.registers.MODE1, _periodic_time, 0),  \
    .registers =                                                                            \
    {                                                                                       \
        .MODE1 = (PCA9685_USE_INTERNAL_CLOCK | PCA9685_ENABLE_AUTO_INCREMENT_REG | PCA9685_PUT_IN_NORMAL_MODE),                 \
        .MODE2 = (PCA9685_OUTPUT_LOGIC_STATE_NORMAL | PCA9685_UPDATE_OUTPUT_ON_STOP_CMD | PCA9685_OUTPUTS_ARE_DRIVE),           \
        .SUBADR1 = 0,                                                                       \
        .SUBADR2 = 0,                                                                       \
        .SUBADR3 = 0,                                                                       \
        .ALLCALLADR = 0,                                                                    \
        .PWM = {0},                                                                         \
        .PRESCALE = 0,                                                                      \
        .TEST_MODE = 0                                                                      \
    }                                                                                       \
}

#define PCA9685_DEF(_name, _i2c_module, _i2c_address, _periodic_time)                       \
static PCA9685_CONFIG _name = PCA9685_INSTANCE(_name, _i2c_module, _i2c_address, _periodic_time)

#define e_pca9685_set_pwm(_var, _id, _duty_cycle)                       _var.registers.PWM[_id].phase = 0, _var.registers.PWM[_id].dc = (_duty_cycle & 0x0fff)
#define e_pca9685_set_pwm_with_phase(_var, _id, _phase, _duty_cycle)    _var.registers.PWM[_id].phase = (_phase & 0x0fff), _var.registers.PWM[_id].dc = ((_phase + _duty_cycle)&0x0fff)
#define e_pca9685_clear_output(_var, _id)                               _var.registers.PWM[_id].phase = 0x0000, _var.registers.PWM[_id].dc = 0x1000
#define e_pca9685_set_output(_var, _id)                                 _var.registers.PWM[_id].phase = 0x1000, _var.registers.PWM[_id].dc = 0x0000

void e_pca9685_deamon(PCA9685_CONFIG *var);

#endif

