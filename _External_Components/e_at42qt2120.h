#ifndef __DEF_E_AT42QT2120
#define	__DEF_E_AT42QT2120

typedef enum
{
    AT42QT2120_FLAG_RESET                               = 0,
    AT42QT2120_FLAG_CALIBRATE,
    AT42QT2120_FLAG_SET_PARAMS,
    AT42QT2120_FLAG_SET_KEY_DETECT_THRESHOLD_0_TO_11,
    AT42QT2120_FLAG_SET_KEY_CONTROL_0_TO_11,
    AT42QT2120_FLAG_SET_KEY_PULSE_SCALE_0_TO_11,            
    AT42QT2120_FLAG_GET_KEY_SIGNAL,
    AT42QT2120_FLAG_GET_REF_DATA,
    AT42QT2120_FLAG_GET_STATUS_KEYS_AND_SLIDER,
            
    AT42QT2120_FLAG_NUMBERS,
    AT42QT2120_FLAG_NO_FUNCTION                         = 0xff
} AT42QT2120_FLAGS;

typedef enum
{
    AT42QT2120_ADDR_RESET                               = 7,
    AT42QT2120_ADDR_CALIBRATE                           = 6,
    AT42QT2120_ADDR_SET_PARAMS                          = 8,
    AT42QT2120_ADDR_SET_KEY_DETECT_THRESHOLD_0_TO_11    = 16,
    AT42QT2120_ADDR_SET_KEY_CONTROL_0_TO_11             = 28,
    AT42QT2120_ADDR_SET_KEY_PULSE_SCALE_0_TO_11         = 40,
    AT42QT2120_ADDR_GET_KEY_SIGNAL                      = 52,
    AT42QT2120_ADDR_GET_REF_DATA                        = 76,
    AT42QT2120_ADDR_GET_STATUS_KEYS_AND_SLIDER          = 2            
} AT42QT2120_REGISTERS_ADDRESS;

typedef enum
{
    AT42QT2120_KEY_0                                    = 0,
    AT42QT2120_KEY_1,
    AT42QT2120_KEY_2,
    AT42QT2120_KEY_3,
    AT42QT2120_KEY_4,
    AT42QT2120_KEY_5,
    AT42QT2120_KEY_6,
    AT42QT2120_KEY_7,
    AT42QT2120_KEY_8,
    AT42QT2120_KEY_9,
    AT42QT2120_KEY_10,
    AT42QT2120_KEY_11,
} AT42QT2120_KEYS;

typedef enum
{
    AT42QT2120_NO_AKS                                   = 0x00,
    AT42QT2120_AKS_GROUP_1                              = 0x04,
    AT42QT2120_AKS_GROUP_2                              = 0x08,
    AT42QT2120_AKS_GROUP_3                              = 0x0e,
} AT42QT2120_AKS_GROUP;

typedef struct
{
    // Read only registers (addresses 0..5)
    uint8_t                 chip_id;                    // Always 0x3e
    uint8_t                 firmware_version;           // Major version (4 MSB bits) - Minor version (4 LSB bits)
    uint8_t                 detection_status;           // CALIBRATE (this bit is set during a calibration sequence) | OVERFLOW (this bit is set if the time to acquire all key signals exceeds 16 ms) | - | - | - | - | SDET (slider detection) | TDET (touch key detection)
    uint8_t                 keys0to7_status;            // Key 7 | Key 6 | Key 5 | Key 4 | Key 3 | Key 2 | Key 1 | Key 0
    uint8_t                 keys8to11_status;           // - | - | - | - | Key 11 | Key 10 | Key 9 | Key 8 
    uint8_t                 slider_position;            // Value 0..255
    // Read/Write registers (addresses 6..51)
    uint8_t                 calibrate;                  // A value other than 0 triggers the calibration
    uint8_t                 reset;                      // A value other than 0 triggers the reset
    uint8_t                 low_power_mode;             // This 8-bits value determines the number of 16 ms intervals between key measurements. (0: power down, 1: 16ms, 2: 32ms, ..., 255: 4,08s)
    uint8_t                 towards_touch_drift;    
    uint8_t                 away_from_touch_drift;
    uint8_t                 detection_integrator;       // This 8-bits value controls the number of consecutive measurements that must be confirmed as having passed the key threshold before that key is registered as being in detect. Range: 1..32
    uint8_t                 touch_recall_delay;         // If an object unintentionally contacts a key resulting in a detection for a prolonged interval it is usually desirable to re calibrate the key in order to restore its function, perhaps after a time delay of some seconds.
                                                        // The TRD timer monitors such detections; if a detection event exceeds the timer's setting, the key will be automatically re calibrated. After a re calibration has taken place, the affected key will once again function normally even if it is still being contacted by the foreign object. 
                                                        // TRD can be disabled by setting it to zero (infinite timeout) in which case the key will never autorecalibrate during a continuous detection.
                                                        // TRD is set globally, which can range in value from 1..255. TRD above 0 is expressed in 0,16s increments.
    uint8_t                 drift_hold_time;            // This is used to restrict drift on all keys while one or more keys are activated. DHT defines the length of time the drift is halted after a key detection. When DHT = 0, drifting is never suspended, even during a valid touch of another key. 
                                                        // This feature is particularly useful in cases of high-density keypads where touching a key or hovering a finger over the keypad would cause untouched keys to drift, and therefore create a sensitivity shift, and ultimately inhibit any touch detection. It is expressed in 0,16s increments.
    uint8_t                 slider_options;             // EN (enable slider/wheel on the first three channels) | WHEEL (0: slider / 1: wheel) | - | - | - | - | - | -
    uint8_t                 charge_time;
    uint8_t                 key_detect_threshold[12];   // These 8-bits values set the threshold value for each key to register a detection. Do not use a setting of 0 as this causes a key to go into detection when its signal is equal its reference.
    uint8_t                 key_control[12];            // - | - | - | GUARD | AKS.2 | AKS.1 | AKS.0 | GPO | EN
                                                        // GUARD: If set to 1, this key act as a guard channel. A key set as a guard key does not affect the Detection Status or Key Status register.
                                                        // AKS: These bits control which keys are included in an AKS group. There can be up to three groups, each containing any number of keys (up to the maximum allowed for the mode). A setting of 0 disables AKS for that key.
                                                        // Each key can have a value between 0 and 3, which assigns it ti an AKS group of that number. A key may only go into detect when it has the largest signal change of any key in its group. A value of 0 means the key is not in any AKS group.
                                                        // GPO: If set to 0, this key is a driven low output. If set to 1 then the output is driven high. Setting this bit only has effect if the EN bit is set to 1.
                                                        // EN: If set to 0, indicates that this key is to be used as a touch channel. Setting this bit to 1 will disable the key for touch use and make the channel pin an output.
                                                        // It is not possible to enable the channels 0 and 1 as an output. It is recommended to manually initiate a calibration cycle after a change is made to the EN bit regardless of this. 
    uint8_t                 key_pulse_scale[12];        // PULSE.3 | PULSE.2 | PULSE.1 | PULSE.0 | SCALE.3 | SCALE.2 | SCALE.1 | SCALE.0
                                                        // The PULSE/SCALE settings are used to set up a proximity key. In normal mode the proximity key is set up by configuring a key's PULSE/SCALE settings via an I2C bus. 
                                                        // These bits represent two numbers; the low nibble is SCALE, high nibble is PULSE.
                                                        // Each acquisitions cycle consists signal accumulation and signal averaging. PULSE determines the number of measurements accumulated, SCALE the averaging factor.
                                                        // The SCALE factor (averaging factor) for the accumulated signal is an exponent of 2.
                                                        // PULSE is the number of measurements accumulated and is an exponent of 2.
    // Read only registers (addresses 52..99)
    uint8_t                 key_signal[24];             // Key signal 0 [0]: MSB / [1]: LSB, Key signal 1 [2]: MSB / [3]: LSB...
    uint8_t                 reference_data[24];         // Ref data 0 [0]: MSB / [1]: LSB, Ref data 1 [2]: MSB / [3]: LSB...
} AT42QT2120_REGS;

typedef struct
{
    bool                    is_init_done;
    I2C_PARAMS              i2c_params;
    I2C_FUNCTIONS           i2c_functions;
    AT42QT2120_REGS         registers;
} AT42QT2120_CONFIG;

#define AT42QT2120_INSTANCE(_name, _i2c_module, _periodic_time)                             \
{                                                                                           \
    .is_init_done = false,                                                                  \
    .i2c_params = I2C_PARAMS_INSTANCE(_i2c_module, 0x1c, false, (uint8_t*) &_name.registers, _periodic_time, 0), \
    .i2c_functions =                                                                        \
    {                                                                                       \
        .flags = 0,                                                                         \
        .active_function = 0xff,                                                            \
        .maximum_functions = AT42QT2120_FLAG_NUMBERS,                                       \
        .functions_tab =                                                                    \
        {                                                                                   \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_RESET, reset, 1*sizeof(uint8_t)),                                               \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_CALIBRATE, calibrate, 1*sizeof(uint8_t)),                                       \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_SET_PARAMS, low_power_mode, 7*sizeof(uint8_t)),                                 \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_SET_KEY_DETECT_THRESHOLD_0_TO_11, key_detect_threshold, 12*sizeof(uint8_t)),    \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_SET_KEY_CONTROL_0_TO_11, key_control, 12*sizeof(uint8_t)),                      \
            I2C_DEVICE_ADDRESS_W(_name, AT42QT2120_ADDR_SET_KEY_PULSE_SCALE_0_TO_11, key_pulse_scale, 12*sizeof(uint8_t)),              \
            I2C_DEVICE_ADDRESS_R(_name, AT42QT2120_ADDR_GET_KEY_SIGNAL, key_signal, 24*sizeof(uint8_t)),                                \
            I2C_DEVICE_ADDRESS_R(_name, AT42QT2120_ADDR_GET_REF_DATA, reference_data, 24*sizeof(uint8_t)),                              \
            I2C_DEVICE_ADDRESS_R(_name, AT42QT2120_ADDR_GET_STATUS_KEYS_AND_SLIDER, detection_status, 4*sizeof(uint8_t))                \
        }                                                                                   \
    },                                                                                      \
    .registers =                                                                            \
    {                                                                                       \
        .chip_id = 0,                                                                       \
        .firmware_version = 0,                                                              \
        .detection_status = 0,                                                              \
        .keys0to7_status =  0,                                                              \
        .keys8to11_status =  0,                                                             \
        .slider_position = 0,                                                               \
        .calibrate = 0x00,                                                                  \
        .reset = 0x00,                                                                      \
        .low_power_mode = 0x01,                                                             \
        .towards_touch_drift = 0x14,                                                        \
        .away_from_touch_drift = 0x05,                                                      \
        .detection_integrator = 0x04,                                                       \
        .touch_recall_delay = 0xff,                                                         \
        .drift_hold_time = 0x19,                                                            \
        .slider_options = 0x00,                                                             \
        .charge_time = 0x00,                                                                \
        .key_detect_threshold = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},           \
        .key_control = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                                \
        .key_pulse_scale = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                            \
        .key_signal = {0},                                                                  \
        .reference_data = {0}                                                               \
    }                                                                                       \
}

#define AT42QT2120_DEF(_name, _i2c_module, _periodic_time)                                  \
static AT42QT2120_CONFIG _name = AT42QT2120_INSTANCE(_name, _i2c_module, _periodic_time)

uint8_t e_at42qt2120_deamon(AT42QT2120_CONFIG *var);

#define e_at42qt2120_is_flag_empty(var)                 ((var.i2c_functions.flags > 0) ? 0 : 1)

#define e_at42qt2120_reset(var)                         (var.registers.reset = 0x01, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_RESET))
#define e_at42qt2120_calibrate(var)                     (var.registers.calibrate = 0x01, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_CALIBRATE))
#define e_at42qt2120_get_status_keys_and_slider(var)    SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_GET_STATUS_KEYS_AND_SLIDER)

#define e_at42qt2120_disable_slider(var)                (var.registers.slider_options = 0x00, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_PARAMS))
#define e_at42qt2120_enable_slider(var)                 (var.registers.slider_options = 0x80, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_PARAMS))
#define e_at42qt2120_enable_wheel(var)                  (var.registers.slider_options = 0xc0, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_PARAMS))

#define e_at42qt2120_set_key_detect_threshold(var, indice, value)   (var.registers.key_detect_threshold[indice] = value, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_KEY_DETECT_THRESHOLD_0_TO_11))
#define e_at42qt2120_set_key_control(var, indice, value)            (var.registers.key_control[indice] = value, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_KEY_CONTROL_0_TO_11))
#define e_at42qt2120_set_key_pulse_scale(var, indice, value)        (var.registers.key_pulse_scale[indice] = value, SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_SET_KEY_PULSE_SCALE_0_TO_11))

#define e_at42qt2120_get_key_signal(var)                SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_GET_KEY_SIGNAL)
#define e_at42qt2120_get_ref_data(var)                  SET_BIT(var.i2c_functions.flags, AT42QT2120_FLAG_GET_REF_DATA)

#endif
