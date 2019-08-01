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
    LED_RESO_ALL                = 1,
    LED_RESO_1_2,
    LED_RESO_1_3,
    LED_RESO_1_4,
    LED_RESO_1_5,
    LED_RESO_1_6,
    LED_RESO_1_7,
    LED_RESO_1_8,
    LED_RESO_1_9,
    LED_RESO_1_255              = 255
} PINK_LADY_RESOLUTIONS;

typedef struct
{
    RGBW_COLOR                  delta_color;
    uint16_t                    number_of_led;
    uint8_t                     intensity;
    uint16_t                    ind_pos;
    uint16_t                    ind_neg;
    uint16_t                    ind_;
    uint32_t                    time_between_increment;
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
} PINK_LADY_SEGMENT_PARAMS;

#define PINK_LADY_SEGMENT_INSTANCE(_p_pink_lady_params)                                 \
{                                                                                       \
    .delta_color = { 0, 0, 0, 0 },                                                      \
    .number_of_led = 0,                                                                 \
    .intensity = 0,                                                                     \
    .ind_pos = 0,                                                                       \
    .ind_neg = 0,                                                                       \
    .ind_ = 0,                                                                          \
    .time_between_increment = 0,                                                        \
    .sm = { 0, 0 },                                                                     \
    .p_led = (RGBW_COLOR*) _p_pink_lady_params ## _led_ram_allocation,                  \
    .p_led_copy = (RGBW_COLOR*) _p_pink_lady_params ## _copy_led_ram_allocation,        \
    .p_ind_red = NULL,                                                                  \
    .p_ind_green = NULL,                                                                \
    .p_ind_blue = NULL,                                                                 \
    .p_ind_white = NULL,                                                                \
    .p_lowest_value_red = NULL,                                                         \
    .p_lowest_value_green = NULL,                                                       \
    .p_lowest_value_blue = NULL,                                                        \
    .p_lowest_value_white = NULL                                                        \
}

#define PINK_LADY_SEGMENT_DEF(_name, _p_pink_lady_params)                               \
static PINK_LADY_SEGMENT_PARAMS _name = PINK_LADY_SEGMENT_INSTANCE(_p_pink_lady_params)

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
} PINK_LADY_PARAMS;

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
static PINK_LADY_PARAMS _name = PINK_LADY_INSTANCE(_spi_id, _led_model, _name ## _led_ram_allocation, _name ## _copy_led_ram_allocation, _name ## _tx_buffer_ram_allocation, _number_total_of_leds)	

void pink_lady_deamon(PINK_LADY_PARAMS *var);
uint8_t pink_lady_set_segment_params(PINK_LADY_SEGMENT_PARAMS *p_seg_params, uint16_t from, uint16_t to, RGBW_COLOR color1, RGBW_COLOR color2, PINK_LADY_RESOLUTIONS resolution, uint32_t deadline_to_appear);

#define pink_lady_reset_segment_params(seg_params)      (seg_params.sm.index = 0)
#define pink_lady_is_segment_busy(seg_params)           ((seg_params.sm.index > 0) ? true : false)

#define pink_lady_set_led_rgb(var, ind, r, g, b)        (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b)
#define pink_lady_set_led_rgbw(var, ind, r, g, b, w)    (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b, var.p_led[ind].white = w)

#endif
