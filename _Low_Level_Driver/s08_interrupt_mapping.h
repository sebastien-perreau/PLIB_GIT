#ifndef __DEF_INTERRUPT_MAPPING
#define	__DEF_INTERRUPT_MAPPING

typedef enum
{
    IRQ_CT = 0,             // Core Timer Interrupt

    IRQ_CS0,                // Core Software Interrupt 0
    IRQ_CS1,                // Core Software Interrupt 1

    IRQ_INT0,               // External Interrupt 0
    IRQ_INT1,               // External Interrupt 1
    IRQ_INT2,               // External Interrupt 2
    IRQ_INT3,               // External Interrupt 3
    IRQ_INT4,               // External Interrupt 4

    IRQ_T1,                 // Timer1
    IRQ_T2,                 // Timer2
    IRQ_T3,                 // TImer3
    IRQ_T4,                 // Timer4
    IRQ_T5,                 // Timer5

    IRQ_IC1,                // Input Capture 1
    IRQ_IC2,                // Input Capture 2
    IRQ_IC3,                // Input Capture 3
    IRQ_IC4,                // Input Capture 4
    IRQ_IC5,                // Input Capture 5

    IRQ_IC1E,               // Input Capture Error 1
    IRQ_IC2E,               // Input Capture Error 2
    IRQ_IC3E,               // Input Capture Error 3
    IRQ_IC4E,               // Input Capture Error 4
    IRQ_IC5E,               // Input Capture Error 5

    IRQ_OC1,                // Output Compare 1
    IRQ_OC2,                // Output Compare 2
    IRQ_OC3,                // Output Compare 3
    IRQ_OC4,                // Output Compare 4
    IRQ_OC5,                // Output Compare 5

    IRQ_CN,                 // Change Notice

    IRQ_SPI1,               // SPI1 (use to clear all flags)
    IRQ_SPI2,               // SPI2 (use to clear all flags)
    IRQ_SPI3,               // SPI3 (use to clear all flags)
    IRQ_SPI4,               // SPI4 (use to clear all flags)

    IRQ_SPI1E,              // SPI1 Fault
    IRQ_SPI2E,              // SPI2 Fault
    IRQ_SPI3E,              // SPI3 Fault
    IRQ_SPI4E,              // SPI4 Fault

    IRQ_SPI1TX,             // SPI1 Transfer Done
    IRQ_SPI2TX,             // SPI2 Transfer Done
    IRQ_SPI3TX,             // SPI3 Transfer Done
    IRQ_SPI4TX,             // SPI4 Transfer Done

    IRQ_SPI1RX,             // SPI1 Receive Done
    IRQ_SPI2RX,             // SPI2 Receive Done
    IRQ_SPI3RX,             // SPI3 Receive Done
    IRQ_SPI4RX,             // SPI4 Receive Done

    IRQ_I2C1,               // I2C1 (use to clear all flags)
    IRQ_I2C2,               // I2C2 (use to clear all flags)
    IRQ_I2C3,               // I2C3 (use to clear all flags)
    IRQ_I2C4,               // I2C4 (use to clear all flags)
    IRQ_I2C5,               // I2C5 (use to clear all flags)

    IRQ_I2C1B,              // I2C1 Bus Colision Event
    IRQ_I2C2B,              // I2C2 Bus Colision Event
    IRQ_I2C3B,              // I2C3 Bus Colision Event
    IRQ_I2C4B,              // I2C4 Bus Colision Event
    IRQ_I2C5B,              // I2C5 Bus Colision Event

    IRQ_I2C1S,              // I2C1 Slave Event
    IRQ_I2C2S,              // I2C2 Slave Event
    IRQ_I2C3S,              // I2C3 Slave Event
    IRQ_I2C4S,              // I2C4 Slave Event
    IRQ_I2C5S,              // I2C5 Slave Event

    IRQ_I2C1M,              // I2C1 Master Event
    IRQ_I2C2M,              // I2C2 Master Event
    IRQ_I2C3M,              // I2C3 Master Event
    IRQ_I2C4M,              // I2C4 Master Event
    IRQ_I2C5M,              // I2C5 Master Event

    IRQ_U1,                 // UART1 (use to clear all flags)
    IRQ_U2,                 // UART2 (use to clear all flags)
    IRQ_U3,                 // UART3 (use to clear all flags)
    IRQ_U4,                 // UART4 (use to clear all flags)
    IRQ_U5,                 // UART5 (use to clear all flags)
    IRQ_U6,                 // UART6 (use to clear all flags)

    IRQ_U1E,                // UART1 Error Event
    IRQ_U2E,                // UART2 Error Event
    IRQ_U3E,                // UART3 Error Event
    IRQ_U4E,                // UART4 Error Event
    IRQ_U5E,                // UART5 Error Event
    IRQ_U6E,                // UART6 Error Event

    IRQ_U1RX,               // UART1 RX Event
    IRQ_U2RX,               // UART2 RX Event
    IRQ_U3RX,               // UART3 RX Event
    IRQ_U4RX,               // UART4 RX Event
    IRQ_U5RX,               // UART5 RX Event
    IRQ_U6RX,               // UART6 RX Event

    IRQ_U1TX,               // UART1 TX Event
    IRQ_U2TX,               // UART2 TX Event
    IRQ_U3TX,               // UART3 TX Event
    IRQ_U4TX,               // UART4 TX Event
    IRQ_U5TX,               // UART5 TX Event
    IRQ_U6TX,               // UART6 TX Event

    IRQ_AD1,                // ADC1 Convert Done

    IRQ_PMP,                // Parallel Master Port
    IRQ_PMPE,               // Parallel Master Port Error

    IRQ_CMP1,               // Comparator 1 Interrupt
    IRQ_CMP2,               // Comparator 2 Interrupt

    IRQ_FSCM,               // Fail-safe Monitor

    IRQ_FCE,                // Flash Control Event

    IRQ_RTCC,               // Real Time Clock

    IRQ_DMA0,               // DMA Channel 0
    IRQ_DMA1,               // DMA Channel 1
    IRQ_DMA2,               // DMA Channel 2
    IRQ_DMA3,               // DMA Channel 3
    IRQ_DMA4,               // DMA Channel 4
    IRQ_DMA5,               // DMA Channel 5
    IRQ_DMA6,               // DMA Channel 6
    IRQ_DMA7,               // DMA Channel 7

    IRQ_USB,                // USB

    IRQ_CAN1,               // CAN1
    IRQ_CAN2,               // CAN2

    IRQ_ETHERNET,           // Ethernet

    IRQ_NUM                 // Number of available interrupt sources
} IRQ_SOURCE;

typedef enum
{
    IRQ_DISABLED            = 0,
    IRQ_ENABLED             = 1
} IRQ_EN_DIS;

typedef enum
{
    REG                     = 0,
    REG_CLR,
    REG_SET,
    REG_INV
} _ENUM_REGISTER;

typedef struct
{
    uint8_t                 priority;
    uint8_t                 sub_priority;
} IRQ_DATA_PRIORITY;

typedef struct
{
    volatile uint32_t       *IFS;
    volatile uint32_t       *IEC;
    volatile uint32_t       *IPC;
    volatile uint32_t       MASK;
    volatile uint32_t       SUB_PRI_POS;
    volatile uint32_t       PRI_POS;
} IRQ_REGISTERS;

typedef enum
{
    IRQ_NONE                = 0x00,
    IRQ_ALL                 = 0x07,
    IRQ_SPI_FAULT           = 0x01,
    IRQ_SPI_TX              = 0x02,
    IRQ_SPI_RX              = 0x04,
    IRQ_I2C_MASTER          = 0x01,
    IRQ_I2C_BUS_COLISION    = 0x02,
    IRQ_I2C_SLAVE           = 0x04,
    IRQ_UART_ERROR          = 0x01,
    IRQ_UART_RX             = 0x02,
    IRQ_UART_TX             = 0x04
} IRQ_EVENT_TYPE;
            
void irq_link_data_priority(const IRQ_DATA_PRIORITY *p_data_priority);
IRQ_DATA_PRIORITY irq_change_notice_priority();
IRQ_DATA_PRIORITY irq_adc10_priority();
IRQ_DATA_PRIORITY irq_timer_priority(uint8_t id);
IRQ_DATA_PRIORITY irq_dma_priority(uint8_t id);
IRQ_DATA_PRIORITY irq_uart_priority(uint8_t id);
IRQ_DATA_PRIORITY irq_spi_priority(uint8_t id);
IRQ_DATA_PRIORITY irq_i2c_priority(uint8_t id);
IRQ_DATA_PRIORITY irq_can_priority(uint8_t id);

void irq_init(IRQ_SOURCE source, bool enable, IRQ_DATA_PRIORITY priority);
void irq_clr_flag(IRQ_SOURCE source);
void irq_set_flag(IRQ_SOURCE source);
uint32_t irq_get_flag(IRQ_SOURCE source);
void irq_enable(IRQ_SOURCE source, bool enable);

#endif
