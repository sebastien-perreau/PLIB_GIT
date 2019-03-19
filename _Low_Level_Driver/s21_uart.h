#ifndef __DEF_UART
#define	__DEF_UART

typedef enum
{
    UART1 = 0,
    UART2, 
    UART3, 
    UART4, 
    UART5, 
    UART6,
    UART_NUMBER_OF_MODULES
} UART_MODULE;

typedef enum
{
    UART_BAUDRATE_600           = 600,
    UART_BAUDRATE_9600          = 9600,
    UART_BAUDRATE_19200         = 19200,
    UART_BAUDRATE_115200        = 115200,
    UART_BAUDRATE_230400        = 230400,
    UART_BAUDRATE_1M            = 1000000,
    UART_BAUDRATE_2M            = 2000000
} UART_BAUDRATE;

typedef enum
{
    UART_ENABLE                 = 0x00000001,
    UART_DISABLE                = 0x00000000,
    UART_ENABLE_RX_PIN          = 0x00001000,
    UART_ENABLE_TX_PIN          = 0x00000400
} UART_ENABLE_MODE;

typedef enum
{
    UART_ENABLE_HIGH_SPEED      = 0x00000008,   // MODE register
    UART_ENABLE_LOOPBACK        = 0x00000040,   // MODE register
    UART_ENABLE_WAKE_UP         = 0x00000080,   // MODE register
    UART_RTS_SIMPLEX_MODE       = 0x00000800,   // MODE register
    UART_ENABLE_IRDA            = 0x00001000,   // MODE register
    UART_ENABLE_STOP_ON_IDLE    = 0x00002000,   // MODE register
            
    UART_RX_POLARITY_LOW        = 0x00000010,   // MODE register
    UART_RX_POLARITY_HIGH       = 0x00000000,   // MODE register
    UART_TX_POLARITY_LOW        = 0x20000000,   // STA register
    UART_TX_POLARITY_HIGH       = 0x00000000,   // STA register
           
    UART_ENABLE_PINS_BIT_CLOCK  = 0x00000300,   // MODE register
    UART_ENABLE_PINS_CTS_RTS    = 0x00000200,   // MODE register
    UART_ENABLE_PINS_RTS        = 0x00000100,   // MODE register
    UART_ENABLE_PINS_TX_RX_ONLY = 0x00000000,   // MODE register
            
    UART_CONFIG_MASK            = 0x20003bd8
} UART_CONFIG_MODE;

typedef enum
{
    UART_DATA_SIZE_8_BITS       = 0x00000000,
    UART_DATA_SIZE_9_BITS       = 0x00000006,
    UART_PARITY_ODD             = 0x00000004,
    UART_PARITY_EVEN            = 0x00000002,
    UART_PARITY_NONE            = 0x00000000,
    UART_STOP_BITS_2            = 0x00000001,
    UART_STOP_BITS_1            = 0x00000000,
    UART_LINE_CONTROL_MASK      = 0x00000007
} UART_LINE_CONTROL_MODE;

typedef enum
{
    UART_INTERRUPT_ON_TX_BUFFER_EMPTY   = 0x00008000,
    UART_INTERRUPT_ON_TX_DONE           = 0x00004000,
    UART_INTERRUPT_ON_TX_NOT_FULL       = 0x00000000,
    UART_INTERRUPT_ON_RX_FULL           = 0x000000C0,
    UART_INTERRUPT_ON_RX_3_QUARTER_FULL = 0x00000080,
    UART_INTERRUPT_ON_RX_HALF_FULL      = 0x00000040,
    UART_INTERRUPT_ON_RX_NOT_EMPTY      = 0x00000000,
    UART_FIFO_MASK                      = 0x0000c0c0
} UART_FIFO_MODE;

typedef enum
{
    UART_ENABLE_ADDRESS_DETECTION       = 0x00100000,
    UART_DISABLE_ADDRESS_DETECTION      = 0x00000000,
    UART_ADDRESS_DETECTION_MASK         = 0x001f0000
} UART_ADDRESS_DETECTION;

#define UART_STD_PARAMS         UART_ENABLE | UART_ENABLE_RX_PIN | UART_ENABLE_TX_PIN,                                                      \
                                UART_ENABLE_HIGH_SPEED | UART_RX_POLARITY_HIGH | UART_TX_POLARITY_HIGH | UART_ENABLE_PINS_TX_RX_ONLY,       \
                                UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1,                                                \
                                UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY,                                                 \
                                0x00 | UART_DISABLE_ADDRESS_DETECTION

typedef struct
{
	volatile uint32_t	MODE;
	volatile uint32_t	MODECLR;
	volatile uint32_t	MODESET;
	volatile uint32_t	MODEINV;
    
	volatile uint32_t	STA;
	volatile uint32_t	STACLR;
	volatile uint32_t	STASET;
	volatile uint32_t	STAINV;
    
	volatile uint32_t	TX;
	volatile uint32_t	TXCLR;
	volatile uint32_t	TXSET;
	volatile uint32_t	TXINV;
    
    volatile uint32_t	RX;
	volatile uint32_t	RXCLR;
	volatile uint32_t	RXSET;
	volatile uint32_t	RXINV;
    
    volatile uint32_t	BRG;
	volatile uint32_t	BRGCLR;
	volatile uint32_t	BRGSET;
	volatile uint32_t	BRGINV;
} UART_REGISTERS;

typedef void (*uart_event_handler_t)(uint8_t id, IRQ_EVENT_TYPE event_type, uint32_t event_value);

void uart_init(     UART_MODULE id, 
                    uart_event_handler_t evt_handler,
                    IRQ_EVENT_TYPE event_type_enable,
                    UART_BAUDRATE baudrate,
                    UART_ENABLE_MODE enable,
                    UART_CONFIG_MODE config_mode, 
                    UART_LINE_CONTROL_MODE control_mode,
                    UART_FIFO_MODE fifo_mode,
                    UART_ADDRESS_DETECTION address_detection);
void uart_enable(UART_MODULE id, UART_ENABLE_MODE enable_mode);
void uart_set_params(UART_MODULE id, UART_CONFIG_MODE config_mode);
void uart_set_line_control(UART_MODULE id, UART_LINE_CONTROL_MODE control_mode);
void uart_set_fifo(UART_MODULE id, UART_FIFO_MODE fifo_mode);
void uart_set_adress_detection(UART_MODULE id, uint8_t address, UART_ADDRESS_DETECTION address_detection);
void uart_set_baudrate(UART_MODULE id, uint32_t baudrate);
uint32_t uart_get_baudrate(UART_MODULE id);
bool uart_transmission_has_completed(UART_MODULE id);
bool uart_is_tx_ready(UART_MODULE id);
bool uart_is_rx_data_available(UART_MODULE id);
bool uart_send_break(UART_MODULE id);
bool uart_send_data(UART_MODULE id, uint16_t data);
bool uart_get_data(UART_MODULE id, uint16_t *p_data);

const uint8_t uart_get_tx_irq(UART_MODULE id);
const uint8_t uart_get_rx_irq(UART_MODULE id);
const void *uart_get_tx_reg(UART_MODULE id);
const void *uart_get_rx_reg(UART_MODULE id);

void uart_interrupt_handler(UART_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data);

#endif