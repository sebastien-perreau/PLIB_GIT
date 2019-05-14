#ifndef __DEF_E_TPS92662
#define	__DEF_E_TPS92662

#define TPS92662_MAX_TX_SIZE (1 + 1 + 1 + 32 + 2)
#define TPS92662_MAX_RX_SIZE (1 + 1 + 1 + 2 + 32 + 2)   // In the receip line we found the TX data and then the RX data

typedef enum
{
    SM_TPS92662_HOME = 0,
    SM_TPS92662_SEARCH,
            
    SM_TPS92662_READ_IC_IDENTIFIER,     // Upper priority
    SM_TPS92662_READ_ERRORS,
    SM_TPS92662_READ_ADCS,
    
    SM_TPS92662_WRITE_SYSTEM_CONFIG,
    SM_TPS92662_WRITE_SLEW_RATE,
    SM_TPS92662_WRITE_OVER_VOLTAGE_LIMIT,
    SM_TPS92662_WRITE_PARALLEL_LED_STRING,
    SM_TPS92662_WRITE_DEFAULT_PULSE_WIDTH,
    SM_TPS92662_WRITE_WATCHDOG_TIMER,
    SM_TPS92662_WRITE_PWM_TICK_PERIOD,
    SM_TPS92662_WRITE_ADC_ID,
    SM_TPS92662_WRITE_SOFTSYNC,
    SM_TPS92662_WRITE_PHASE_AND_WIDTH,
    SM_TPS92662_WRITE_PHASE,
    SM_TPS92662_WRITE_WIDTH,            // Lower priority
            
    SM_TPS92662_MAX_FLAGS
} TPS92662_SM;

typedef enum
{
    TPS_WRITE_1B                            = 0,
    TPS_WRITE_2B,
    TPS_WRITE_3B,
    TPS_WRITE_4B,
    TPS_WRITE_12B,
    TPS_WRITE_16B,
    TPS_WRITE_32B,
            
    TPS_READ_1B,
    TPS_READ_2B,
    TPS_READ_3B,
    TPS_READ_4B,
    TPS_READ_12B,
    TPS_READ_16B,
    TPS_READ_32B
} TPS92662_INIT_PARAM;

typedef enum
{
    TPS92662_ADDR_PHASE                     = 0x00,
    TPS92662_ADDR_WIDTH                     = 0x10,
    TPS92662_ADDR_SLEW_RATE                 = 0x70,
    TPS92662_ADDR_OVER_VOLTAGE_LIMIT        = 0x71,
    TPS92662_ADDR_PARALLEL_LED_STRING       = 0x72,
    TPS92662_ADDR_DEFAULT_PULSE_WIDTH       = 0x73,
    TPS92662_ADDR_SYSTEM_CONFIG             = 0x80,
    TPS92662_ADDR_WATCHDOG_TIMER            = 0x81,
    TPS92662_ADDR_PWM_TICK_PERIOD           = 0x82,
    TPS92662_ADDR_ADC_ID                    = 0x83,
    TPS92662_ADDR_SOFTSYNC                  = 0x84,
    TPS92662_ADDR_ADC                       = 0xa0,
    TPS92662_ADDR_ERRORS                    = 0xb0,
    TPS92662_ADDR_IC_IDENTIFIER             = 0xff
} TPS92662_ADDRESS_REGISTERS;

typedef enum
{
    TPS92662_SLEW_LED_STRING_12_10_SLOW     = 0x00,
    TPS92662_SLEW_LED_STRING_12_10_MEDIUM   = 0x01,
    TPS92662_SLEW_LED_STRING_12_10_FAST     = 0x03,
            
    TPS92662_SLEW_LED_STRING_09_07_SLOW     = 0x00,
    TPS92662_SLEW_LED_STRING_09_07_MEDIUM   = 0x04,
    TPS92662_SLEW_LED_STRING_09_07_FAST     = 0x0c,
            
    TPS92662_SLEW_LED_STRING_06_04_SLOW     = 0x00,
    TPS92662_SLEW_LED_STRING_06_04_MEDIUM   = 0x10,
    TPS92662_SLEW_LED_STRING_06_04_FAST     = 0x30,
            
    TPS92662_SLEW_LED_STRING_03_01_SLOW     = 0x00,
    TPS92662_SLEW_LED_STRING_03_01_MEDIUM   = 0x40,
    TPS92662_SLEW_LED_STRING_03_01_FAST     = 0xc0
} TPS92662_SLEWRATE;

typedef enum
{
    TPS92662_OVLMT_03_01_LOW                = 0x00,
    TPS92662_OVLMT_03_01_MEDIUM             = 0x01,
    TPS92662_OVLMT_03_01_HIGH               = 0x02,
            
    TPS92662_OVLMT_06_04_LOW                = 0x00,
    TPS92662_OVLMT_06_04_MEDIUM             = 0x04,
    TPS92662_OVLMT_06_04_HIGH               = 0x08,
            
    TPS92662_OVLMT_09_07_LOW                = 0x00,
    TPS92662_OVLMT_09_07_MEDIUM             = 0x10,
    TPS92662_OVLMT_09_07_HIGH               = 0x20,
            
    TPS92662_OVLMT_12_10_LOW                = 0x00,
    TPS92662_OVLMT_12_10_MEDIUM             = 0x40,
    TPS92662_OVLMT_12_10_HIGH               = 0x80
} TPS92662_OVERVOLTAGE_LIMIT;

typedef enum
{
    TPS92662_PARALLEL_NONE                  = 0x00,
    TPS92662_PARALLEL_S1_S2                 = 0x01,
    TPS92662_PARALLEL_S1_S2_AND_S3_S4       = 0x02,
    TPS92662_PARALLEL_S1_S2_S3_S4           = 0x03
} TPS92662_PARALLEL_LED_STRING;

typedef enum
{
    TPS92662_SYS_CONF_SEPTR_ENABLE          = 0x80,     // Separate TX/RX enabled (signals on TX do not appear on RX)
    TPS92662_SYS_CONF_SEPTR_DISABLE         = 0x00,     // Separate TX/RX disabled (signals on TX appear on RX)
            
    TPS92662_SYS_CONF_I2CEN_ENABLE          = 0x40,     // I2C function enabled
    TPS92662_SYS_CONF_I2CEN_DISABLE         = 0x00,     // I2C function disabled
            
    TPS92662_SYS_CONF_ACKEN_ENABLE          = 0x20,     // Acknowledge (0x7f) will be transmitted following successfully received writes
    TPS92662_SYS_CONF_ACKEN_DISABLE         = 0x00,     // No acknowledge will be transmitted following successfully received writes
                
    TPS92662_SYS_CONF_PSON_ON_TIMES         = 0x10,     // LED phase shifts apply to LED turn-on times
    TPS92662_SYS_CONF_PSON_OFF_TIMES        = 0x00,     // LED phase shifts apply to LED turn-off times
            
    TPS92662_SYS_CONF_CMWEN_ENABLE          = 0x08,     // Communications watchdog timer enabled
    TPS92662_SYS_CONF_CMWEN_DISABLE         = 0x00,     // Communications watchdog timer disabled
            
    TPS92662_SYS_CONF_SYNCOEN_ENABLE        = 0x04,     // SYNC output enabled
    TPS92662_SYS_CONF_SYNCOEN_DISABLE       = 0x00,     // SYNC output disabled
            
    TPS92662_SYS_CONF_SYNCPEN_CONTINUE      = 0x00,     // If SYNCOEN = 1, the SYNC output is driven continuously low (no pulses)
    TPS92662_SYS_CONF_SYNCPEN_50_PERCENT    = 0x02,     // If SYNCOEN = 1, the SYNC output is driven with a 50% duty cycle pulse to synchronize other connected TPS92662 ICs
            
    TPS92662_SYS_CONF_SET_PWR               = 0x01      // This bit is reset to 0 upon power-up. It may be written to 1 by the MCU. Reading this bit allows the MCU to detect when there has been z power cycle.
                                                        // 0 = A power cycle has occurred since last write to '1'
                                                        // 1 = No power cycle has occurred since last write to '1'
} TPS92662_SYSTEM_CONFIG;

typedef enum
{
    TPS92662_DIV1_1                         = 0x00,
    TPS92662_DIV1_50                        = 0x40,
    TPS92662_DIV1_125                       = 0x80,
    TPS92662_DIV1_200                       = 0xc0,
            
    TPS92662_DIV2_2                         = 0,
    TPS92662_DIV2_3                         = 1,
    TPS92662_DIV2_4                         = 2,
    TPS92662_DIV2_6                         = 3,
    TPS92662_DIV2_8                         = 4,
    TPS92662_DIV2_9                         = 5,
    TPS92662_DIV2_10                        = 6,
    TPS92662_DIV2_11                        = 7,
    TPS92662_DIV2_12                        = 8,
    TPS92662_DIV2_13                        = 9,
    TPS92662_DIV2_14                        = 10,
    TPS92662_DIV2_15                        = 11,
    TPS92662_DIV2_16                        = 12,
    TPS92662_DIV2_17                        = 13,
    TPS92662_DIV2_18                        = 14,
    TPS92662_DIV2_19                        = 15,
    TPS92662_DIV2_20                        = 16,
    TPS92662_DIV2_21                        = 17,
    TPS92662_DIV2_22                        = 18,
    TPS92662_DIV2_23                        = 19,
    TPS92662_DIV2_24                        = 20,
    TPS92662_DIV2_25                        = 21,
    TPS92662_DIV2_26                        = 22,
    TPS92662_DIV2_27                        = 23,
    TPS92662_DIV2_28                        = 24,
    TPS92662_DIV2_29                        = 25,
    TPS92662_DIV2_30                        = 26,
    TPS92662_DIV2_31                        = 27,
    TPS92662_DIV2_32                        = 28,
    TPS92662_DIV2_33                        = 29,
    TPS92662_DIV2_34                        = 30,
    TPS92662_DIV2_35                        = 31,            
    TPS92662_DIV2_36                        = 32,
    TPS92662_DIV2_37                        = 33,
    TPS92662_DIV2_38                        = 34,
    TPS92662_DIV2_39                        = 35,
    TPS92662_DIV2_40                        = 36,
    TPS92662_DIV2_41                        = 37,
    TPS92662_DIV2_42                        = 38,
    TPS92662_DIV2_43                        = 39,
    TPS92662_DIV2_44                        = 40,
    TPS92662_DIV2_45                        = 41,
    TPS92662_DIV2_46                        = 42,
    TPS92662_DIV2_47                        = 43,
    TPS92662_DIV2_49                        = 44,
    TPS92662_DIV2_50                        = 45,
    TPS92662_DIV2_51                        = 46,
    TPS92662_DIV2_52                        = 47,
    TPS92662_DIV2_53                        = 48,
    TPS92662_DIV2_54                        = 49,
    TPS92662_DIV2_55                        = 50,
    TPS92662_DIV2_56                        = 51,
    TPS92662_DIV2_57                        = 52,
    TPS92662_DIV2_58                        = 53,
    TPS92662_DIV2_59                        = 54,
    TPS92662_DIV2_60                        = 55,
    TPS92662_DIV2_62                        = 56,
    TPS92662_DIV2_63                        = 57,
    TPS92662_DIV2_65                        = 58,
    TPS92662_DIV2_68                        = 59,
    TPS92662_DIV2_71                        = 60,
    TPS92662_DIV2_74                        = 61,
    TPS92662_DIV2_78                        = 62,
    TPS92662_DIV2_85                        = 63
} TPS92662_PWM_DIVISION;

typedef struct
{
    uint16_t                phase[12];
    uint16_t                width[12];
    
    uint8_t                 slew_rate;                  // R/W [0x70 / 11 11 11 11]
                                                        // The LED bypass gate drivers have variable slew rates, one setting per LED string. 
                                                        // SLEWRATE register = [SLEW_LED_STRING_12_10] [SLEW_LED_STRING_09_07] [SLEW_LED_STRING_06_04] [SLEW_LED_STRING_03_01] 
                                                        // There are two bits for each LED string:
                                                        // 00: Slow rate
                                                        // 01: Medium rate
                                                        // 10: Fast rate
                                                        // 11 (default): Fast rate
    uint8_t                 over_voltage_limit;         // R/W [0x71 / 10101010]
                                                        // The LED bypass switches have configurable overvoltage limits, one setting per LED string.
                                                        // The OVLMT register is mapped as follow: OVLMT_12_10[7:6] OVLMT_09_07[5:4] OVLMT_06_04[3:2] OVLMT_03_01[1:0]
                                                        // There are two bits for each LED string. The two-bit overvoltage limits are mapped to three levels of low, medium and high threshold voltage.
                                                        // 00: V_TH_O1 (low threshold voltage 5V min / 6V typ / 6,9V max)
                                                        // 01: V_TH_O2 (medium threshold voltage 10,2V min / 12V typ / 13,8V max)
                                                        // 10 (default): V_TH_O3 (high threshold voltage 16,5V min / 18V typ / 20V max)
                                                        // 11: V_TH_O3 (high threshold voltage 16,5V min / 18V typ / 20V max)
    uint8_t                 parallel_led_string;        // R/W [0x72 / xxxxxx 00]
                                                        // The combinations of LED strings that may be paralleled are as follows (S1 = LED1-3 / S2 = LED4-6 / S3 = LED7-9 / S4 = LED10-12):
                                                        // 00 (default): None (each string of bypass switches is independent from the others)
                                                        // 01: One pair (strings of S1 and S2 are connected in parallel, S3 and S4 are independent)
                                                        // 10: Two pairs (strings of S1 and S2 are connected in parallel, S3 and S4 are connected in parallel)
                                                        // 11: All (strings of S1, S2, S3 and S4 are all connected in parallel)
    uint8_t                 default_pulse_width[12];    // R/W [0x73..0x78 / (0x73)0000 0000 .. (0x78)0000 0000]
                                                        // The default pulse width registers is used for PWM dimming of each LED in the case where there has been a communications watchdog timeout condition.
                                                        // (@ 0x73) DEFWIDTH_LED2[7..4] DEFWIDTH_LED1[3..0]
                                                        // (@ 0x74) DEFWIDTH_LED4[7..4] DEFWIDTH_LED3[3..0]
                                                        // (@ 0x75) DEFWIDTH_LED6[7..4] DEFWIDTH_LED5[3..0]
                                                        // (@ 0x76) DEFWIDTH_LED8[7..4] DEFWIDTH_LED7[3..0]
                                                        // (@ 0x77) DEFWIDTH_LED10[7..4] DEFWIDTH_LED9[3..0]
                                                        // (@ 0x78) DEFWIDTH_LED12[7..4] DEFWIDTH_LED11[3..0]
                                                        // This 4-bit value determines the pulse width for LEDn as follow:
                                                        // 0..15 LEDn pulse width -> 0 (LED fully off) / 64 / 128 / 192 / 256 / 320 / 384 / 448 / 512 / 576 / 640 / 704 / 768 / 832 / 896 / 1023 (LED fully on)
    uint8_t                 system_config;              // R/W [0x80 / 00000000]
                                                        // The control register SYSCFG allow control of several functions. See. enumeration TPS92662_SYSTEM_CONFIG for more details.
    uint8_t                 watchdog_timer;             // R/W [0x81 / xxxxx 101]
                                                        // These 3-bit selects the tap point on the 24-bit communications watchdog timer to establish the timeout condition.
                                                        // 0: 16 Tap Point (bit number)
                                                        // 1: 17 Tap Point (bit number)
                                                        // 2: 18 Tap Point (bit number)
                                                        // 3: 19 Tap Point (bit number)
                                                        // 4: 20 Tap Point (bit number)
                                                        // 5 (default): 21 Tap Point (bit number)
                                                        // 6: 22 Tap Point (bit number)
                                                        // 7: 23 Tap Point (bit number)
    uint8_t                 pwm_tick_period;            // R/W [0x82 / 00 001100]
                                                        // This register determines division block applied to input CLK, generating the PWM clock cycle. PWM_CLK = CLK quartz / (DIV1 (PTBASE) x DIV2 (PTCNT) x 1024)
                                                        // There are two fields: PTBASE[7:6] PTCNT[5:0]
                                                        // These two fields program the two divider blocks in series, which are applied to the input CLK and generate the PWM clock. 
                                                        // PTBASE: 0 = div 1 (default) // 1 = div 50 // 2 = div 125 // 3 = div 200
                                                        // PTCNT: [0..15] div values    -> 2 / 3 / 4 / 6 / 8 / 9 / 10 / 11 / 12 / 13 / 14 / 15 / 16 (default) / 17 / 18 / 19
                                                        // PTCNT: [16..31] div values   -> 20 / 21 / 22 / 23 / 24 / 25 / 26 / 27 / 28 / 29 / 30 / 31 / 32 / 33 / 34 / 35
                                                        // PTCNT: [32..47] div values   -> 36 / 37 / 38 / 39 / 40 / 41 / 42 / 43 / 44 / 45 / 46 / 47 / 49 / 50 / 51 / 52
                                                        // PTCNT: [48..63] div values   -> 53 / 54 / 55 / 56 / 57 / 58 / 59 / 60 / 62 / 63 / 65 / 68 / 71 / 74 / 78 / 85
    uint8_t                 adc_id;                     // R/W [0x83 / xxxxxx 00]
                                                        // This 2-bits value determines whether to use the ADDR0 analog input as part of the device ID.
                                                        // This register is intended to be written with a SINGLE byte BROADCAST WRITE.
                                                        // 0 or 3: ADDR0 input is used as a single bit value (GND or AREF) -> Up to 8 TPS devices on one bus (device_id = 0 0 ADDR2 ADDR1 ADDR0).
                                                        // 1: ADDR0 input is sampled as a two bits value (GND / 0,25*AREF / 0,5*AREF / AREF) -> Up to 16 TPS devices on one bus (device_id = 0 ADDR2 ADDR1 ADC_ADDR0_b1 ADC_ADDR0_b0).
                                                        // 2: ADDR0 input is sampled as a three bits value (GND / 0,125*AREF / 0,25*AREF / 0,375*AREF / 0,5*AREF / 0,625*AREF / 0,75*AREF / AREF) -> Up to 32 TPS devices on one bus (device_id = ADDR2 ADDR1 ADC_ADDR0_b2 ADC_ADDR0_b1 ADR_ADDR0_b0).
    uint8_t                 softsync;                   // Write Only [0x84 / xxxxxxx 0]
                                                        // Set this bit to '1' to re-initialize the internal PWM counter (TCNT) to zero. This bit always read back as a 0.
    uint8_t                 adc[2];                     // Read Only [0xa0..0xa1 / 00000000]
                                                        // The ADC registers hold the most recent conversion results (8 bits) of ADC conversions. 
    uint16_t                fault_error;                // R/W [0xb0..0xb1 / (0xb1)xxxx 0000 (0xb0)00000000]
    uint8_t                 crc_error;                  // R/W [0xb2 / 00000000]
                                                        // This register value is incremented each time a CRC error is received. 
                                                        // This register is not automatically cleared when a communications reset is received and it does not wrap back to 0 when it reaches 0xff. It must be cleared manually by writing it back to 0.
    uint8_t                 ic_identification;          // Read Only [0xff / 10010010]
} TPS92662_REGS;

// NOTE: On ONE UART line we can have only ONE EXTERNAL DEVICE type. So no need to use a BUS MANAGEMENT. 
typedef struct
{
    bool                    is_init_done;
    UART_MODULE             uart_id;
    DMA_MODULE              dma_tx_id;
    DMA_MODULE              dma_rx_id;
    _IO                     chip_enable;
    uint32_t                uart_baudrate;    
    DMA_CHANNEL_TRANSFER    dma_tx_params;
    DMA_CHANNEL_TRANSFER    dma_rx_params;
    
    uint8_t                 number_of_device;
    uint8_t                 *p_device_id;    
    TPS92662_REGS           *p_registers;   
    uint8_t                 *p_transfer;
    uint8_t                 *p_receip;
    
    uint32_t                *p_flags;
    uint8_t                 selected_device_index;
    state_machine_t         state_machine_for_read_write_request;
    state_machine_t         state_machine;
} TPS92662_PARAMS;

#define TPS92662_INSTANCE(_uart_id, _dma_tx_id, _dma_rx_id, _io_port, _io_indice, _uart_baudrate, _number_of_device, _p_transfer, _p_receip, _p_device_id, _p_registers, _p_flags) \
{                                                                           \
    .is_init_done = 0,                                                      \
    .uart_id = _uart_id,                                                    \
    .dma_tx_id = _dma_tx_id,                                                \
    .dma_rx_id = _dma_rx_id,                                                \
    .chip_enable = { _io_port, _io_indice },                                \
    .uart_baudrate = _uart_baudrate,                                        \
    .dma_tx_params = {_p_transfer, NULL, 0, 1, 1, 0},                       \
    .dma_rx_params = {NULL, _p_receip, 1, 0, 1, 0},                         \
    .number_of_device = _number_of_device,                                  \
    .p_device_id = _p_device_id,                                            \
    .p_registers = _p_registers,                                            \
    .p_transfer = _p_transfer,                                              \
    .p_receip = _p_receip,                                                  \
    .p_flags = _p_flags,                                                    \
    .selected_device_index = 0,                                             \
    .state_machine_for_read_write_request = {0},                            \
    .state_machine = {0}                                                    \
}

#define TPS92662_DEF(_name, _uart_id, _dma_tx_id, _dma_rx_id, _chip_enable_pin, _uart_baudrate, ...)        \
static uint8_t _name ## _device_id_ram_allocation[COUNT_ARGUMENTS( __VA_ARGS__ )] = { __VA_ARGS__ };        \
static TPS92662_REGS _name ## _registers_ram_allocation[COUNT_ARGUMENTS( __VA_ARGS__ )] = {0};              \
static uint8_t _name ## _transfer_ram_allocation[TPS92662_MAX_TX_SIZE] = {0};                               \
static uint8_t _name ## _receip_ram_allocation[TPS92662_MAX_RX_SIZE] = {0};                                 \
static uint32_t _name ## _flags[COUNT_ARGUMENTS( __VA_ARGS__ )] = {0};                                      \
static TPS92662_PARAMS _name = TPS92662_INSTANCE(_uart_id, _dma_tx_id, _dma_rx_id, __PORT(_chip_enable_pin), __INDICE(_chip_enable_pin), _uart_baudrate, COUNT_ARGUMENTS( __VA_ARGS__ ), _name ## _transfer_ram_allocation, _name ## _receip_ram_allocation, _name ## _device_id_ram_allocation, _name ## _registers_ram_allocation, _name ## _flags)

typedef void (*p_tps92662_function)(TPS92662_PARAMS *var, uint8_t device_index, uint8_t *buffer);

uint8_t e_tps92662_deamon(TPS92662_PARAMS *var);

// Defines VARIABLE version
#define e_tps92662_get_ic_identifier(var, _id_device)                                   (SET_BIT(var.p_flags[_id_device], SM_TPS92662_READ_IC_IDENTIFIER))
#define e_tps92662_get_errors(var, _id_device)                                          (SET_BIT(var.p_flags[_id_device], SM_TPS92662_READ_ERRORS))
#define e_tps92662_get_adc(var, _id_device)                                             (SET_BIT(var.p_flags[_id_device], SM_TPS92662_READ_ADCS))

#define e_tps92662_set_system_config(var, _id_device, _value)                           (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_SYSTEM_CONFIG), var.p_registers[_id_device].system_config = _value)
#define e_tps92662_set_slew_rate(var, _id_device, _value)                               (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_SLEW_RATE), var.p_registers[_id_device].slew_rate = _value)
#define e_tps92662_set_overvoltage_limit(var, _id_device, _value)                       (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_OVER_VOLTAGE_LIMIT), var.p_registers[_id_device].over_voltage_limit = _value)
#define e_tps92662_set_parallel_led_string(var, _id_device, _value)                     (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_PARALLEL_LED_STRING), var.p_registers[_id_device].parallel_led_string = _value)
#define e_tps92662_set_default_pulse_width(var, _id_device, _id_led, _value)            (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_DEFAULT_PULSE_WIDTH), var.p_registers[_id_device].default_pulse_width[_id_led] = _value)
#define e_tps92662_set_watchdog_timer(var, _id_device, _value)                          (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_WATCHDOG_TIMER), var.p_registers[_id_device].watchdog_timer = _value)
#define e_tps92662_set_pwm_tick_period(var, _id_device, _value)                         (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_PWM_TICK_PERIOD), var.p_registers[_id_device].pwm_tick_period = _value)
#define e_tps92662_set_adc_id(var, _id_device, _value)                                  (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_ADC_ID), var.p_registers[_id_device].adc_id = _value)
#define e_tps92662_set_softsync(var, _id_device, _value)                                (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_SOFTSYNC), var.p_registers[_id_device].softsync = _value)

#define e_tps92662_set_phase_and_width(var, _id_device, _id_led, _phase, _width)        (var.p_registers[_id_device].phase[_id_led] = _phase, var.p_registers[_id_device].width[_id_led] = _width)
#define e_tps92662_set_phase(var, _id_device, _id_led, _phase)                          (var.p_registers[_id_device].phase[_id_led] = _phase)
#define e_tps92662_set_width(var, _id_device, _id_led, _width)                          (var.p_registers[_id_device].width[_id_led] = _width)

#define e_tps92662_send_phases_and_widths(var, _id_device)                              (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_PHASE_AND_WIDTH))
#define e_tps92662_send_phases(var, _id_device)                                         (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_PHASE))
#define e_tps92662_send_widths(var, _id_device)                                         (SET_BIT(var.p_flags[_id_device], SM_TPS92662_WRITE_WIDTH))

// Defines POINTER version
#define e_tps92662_set_system_config_ptr(var, _id_device, _value)                       (SET_BIT(var->p_flags[_id_device], SM_TPS92662_WRITE_SYSTEM_CONFIG), var->p_registers[_id_device].system_config = _value)
#define e_tps92662_get_ic_identifier_ptr(var, _id_device)                               (SET_BIT(var->p_flags[_id_device], SM_TPS92662_READ_IC_IDENTIFIER))
#define e_tps92662_set_pwm_tick_period_ptr(var, _id_device, _value)                     (SET_BIT(var->p_flags[_id_device], SM_TPS92662_WRITE_PWM_TICK_PERIOD), var->p_registers[_id_device].pwm_tick_period = _value)
#define e_tps92662_send_phases_and_widths_ptr(var, _id_device)                          (SET_BIT(var->p_flags[_id_device], SM_TPS92662_WRITE_PHASE_AND_WIDTH))

#endif