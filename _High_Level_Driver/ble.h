#ifndef __BLE_H
#define __BLE_H

#define _VSD_VERSIONS               "x.xx.xx"

#define ID_BOOT_MODE                0x00
#define ID_PA_LNA					0x01
#define ID_LED_STATUS				0x02
#define ID_SET_NAME					0x03
#define ID_GET_VERSION				0x04
#define ID_ADV_INTERVAL				0x05
#define ID_ADV_TIMEOUT				0x06
#define ID_GET_CONN_STATUS          0x07
#define ID_GET_BLE_PARAMS			0x08
#define ID_SOFTWARE_RESET			0xff

#define ID_CHAR_BUFFER              0x30

#define ID_SET_BLE_CONN_PARAMS      0x20
#define ID_SET_BLE_PHY_PARAMS       0x21
#define ID_SET_BLE_ATT_SIZE_PARAMS  0x22


#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))
#define UNIT_0_625_MS               625
#define UNIT_1_25_MS                1250
#define UNIT_10_MS                  10000

#define BLE_GAP_PHY_1MBPS           0x01
#define BLE_GAP_PHY_2MBPS           0x02

#define RESET_BLE_PICKIT            0x01
#define RESET_ALL                   0x02


typedef enum
{
    UART_NO_MESSAGE = 0,
    UART_ACK_MESSAGE,
    UART_NACK_MESSAGE,
    UART_NEW_MESSAGE,
    UART_OTHER_MESSAGE
} BLE_UART_MESSAGE_TYPE;

typedef struct
{
    BLE_UART_MESSAGE_TYPE           message_type;
	bool                            receive_in_progress;
	uint8_t                         buffer[256];
	uint8_t                         index;
	uint64_t                        tick;
} ble_uart_t;

typedef struct
{
	uint8_t                         id;
	uint8_t                         type;
	uint8_t                         length;
	uint8_t                         data[251];
} ble_serial_message_t;

typedef union
{
    struct
    {
        unsigned                    pa_lna:1;
        unsigned                    led_status:1;
        unsigned                    set_name:1;
        unsigned                    get_version:1;
        unsigned                    adv_interval:1;
        unsigned                    adv_timeout:1;
        unsigned                    send_reset_ble_pickit:1;
        unsigned                    send_reset_all:1;
        unsigned                    exec_reset:1;
        
        unsigned                    send_buffer:1;
        
        unsigned                    set_conn_params:1;
        unsigned                    set_phy_params:1;
        unsigned                    set_att_size_params:1;
    };
    struct
    {
        uint32_t                    w;
    };
} ble_flags_t;

typedef struct
{
    uint16_t                        min_conn_interval;
    uint16_t                        max_conn_interval;
    uint16_t                        slave_latency;
    uint16_t                        conn_sup_timeout;
} ble_gap_conn_params_t;

typedef struct
{
    uint16_t                        max_tx_octets;
    uint16_t                        max_rx_octets;
} ble_gap_data_length_params_t;

typedef struct
{
	ble_gap_conn_params_t 			conn_params;
	uint8_t                         phys_params;
	ble_gap_data_length_params_t	mtu_size_params;
	uint32_t						adv_interval;
	uint32_t						adv_timeout;
} ble_pickit_gap_params;

typedef struct
{
    ble_pickit_gap_params           preferred_gap_params;
    bool                            pa_lna_enable;
    bool                            led_status_enable;
} ble_pickit_params;

typedef struct
{
    char                            vsd_version[8];
    char                            device_name[20];
} ble_device_infos_t;

typedef struct
{
    bool                            is_pa_lna_enabled;
    bool                            is_led_status_enabled;
} ble_hardware_status_t;

typedef struct
{
    bool                            is_conn_status_updated;
    bool                            is_connected_to_a_central;
    bool                            is_in_advertising_mode;
} ble_conn_status_t;

typedef struct
{
    bool                            is_gap_params_updated;
    ble_pickit_gap_params			current_gap_params;  
} ble_gap_status_t;

typedef struct
{
    ble_device_infos_t              infos;
    ble_flags_t                     flags;
    ble_hardware_status_t           hardware;
    ble_conn_status_t               connection;
    ble_gap_status_t                gap;    
} ble_status_t;

typedef struct
{
    uint8_t                         in_data[247];
    uint8_t                         in_length;
    bool                            in_is_updated;
    uint8_t                         out_data[247];
    uint8_t                         out_length;
} ble_char_buffer_t;

typedef struct
{
    ble_char_buffer_t               buffer;
} ble_characteristics_t;

typedef struct
{
	ble_uart_t                      uart;
	ble_serial_message_t            incoming_message_uart; 
    ble_status_t                    status;
    ble_pickit_params               params;
    ble_characteristics_t           service;
} ble_params_t;

#define BLE_DEVICE_INFOS_INSTANCE(_name)                        \
{                                                               \
    .vsd_version = {_VSD_VERSIONS},                             \
    .device_name = {_name}                                      \
}

#define BLE_START_CONN_PARAMS_INSTANCE()						\
{																\
	.min_conn_interval 	= MSEC_TO_UNITS(15, UNIT_1_25_MS),		\
	.max_conn_interval 	= MSEC_TO_UNITS(15, UNIT_1_25_MS),		\
	.slave_latency 		= 0,									\
	.conn_sup_timeout 	= MSEC_TO_UNITS(4000, UNIT_10_MS),		\
}

#define BLE_START_MTU_SIZE_PARAMS_INSTANCE()					\
{																\
	.max_tx_octets 		= 247 + 4U,                             \
	.max_rx_octets 		= 247 + 4U,                             \
}

#define BLE_PICKIT_GAP_PARAMS_INSTANCE()						\
{																\
	.conn_params = BLE_START_CONN_PARAMS_INSTANCE(),            \
	.phys_params = BLE_GAP_PHY_2MBPS,                           \
	.mtu_size_params = BLE_START_MTU_SIZE_PARAMS_INSTANCE(),    \
	.adv_interval = MSEC_TO_UNITS(100, UNIT_0_625_MS),			\
	.adv_timeout = 18000,										\
}

#define BLE_PICKIT_STATUS_INSTANCE(_name)                       \
{                                                               \
    .infos = BLE_DEVICE_INFOS_INSTANCE(_name),                  \
    .flags = {{0}},                                             \
    .hardware = {0},                                            \
    .connection = {0},                                          \
    .gap = {0, {{0}, 0, {0}, 0, 0}},                            \
}

#define BLE_PICKIT_PARAMS_INSTANCE()							\
{																\
	.preferred_gap_params = BLE_PICKIT_GAP_PARAMS_INSTANCE(),   \
	.pa_lna_enable = false,										\
	.led_status_enable = true,									\
}

#define BLE_PARAMS_INSTANCE(_name)                              \
{                                                               \
	.uart = {0},                                                \
	.incoming_message_uart = {0},                               \
	.status = BLE_PICKIT_STATUS_INSTANCE(_name),                \
    .params = BLE_PICKIT_PARAMS_INSTANCE(),                     \
    .service = {0},                                             \
}

#define BLE_DEF(_var, _name)                                    \
static ble_params_t _var = BLE_PARAMS_INSTANCE(_name)

typedef void (*p_ble_function)(uint8_t *buffer);

void ble_init(UART_MODULE uart_id, uint32_t data_rate, ble_params_t * p_ble_params);
void ble_stack_tasks();

#endif
