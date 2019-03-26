#ifndef __DEF_PINK_LADY
#define __DEF_PINK_LADY

#define WS2812B_TIMING          2500000
#define SK6812RGBW_TIMING       3333333

typedef enum
{
    // DO NOT MODIFY
    // xxx_MODEL = 0x00 aaaaaa bbb ccccc with a = reset size in byte (6 bits) / b = indice LED model (3 bits) / c = number_of_color_chip x number_of_spi_byte_per_color_chip (5 bits)
    WS2812B_MODEL               = 0x102c,     // Reset = 16 bytes @ 2,500000 MHz SPI for Reset Time at least 50 us, Indice = 1, Size = 3 x 4 = 12
    SK6812RGBW_MODEL            = 0x2250,     // Reset = 34 bytes @ 3,333333 MHz SPI for Reset Time at least 80 us, Indice = 2, Size = 4 x 4 = 16
    
    WS2812B_INDICE_MASK         = 0x0020,
    SK6812RGBW_INDICE_MASK      = 0x0040,
    LED_INDICE_MASK             = 0x00e0
} PINK_LADY_MODELS;

typedef struct
{
    uint8_t                     red;
    uint8_t                     green;
    uint8_t                     blue;
    uint8_t                     white;
} RGBW_COLOR;

typedef struct
{
    bool                        is_init_done;
    SPI_MODULE                  spi_id;
    DMA_MODULE                  dma_id;
    DMA_CHANNEL_TRANSFER        dma_params;
    PINK_LADY_MODELS            led_model;
    uint16_t                    number_of_leds;
    RGBW_COLOR                  *p_led;
    uint32_t                    *p_buffer;
    uint32_t                    *p_led_model_mapping;
    uint16_t                    ind_update_tx_buffer;
} PINK_LADY_PARAMS;

#define PINK_LADY_INSTANCE(_spi_id, _dma_id, _led_model, _led_ram_buffer, _tx_buffer_ram, _number_total_of_leds)                    \
{                                                                                                                                   \
    .is_init_done = 0,                                                                                                              \
    .spi_id = _spi_id,                                                                                                              \
    .dma_id = _dma_id,                                                                                                              \
    .dma_params = {_tx_buffer_ram, NULL, sizeof(_tx_buffer_ram), 1, 1, 0},                                                          \
    .led_model = _led_model,                                                                                                        \
    .number_of_leds = _number_total_of_leds,                                                                                        \
    .p_led = _led_ram_buffer,                                                                                                       \
    .p_buffer = (uint32_t *)_tx_buffer_ram,                                                                                         \
    .p_led_model_mapping = NULL,                                                                                                    \
    .ind_update_tx_buffer = 0                                                                                                       \
}

#define PINK_LADY_DEF(_name, _spi_id, _dma_id, _led_model, _number_total_of_leds)                                                   \
static uint8_t _name ## _tx_buffer_ram_allocation[_number_total_of_leds * (_led_model & 0x1f) + ((_led_model >> 8) & 0x3f)] = {0};  \
static RGBW_COLOR _name ## _led_ram_allocation[_number_total_of_leds] = {0};                                                        \
static PINK_LADY_PARAMS _name = PINK_LADY_INSTANCE(_spi_id, _dma_id, _led_model, _name ## _led_ram_allocation, _name ## _tx_buffer_ram_allocation, _number_total_of_leds)	

void pink_lady_deamon(PINK_LADY_PARAMS *var);

#define pink_lady_set_led_rgb(var, ind, r, g, b)        (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b)
#define pink_lady_set_led_rgbw(var, ind, r, g, b, w)    (var.p_led[ind].red = r, var.p_led[ind].green = g, var.p_led[ind].blue = b, var.p_led[ind].white = w)

#endif
