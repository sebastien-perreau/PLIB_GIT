#ifndef __DEF_EXAMPLES
#define __DEF_EXAMPLES

typedef enum
{
    _SETUP = 0,
    _MAIN
} _EXAMPLE_STATUS;

void _EXAMPLE_TICK_FUNCTIONS();
void _EXAMPLE_TIMER();
void _EXAMPLE_DMA_RAM_TO_RAM();
void _EXAMPLE_DMA_UART();
void _EXAMPLE_DMA_SPI();
void _EXAMPLE_PWM();
void _EXAMPLE_SOFTWARE_PWM();
void _EXAMPLE_SWITCH();
void _EXAMPLE_ENCODER();
void _EXAMPLE_AVERAGE_AND_NTC();
void _EXAMPLE_25LC512();
void _EXAMPLE_MCP23S17();
void _EXAMPLE_LOG(acquisitions_params_t var);
void _EXAMPLE_UART();
void _EXAMPLE_PCA9685();
void _EXAMPLE_AT42QT2120();
void _EXAMPLE_VEML7700();
void _EXAMPLE_BLE(ble_params_t * p_ble);
void _EXAMPLE_LIN();
void _EXAMPLE_PINK_LADY();
void _EXAMPLE_TPS92662();
void _EXAMPLE_SD_CARD();

#endif
