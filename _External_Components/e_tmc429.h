#ifndef __DEF_TMC429
#define	__DEF_TMC429

#define READ_REGISTER           0x01000000
#define WRITE_REGISTER          0xFEFFFFFF

#define COMMON                  0
#define MOTOR_0                 0
#define MOTOR_1                 1
#define MOTOR_2                 2

#define IDX_X_TARGET            0x00000000
#define IDX_X_ACTUAL            0x02000000
#define IDX_V_MIN               0x04000000
#define IDX_V_MAX               0x06000000
#define IDX_V_TARGET            0x08000000
#define IDX_V_ACTUAL            0x0A000000
#define IDX_A_MAX               0x0C000000
#define IDX_A_ACTUAL            0x0E000000
#define IDX_A_THRESHOLD         0x10000000
#define IDX_PMUL_PDIV           0x12000000
#define IDX_REFCONF_RM          0x14000000
#define IDX_IMASK_IFLAGS        0x16000000
#define IDX_PULSEDIV_RAMPDIV    0x18000000
#define IDX_DX_REF_TOLERANCE    0x1A000000
#define IDX_X_LATCHED           0x1C000000
#define IDX_USTEP_COUNT         0x1E000000

#define IDX_DATAGRAM_LOW        0x60000000
#define IDX_DATAGRAM_HIGH       0x62000000
#define IDX_COVER_POS_LEN       0x64000000
#define IDX_COVER_DATAGRAM      0x66000000
#define IDX_IF_CONFIGURATION    0x68000000
#define IDX_POS_COMP            0x6A000000
#define IDX_IM                  0x6C000000
#define IDX_POWER_DOWN          0x70000000
#define IDX_TYPE_VERSION        0x72000000
#define IDX_SWITCHS             0x7C000000
#define IDX_SMGP                0x7E000000

#define IDX_RAM                 0x80000000

typedef enum
{
    FULL_STEP                   = 1,
    U_STEP2                     = 2,
    U_STEP4                     = 4,
    U_STEP8                     = 8,
    U_STEP16                    = 16,
    U_STEP32                    = 32,
    U_STEP64                    = 64,
}USRS_CONFIG;

typedef enum
{
    I_100                       = 0,    // 100% of Is
    I_12_5                      = 1,    // 12.5% of Is
    I_25                        = 2,    // 25% of Is
    I_37_5                      = 3,    // 37.5% of Is
    I_50                        = 4,    // 50% of Is
    I_62_5                      = 5,    // 62.5% of Is
    I_75                        = 6,    // 75% of Is
    I_87_5                      = 7,    // 87.5% of Is
}CURRENT_CONFIG;

typedef enum
{
    // Default mode for positioning applications with trapezoidal ramp.
    RAMP_MODE                   = 0,
    // Similar to ramp_mode but with soft target position approaching. The target position is approached
    // with exponentially reduced velocity.Usefull for applications where vibrations at the target position have to be minimized.
    SOFT_MODE                   = 1,
    // For velocity control applications, change of velocities with linear ramp.
    VELOCITY_MODE               = 2,
    // The velocity is controlled by the microcontroller, motion parameters limits are ignored. This mode is povided for motion control
    // applications, where the ramp generation es completely controlled by the microcontroler.
    HOLD_MODE                   = 3,
}RAMPMODE_CONFIG;

typedef enum
{
    // The motor will be stopped when the velocity is negative (V_ACTUAL < 0) and the left reference switch becomes active.
    DISABLE_STOP_L              = 0x00000100,
    // Left reference switch is disabled as an automatic stop switch.
    ENABLE_STOP_L               = 0x00000000,
    // Stops a motor if the velocity is positive (V_ACTUAL > 0) and the right reference switch becomes active.
    DISABLE_STOP_R              = 0x00000200,
    // Right reference switch is disabled as an automatic stop switch.
    ENABLE_STOP_R               = 0x00000000,
    // Stopping takes place in consideration of motion parameter limits; stops with linear ramp.
    SOFT_STOP                   = 0x00000400,
    // Stopping takes place immediately; motion parameter limits are ignored.
    HARD_STOP                   = 0x00000000,
}REFCONF_CONFIG;

typedef enum
{
    // Invert common polarity for all reference switches. If this bit is set, a low level on the input signals an active reference switch.
    IF_INV_REF                  = 0x00000001,
    REF_SWITCH_ACTIVE_HIGH      = 0x00000000,
    REF_SWITCH_ACTIVE_LOW       = 0x00000001,
    // Map internal non multiplexed interrupt status to nINT_SDO_C (needs SDOZ_C as SDO_C for readback information from the TMC429 to
    // the microcontroller). With SDO_INT = 1 the nINT_SDO_C is a non multiplexed nINToutput to the microcontroller.
    IF_SDO_INT                  = 0x00000002,
    // Toggle on each step pulse (this halfs the step frequency, both pulse edges represent steps). step_half reduces the required step
    // pulse bandwidth and is useful it for low-bandwidth optocouplers. This function can be used for the TMC262 stepper driver.
    IF_STEP_HALF                = 0x00000004,
    // Invert step pulse polarity. This configuration can be used for adaption of the step polarity to external driver stages.
    IF_INV_STP                  = 0x00000008,
    // Invert step pulse polarity. This is for adaption to external driver stages. Alternatively, this can be used as a shaft bit
    // to adjust the direction of motion for a motor, but do not use this as a direction bit because it has no effect on the internal
    // handling of signs (X_ACTUAL, V_ACTUAL?).
    IF_INV_DIR                  = 0x00000010,
    // Enable Step/Dir. This bit switches the driver to Step/Dir mode. If this flag is 0, it operates in SPI mode.
    IF_EN_SD                    = 0x00000020,
    // Select one motor out of three motors for the position compare function output of the TMC429 named POSCMP.
    IF_POS_COMP_MOT0            = 0x00000000,
    IF_POS_COMP_MOT1            = 0x00000040,
    IF_POS_COMP_MOT2            = 0x00000080,
    // Enable TMC429 reference inputs REFR1, REFR2, REFR3. As a default, the right reference inputs are disabled (EN_REFR=0).
    IF_EN_REFR                  = 0x00000100,
}IF_CONFIG;

typedef enum
{
    // Mot1R: only used with TMC429-I. Three available reference switches can be adjusted.
    SMGP_MOT1R                  = 0x00200000,
    // Refmux: used for multiplexing (with 74HC157) the reference switch inputs of the TMC429-I.
    SMGP_REFMUX                 = 0x00100000,
    // Continuous update: enable if a continuous update of the datagrams to the motor driver chain is necessary and an external multiplexer is used.
    SMGP_CONTINUOUS_UPDATE      = 0x00010000,
    // ComInd: Used to define either if a single CS signal nSCS_S is used in common for all stepper motor driver chips or if three CS signals
    // nSCS_S, nSCS2, nSCS3 are used to select stepper motor driver chips individually.
    SMGP_CS_COMMON_INDIVIDUAL   = 0x00000080,
    // LSMD: Number of stepper motor used
    SMGP_1_STEPPER_MOTOR        = 0x00000000,
    SMGP_2_STEPPER_MOTOR        = 0x00000001,
    SMGP_3_STEPPER_MOTOR        = 0x00000002,
}SMGP_CONFIG;

typedef enum
{
    SM_TMC429_HOME = 0,                 // Higher priority

    SM_TMC429_INIT,
    SM_TMC429_SET_GLOBAL_PARAMS,
    SM_TMC429_RESET_POSITION_0,
    SM_TMC429_RESET_POSITION_1,
    SM_TMC429_RESET_POSITION_2,
    SM_TMC429_HARD_STOP_0,
    SM_TMC429_HARD_STOP_1,
    SM_TMC429_HARD_STOP_2,
    SM_TMC429_SOFT_STOP_0,
    SM_TMC429_SOFT_STOP_1,
    SM_TMC429_SOFT_STOP_2,
    SM_TMC429_SET_PARAM_MOTOR_0,
    SM_TMC429_SET_PARAM_MOTOR_1,
    SM_TMC429_SET_PARAM_MOTOR_2,
    SM_TMC429_SET_TARGET_POSITION_0,
    SM_TMC429_SET_TARGET_POSITION_1,
    SM_TMC429_SET_TARGET_POSITION_2,
    SM_TMC429_SET_TARGET_VELOCITY_0,
    SM_TMC429_SET_TARGET_VELOCITY_1,
    SM_TMC429_SET_TARGET_VELOCITY_2,
    SM_TMC429_GET_DYNAMIC_PARAMETERS,   // Lower priority
            
    SM_TMC429_END
}TMC429_SM;

typedef struct
{
    uint32_t   x_target;
    uint32_t   x_actual;
    uint32_t   v_min;
    uint32_t   v_max;
    uint32_t   v_target;
    uint32_t   v_actual;
    uint32_t   a_max;
    uint32_t   a_actual;
    uint32_t   a_threshold;
    uint32_t   pmul_pdiv;
    uint32_t   refconf_rm;
    uint32_t   interrupt_mask_flag;
    uint32_t   pulse_ramp_usrs;
    uint32_t   dx_ref_tolerance;
    uint32_t   x_latched;
    uint32_t   ustep_count_429;
} TMC429_STEPPER_REGISTERS;

typedef struct
{
    uint32_t   datagram_low_word;
    uint32_t   datagram_high_word;
    uint32_t   cover_position_len;
    uint32_t   cover_datagram;
    uint32_t   if_configuration_429;
    uint32_t   pos_comp_429;
    uint32_t   im;
    uint32_t   power_down;
    uint32_t   type_version_429;
    uint32_t   switchs;
    uint32_t   smgp;   // Stepper Motor Global Parameter
} TMC429_COMMON_REGISTERS;

typedef struct
{
    TMC429_STEPPER_REGISTERS    stepper[3];
    TMC429_COMMON_REGISTERS     common;
    uint32_t                    status;
} TMC429_REGISTERS;

typedef struct
{
    SPI_PARAMS                  spi_params;
    TMC429_REGISTERS            registers;
} TMC429_CONFIG;

#define TMC429_COMMON_REGISTERS_INSTANCE(_shaft, _ref_switch_pol)   \
{                                                                   \
    .datagram_low_word = 0,                                         \
    .datagram_high_word = 0,                                        \
    .cover_position_len = 0,                                        \
    .cover_datagram = 0,                                            \
    .if_configuration_429 = IDX_IF_CONFIGURATION | IF_EN_REFR | (_ref_switch_pol & 0x00000001),                     \
    .pos_comp_429 = 0,                                              \
    .im = 0,                                                        \
    .power_down = 0,                                                \
    .type_version_429 = 0,                                          \
    .switchs = 0,                                                   \
    .smgp =  IDX_SMGP | SMGP_CONTINUOUS_UPDATE | 0x00000700 | SMGP_3_STEPPER_MOTOR | ((_shaft << 4) & 0x00000010)   \
}

#define TMC429_REGISTERS_INSTANCE(_shaft, _ref_switch_pol)                  \
{                                                                           \
    .stepper = {0},                                                         \
    .common = TMC429_COMMON_REGISTERS_INSTANCE(_shaft, _ref_switch_pol),    \
    .status = 0,                                                            \
}

#define TMC429_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time, _shaft, _ref_switch_pol) \
{                                                                                               \
    .spi_params = SPI_PARAMS_INSTANCE(_spi_module, _io_port, _io_indice, _periodic_time, 6),    \
    .registers = TMC429_REGISTERS_INSTANCE(_shaft, _ref_switch_pol),                                                                           \
}

#define TMC429_DEF(_name, _spi_module, _cs_pin, _periodic_time, _shaft, _ref_switch_pol)    \
static TMC429_CONFIG _name = TMC429_INSTANCE(_spi_module, _XBR(_cs_pin), _IND(_cs_pin), _periodic_time, _shaft, _ref_switch_pol)

void eTMC429Deamon(TMC429_CONFIG *var);
uint8_t eTMC429SetMotorParam(TMC429_CONFIG *var, uint32_t motor, uint16_t stepper_resolution, uint8_t resolution, uint32_t refConf, uint32_t desire_dps_fs, uint32_t time_acc_ms, uint32_t irun, uint32_t ihold, uint32_t rampmode);
uint32_t eTMC429GetRealVelocity(TMC429_CONFIG var, uint32_t motor, uint16_t stepper_resolution);
uint32_t eTMC429GetRealAcceleration(TMC429_CONFIG var, uint32_t motor);

#define eTMC429HardStop(var, motor)                             SET_BIT(var.spi_params.flags, (SM_TMC429_HARD_STOP_0 + motor))
#define eTMC429SoftStop(var, motor)                             SET_BIT(var.spi_params.flags, (SM_TMC429_SOFT_STOP_0 + motor))
#define eTMC429GetDynamicParameters(var)                        SET_BIT(var.spi_params.flags, SM_TMC429_GET_DYNAMIC_PARAMETERS)
#define eTMC429ResetPosition(var, motor)                        SET_BIT(var.spi_params.flags, (SM_TMC429_RESET_POSITION_0 + motor))
#define eTMC429SetTargetPosition(var, motor, position)          (var.registers.x_target[motor] = IDX_X_TARGET | (motor << 29) | (position & 0xFFFFFF), SET_BIT(var.spi_params.flags, (SM_TMC429_SET_TARGET_POSITION_0 + motor)))
#define eTMC429SetTargetVelocity(var, motor, velocity)          (var.registers.v_target[motor] = IDX_V_TARGET | (motor << 29) | (velocity & 0xFFF), SET_BIT(var.spi_params.flags, (SM_TMC429_SET_TARGET_VELOCITY_0 + motor)))
#define eTMC429SetGlobalParam(var, shaft, refSwitchPolarity)    (var.registers.smgp = IDX_SMGP | SMGP_CONTINUOUS_UPDATE | 0x00000700 | SMGP_3_STEPPER_MOTOR | ((shaft << 4) & 0x00000010), var.registers.if_configuration_429 = IDX_IF_CONFIGURATION | IF_EN_REFR | (refSwitchPolarity & 0x00000001), SET_BIT(var.spi_params.flags, SM_TMC429_SET_GLOBAL_PARAMS))


#endif
