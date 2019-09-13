#ifndef __DEF_PINK_LADY
#define __DEF_PINK_LADY

#define WS2812B_TIMING          3200000
#define SK6812RGBW_TIMING       3500000
#define SK6812RGB_TIMING        3500000

typedef enum
{
    // DO NOT MODIFY
    // xxx_MODEL = 0x00 aaaaaa bbb ccccc with a = reset size in byte (6 bits) / b = indice LED model (3 bits) / c = number_of_color_chip x number_of_spi_byte_per_color_chip (5 bits)
    WS2812B_MODEL               = 0x1a2c,     // Reset = 26 bytes @ 3,200000 MHz SPI for Reset Time at least 60 us, Indice = 1, Size = 3 x 4 = 12
    SK6812RGBW_MODEL            = 0x2450,     // Reset = 36 bytes @ 3,500000 MHz SPI for Reset Time at least 80 us, Indice = 2, Size = 4 x 4 = 16
    SK6812RGB_MODEL             = 0x248c,     // Reset = 36 bytes @ 3,500000 MHz SPI for Reset Time at least 80 us, Indice = 4, Size = 3 x 4 = 12
    
    WS2812B_INDICE_MASK         = 0x0020,
    SK6812RGBW_INDICE_MASK      = 0x0040,
    SK6812RGB_INDICE_MASK       = 0x0080,
    LED_INDICE_MASK             = 0x00e0
} PINK_LADY_MODELS;

typedef enum
{
    LED_RESO_ALL                = 0x01,
    LED_RESO_1_2,
    LED_RESO_1_3,
    LED_RESO_1_4,
    LED_RESO_1_5,
    LED_RESO_1_6,
    LED_RESO_1_7,
    LED_RESO_1_8,
    LED_RESO_1_9,
    LED_RESO_1_255              = 0xff,
            
    LED_RESO_CLEAR              = 0x000,    // Turn off the LEDs if resolution is equal or greater then LED_RESO_1_2
    LED_RESO_JUMP               = 0x100     // Keep the existing color if resolution is equal or greater then LED_RESO_1_2
} PINK_LADY_RESOLUTIONS;

typedef enum
{
    PL_SHIFT_FROM_TO_TO         = 0,
    PL_SHIFT_TO_TO_FROM
} PINK_LADY_DIRECTION;

typedef struct
{
    bool                        enable;
    PINK_LADY_DIRECTION         direction;
    uint16_t                    from;
    uint16_t                    to;
    uint64_t                    refresh_time;
    
    RGBW_COLOR                  * p_led;
    RGBW_COLOR                  * p_led_copy;
    
    bool                        reset_requested;
    uint32_t                    current_iteration;
    uint32_t                    number_of_iterations;
    uint64_t                    tick;
} pink_lady_shift_params_t;

#define PINK_LADY_SHIFT_INSTANCE(_p_pink_lady_params, _direction, _from, _to, _number_of_cycles, _period)    \
{                                                                                       \
    .enable = OFF,                                                                      \
    .direction = _direction,                                                            \
    .from = _from,                                                                      \
    .to = _to,                                                                          \
    .refresh_time = _period,                                                            \
    .p_led = (RGBW_COLOR*) _p_pink_lady_params ## _led_ram_allocation,                  \
    .p_led_copy = (RGBW_COLOR*) _p_pink_lady_params ## _copy_led_ram_allocation,        \
    .reset_requested = true,                                                            \
    .current_iteration = 0,                                                             \
    .number_of_iterations = ((_to - _from + 1) * _number_of_cycles),                    \
    .tick = TICK_INIT                                                                   \
}

#define PICK_LADY_SHIFT_DEF(_name, _p_pink_lady_params, _direction, _from, _to, _number_of_cycles, _period)            \
static pink_lady_shift_params_t _name = PINK_LADY_SHIFT_INSTANCE(_p_pink_lady_params, _direction, _from, _to, _number_of_cycles, _period)

typedef enum
{
    PL_SEGMENT_FREE             = 0,
    PL_SEGMENT_BUSY,
    PL_SEGMENT_FINISHED
} PINK_LADY_MANAGER_STATUS;

typedef enum
{
    PL_ID_0                     = 0,
    PL_ID_1,
    PL_ID_2,
    PL_ID_3,
    PL_ID_4,
    PL_ID_5,
    PL_ID_6,
    PL_ID_7,
    PL_ID_8,
    PL_ID_9,
    PL_ID_10,
    PL_ID_11,
    PL_ID_12,
    PL_ID_13,
    PL_ID_14,
    PL_ID_15,
    PL_ID_16,
    PL_ID_17,
    PL_ID_18,
    PL_ID_19,
    
    PL_ID_MAX
} PINK_LADY_MANAGER_IDENTIFIERS;

typedef struct
{
    RGBW_COLOR                  delta_color;
    uint16_t                    number_of_led;
    uint8_t                     intensity;
    uint16_t                    ind_pos;
    uint16_t                    ind_neg;
    uint16_t                    ind_;
    uint32_t                    time_between_increment;
    PINK_LADY_MANAGER_STATUS    status;
    state_machine_t             sm;
    
    RGBW_COLOR                  * p_led;
    RGBW_COLOR                  * p_led_copy;
    
    uint16_t                    * p_ind_red;
    uint16_t                    * p_ind_green;
    uint16_t                    * p_ind_blue;
    uint16_t                    * p_ind_white;
    
    uint8_t                     * p_lowest_value_red;
    uint8_t                     * p_lowest_value_green;
    uint8_t                     * p_lowest_value_blue;
    uint8_t                     * p_lowest_value_white;
} pink_lady_manager_params_t;

typedef struct
{
    bool                        is_init_done;
    SPI_MODULE                  spi_id;
    DMA_MODULE                  dma_id;
    DMA_CHANNEL_TRANSFER        dma_params;
    PINK_LADY_MODELS            led_model;
    uint16_t                    number_of_leds;
    RGBW_COLOR                  *p_led;
    RGBW_COLOR                  *p_led_copy;
    uint32_t                    *p_buffer;
    uint32_t                    *p_led_model_mapping;
    uint16_t                    ind_update_tx_buffer;
} pink_lady_params_t;

#define PINK_LADY_INSTANCE(_spi_id, _led_model, _led_ram_buffer, _copy_led_ram_buffer, _tx_buffer_ram, _number_total_of_leds)  \
{                                                                                                                                       \
    .is_init_done = 0,                                                                                                                  \
    .spi_id = _spi_id,                                                                                                                  \
    .dma_id = DMA_NUMBER_OF_MODULES,                                                                                                    \
    .dma_params = {_tx_buffer_ram, NULL, sizeof(_tx_buffer_ram), 1, 1, 0},                                                              \
    .led_model = _led_model,                                                                                                            \
    .number_of_leds = _number_total_of_leds,                                                                                            \
    .p_led = _led_ram_buffer,                                                                                                           \
    .p_led_copy = _copy_led_ram_buffer,                                                                                                 \
    .p_buffer = (uint32_t *)_tx_buffer_ram,                                                                                             \
    .p_led_model_mapping = NULL,                                                                                                        \
    .ind_update_tx_buffer = 0                                                                                                           \
}

#define PINK_LADY_DEF(_name, _spi_id, _led_model, _number_total_of_leds)                                                       \
static uint8_t _name ## _tx_buffer_ram_allocation[_number_total_of_leds * (_led_model & 0x1f) + ((_led_model >> 8) & 0x3f)] = {0};      \
static RGBW_COLOR _name ## _led_ram_allocation[_number_total_of_leds] = {0};                                                            \
static RGBW_COLOR _name ## _copy_led_ram_allocation[_number_total_of_leds] = {0};                                                       \
static pink_lady_params_t _name = PINK_LADY_INSTANCE(_spi_id, _led_model, _name ## _led_ram_allocation, _name ## _copy_led_ram_allocation, _name ## _tx_buffer_ram_allocation, _number_total_of_leds)	




void pink_lady_deamon(pink_lady_params_t *var);

uint8_t pink_lady_set_segment_params(pink_lady_params_t *var, PINK_LADY_MANAGER_IDENTIFIERS id, uint16_t from, uint16_t to, RGBW_COLOR color1, RGBW_COLOR color2, PINK_LADY_RESOLUTIONS resolution, uint32_t deadline_to_appear);
PINK_LADY_MANAGER_STATUS pink_lady_get_segment_status(pink_lady_params_t var, PINK_LADY_MANAGER_IDENTIFIERS id);
void pink_lady_reset_segment(pink_lady_params_t var, PINK_LADY_MANAGER_IDENTIFIERS id);
#define pink_lady_release_segment(var, id)                      pink_lady_reset_segment(var, id)

uint8_t pink_lady_shift_pattern(pink_lady_shift_params_t *var);
#define pink_lady_shift_pattern_stop(var)                       (var.enable = OFF)
#define pink_lady_shift_pattern_start(var)                      (var.enable = ON)
#define pink_lady_shift_pattern_reset_and_start(var)            (var.enable = ON, var.reset_requested = true)
#define pink_lady_shift_pattern_reset_and_stop(var)             (var.enable = OFF, var.reset_requested = true)
#define pink_lady_shift_pattern_set_refresh_time(var, time)     (var.refresh_time = time)
#define pink_lady_shift_pattern_set_cycles(var, cycles)         ((var.number_of_iterations = ((var.to - var.from + 1) * cycles)), pink_lady_shift_pattern_reset_and_stop(var))
#define pink_lady_shift_pattern_toggle_direction(var)           ((var.direction = !var.direction), pink_lady_shift_pattern_reset_and_stop(var))
#define pink_lady_shift_pattern_set_direction_from_to_to(var)   ((var.direction = PL_SHIFT_FROM_TO_TO), pink_lady_shift_pattern_reset_and_stop(var))
#define pink_lady_shift_pattern_set_direction_to_to_from(var)   ((var.direction = PL_SHIFT_TO_TO_FROM), pink_lady_shift_pattern_reset_and_stop(var))

#define pink_lady_put_pattern(var, pattern, from, to)           memcpy(&var.p_led[from], pattern, (sizeof(pattern) >= ((to - from + 1) * 4)) ? ((to - from + 1) * 4) : (sizeof(pattern) * 4))
#define pink_lady_set_led_rgb(var, ind, r, g, b)                (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b)
#define pink_lady_set_led_rgbw(var, ind, r, g, b, w)            (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b, var.p_led[ind].white = w)

#define pink_lady_set_all_led_off(var)                          (memset(var.p_led, 0, var.number_of_leds * 4))
#define pink_lady_set_all_led_on(var)                           (memset(var.p_led, 255, var.number_of_leds * 4))

#endif
