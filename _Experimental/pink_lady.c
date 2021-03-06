/*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		21/03/2019		- Initial release
* 
 Electrical characteristics WS2812B:
        DC Voltage = 5V (led in IDLE mode = 400uA)
        I_red = 12mA / I_green = 12mA / I_blue = 12mA
        I_cyan = 24mA / I_orange = 24mA / I_purple = 24mA
        I_white = 36mA
        P_max (white color) = 180mW 
 
        5V 1A (5W)	: 27 leds
        5V 5A (25W)	: 135 leds
        5V 8A (40W)	: 220 leds
 
 Electrical characteristics SK6812RGB Side:
        DC Voltage = 5V (led in IDLE mode = 457uA)
        I_red = 13mA / I_green = 13mA / I_blue = 13mA
        I_cyan = 25mA / I_orange = 25mA / I_purple = 25mA
        I_white = 39mA
        P_max (white color) = 195mW 
 
 Electrical characteristics SK6812RGBW:
        DC Voltage = 5V (led in IDLE mode = 598uA)
        I_red = 10mA / I_green = 10mA / I_blue = 10mA / I_white = 18mA
        I_cyan = 19mA / I_orange = 19mA / I_purple = 19mA
        I_white_rgb = 27mA / I_white_all = 43mA
        P_max (white RGB color) = 135mW 
        P_max (white color) = 90mW 
        P_max (white all color) = 215mW  
 
    WS2812B:    RGB colors
        65 FPS for 512 LEDs on ONE SPI bus.
        30 FPS for 1188 LEDs on ONE SPI bus.
        1 LEDs = 91,5 us (28 us data time + 63,5 us reset time).    
        10 LEDs = 351 us (287,5 us data time + 63,5 us reset time).   
        100 LEDs = 2,94 ms (2876,5 us data time + 63,5 us reset time).  
        1000 LEDs = 28,9 ms (28836,5 us data time + 63,5 us reset time).
    
    SK6812RGBW: RGB+W colors
        53 FPS for 512 LEDs on ONE SPI bus.
        30 FPS for 964 LEDs on ONE SPI bus.
        1 LEDs = 114,5 us (34,5 us data time + 80 us reset time).     
        10 LEDs = 432 us (352 us data time + 80 us reset time).    
        100 LEDs = 3,6 ms (3520 us data time + 80 us reset time).  
        1000 LEDs = 35,3 ms (35220 us data time + 80 us reset time).
 
    SK6812RGB: RGB colors
        73 FPS for 512 LEDs on ONE SPI bus.
        30 FPS for 1279 LEDs on ONE SPI bus.
        1 LEDs = 106 us (26 us data time + 80 us reset time).     
        10 LEDs = 343 us (263 us data time + 80 us reset time).
        100 LEDs = 2,7 ms (2620 us data time + 80 us reset time).  
        1000 LEDs = 26,5 ms (26420 us data time + 80 us reset time).
*********************************************************************/

#include "../PLIB.h"

static const uint32_t sk6812rgbw_ws2812b_mapping[] = 
{
    0x88888888, 0x8e888888, 0xe8888888, 0xee888888, 0x888e8888, 0x8e8e8888, 0xe88e8888, 0xee8e8888, 0x88e88888, 0x8ee88888, 0xe8e88888, 0xeee88888, 0x88ee8888, 0x8eee8888, 0xe8ee8888, 0xeeee8888, 
    0x88888e88, 0x8e888e88, 0xe8888e88, 0xee888e88, 0x888e8e88, 0x8e8e8e88, 0xe88e8e88, 0xee8e8e88, 0x88e88e88, 0x8ee88e88, 0xe8e88e88, 0xeee88e88, 0x88ee8e88, 0x8eee8e88, 0xe8ee8e88, 0xeeee8e88, 
    0x8888e888, 0x8e88e888, 0xe888e888, 0xee88e888, 0x888ee888, 0x8e8ee888, 0xe88ee888, 0xee8ee888, 0x88e8e888, 0x8ee8e888, 0xe8e8e888, 0xeee8e888, 0x88eee888, 0x8eeee888, 0xe8eee888, 0xeeeee888, 
    0x8888ee88, 0x8e88ee88, 0xe888ee88, 0xee88ee88, 0x888eee88, 0x8e8eee88, 0xe88eee88, 0xee8eee88, 0x88e8ee88, 0x8ee8ee88, 0xe8e8ee88, 0xeee8ee88, 0x88eeee88, 0x8eeeee88, 0xe8eeee88, 0xeeeeee88, 
    0x8888888e, 0x8e88888e, 0xe888888e, 0xee88888e, 0x888e888e, 0x8e8e888e, 0xe88e888e, 0xee8e888e, 0x88e8888e, 0x8ee8888e, 0xe8e8888e, 0xeee8888e, 0x88ee888e, 0x8eee888e, 0xe8ee888e, 0xeeee888e, 
    0x88888e8e, 0x8e888e8e, 0xe8888e8e, 0xee888e8e, 0x888e8e8e, 0x8e8e8e8e, 0xe88e8e8e, 0xee8e8e8e, 0x88e88e8e, 0x8ee88e8e, 0xe8e88e8e, 0xeee88e8e, 0x88ee8e8e, 0x8eee8e8e, 0xe8ee8e8e, 0xeeee8e8e, 
    0x8888e88e, 0x8e88e88e, 0xe888e88e, 0xee88e88e, 0x888ee88e, 0x8e8ee88e, 0xe88ee88e, 0xee8ee88e, 0x88e8e88e, 0x8ee8e88e, 0xe8e8e88e, 0xeee8e88e, 0x88eee88e, 0x8eeee88e, 0xe8eee88e, 0xeeeee88e, 
    0x8888ee8e, 0x8e88ee8e, 0xe888ee8e, 0xee88ee8e, 0x888eee8e, 0x8e8eee8e, 0xe88eee8e, 0xee8eee8e, 0x88e8ee8e, 0x8ee8ee8e, 0xe8e8ee8e, 0xeee8ee8e, 0x88eeee8e, 0x8eeeee8e, 0xe8eeee8e, 0xeeeeee8e, 
    0x888888e8, 0x8e8888e8, 0xe88888e8, 0xee8888e8, 0x888e88e8, 0x8e8e88e8, 0xe88e88e8, 0xee8e88e8, 0x88e888e8, 0x8ee888e8, 0xe8e888e8, 0xeee888e8, 0x88ee88e8, 0x8eee88e8, 0xe8ee88e8, 0xeeee88e8, 
    0x88888ee8, 0x8e888ee8, 0xe8888ee8, 0xee888ee8, 0x888e8ee8, 0x8e8e8ee8, 0xe88e8ee8, 0xee8e8ee8, 0x88e88ee8, 0x8ee88ee8, 0xe8e88ee8, 0xeee88ee8, 0x88ee8ee8, 0x8eee8ee8, 0xe8ee8ee8, 0xeeee8ee8, 
    0x8888e8e8, 0x8e88e8e8, 0xe888e8e8, 0xee88e8e8, 0x888ee8e8, 0x8e8ee8e8, 0xe88ee8e8, 0xee8ee8e8, 0x88e8e8e8, 0x8ee8e8e8, 0xe8e8e8e8, 0xeee8e8e8, 0x88eee8e8, 0x8eeee8e8, 0xe8eee8e8, 0xeeeee8e8, 
    0x8888eee8, 0x8e88eee8, 0xe888eee8, 0xee88eee8, 0x888eeee8, 0x8e8eeee8, 0xe88eeee8, 0xee8eeee8, 0x88e8eee8, 0x8ee8eee8, 0xe8e8eee8, 0xeee8eee8, 0x88eeeee8, 0x8eeeeee8, 0xe8eeeee8, 0xeeeeeee8, 
    0x888888ee, 0x8e8888ee, 0xe88888ee, 0xee8888ee, 0x888e88ee, 0x8e8e88ee, 0xe88e88ee, 0xee8e88ee, 0x88e888ee, 0x8ee888ee, 0xe8e888ee, 0xeee888ee, 0x88ee88ee, 0x8eee88ee, 0xe8ee88ee, 0xeeee88ee, 
    0x88888eee, 0x8e888eee, 0xe8888eee, 0xee888eee, 0x888e8eee, 0x8e8e8eee, 0xe88e8eee, 0xee8e8eee, 0x88e88eee, 0x8ee88eee, 0xe8e88eee, 0xeee88eee, 0x88ee8eee, 0x8eee8eee, 0xe8ee8eee, 0xeeee8eee, 
    0x8888e8ee, 0x8e88e8ee, 0xe888e8ee, 0xee88e8ee, 0x888ee8ee, 0x8e8ee8ee, 0xe88ee8ee, 0xee8ee8ee, 0x88e8e8ee, 0x8ee8e8ee, 0xe8e8e8ee, 0xeee8e8ee, 0x88eee8ee, 0x8eeee8ee, 0xe8eee8ee, 0xeeeee8ee, 
    0x8888eeee, 0x8e88eeee, 0xe888eeee, 0xee88eeee, 0x888eeeee, 0x8e8eeeee, 0xe88eeeee, 0xee8eeeee, 0x88e8eeee, 0x8ee8eeee, 0xe8e8eeee, 0xeee8eeee, 0x88eeeeee, 0x8eeeeeee, 0xe8eeeeee, 0xeeeeeeee
};

static pink_lady_manager_params_t pink_lady_manager_tab[SPI_NUMBER_OF_MODULES][PL_ID_MAX];

/*******************************************************************************
 * Function: 
 *      void pink_lady_deamon(PINK_LADY_PARAMS *var)
 * 
 * Description:
 *      This routine is the main state machine for managing a PINK LADY driver.
 * 
 * Parameters:
 *      *var:   A PINK_LADY_PARAMS pointer used by the user/driver to manage the 
 *              state machine.
 * 
 * Return:
 *      none
 ******************************************************************************/
void pink_lady_deamon(pink_lady_params_t *var)
{
    if (!var->is_init_done)
    {
        
        
        PINK_LADY_MANAGER_IDENTIFIERS i;
        for (i = 0 ; i < PL_ID_MAX ; i++)
        {
            pink_lady_manager_tab[var->spi_id][i].status = PL_SEGMENT_FREE;
            pink_lady_manager_tab[var->spi_id][i].sm.index = 0;
            pink_lady_manager_tab[var->spi_id][i].p_led = var->p_led;
            pink_lady_manager_tab[var->spi_id][i].p_led_copy = var->p_led_copy;
        }
        
        
        var->dma_id = dma_get_free_channel();
        
        spi_init(   var->spi_id, 
                    NULL, 
                    IRQ_NONE, 
                    ((var->led_model & SK6812RGBW_INDICE_MASK) > 0) ? SK6812RGBW_TIMING : (((var->led_model & SK6812RGB_INDICE_MASK) > 0) ? SK6812RGB_TIMING : WS2812B_TIMING), 
                    SPI_STD_MASTER_CONFIG);
        
        dma_init(   var->dma_id, 
                    NULL, 
                    DMA_CONT_PRIO_3 | DMA_CONT_AUTO_ENABLE, 
                    DMA_INT_NONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    spi_get_tx_irq(var->spi_id), 
                    0xff);
        
        var->p_led_model_mapping = (uint32_t *)sk6812rgbw_ws2812b_mapping;
        var->dma_params.dst_start_addr = (void *) spi_get_tx_reg(var->spi_id);       
        dma_set_transfer_params(var->dma_id, &var->dma_params);     // The DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer_params()).
                                                                    // Because the DMA channel is configured in "AUTO_ENABLE" then there is no need to re-call "dma_channel_enable()" to re-execute a DMA transfer (the channel is automatically put "ENABLE" at the end of a transmission)
        dma_channel_enable(var->dma_id, ON, false);
        var->is_init_done = true;
    }
    else
    {      
        if ((var->led_model & SK6812RGBW_INDICE_MASK) > 0)
        {
            var->p_buffer[0 + 4*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].green];
            var->p_buffer[1 + 4*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].red];
            var->p_buffer[2 + 4*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].blue];
            var->p_buffer[3 + 4*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].white];
        }
        else if ((var->led_model & SK6812RGB_INDICE_MASK) > 0)
        {
            var->p_buffer[0 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].green];
            var->p_buffer[1 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].red];
            var->p_buffer[2 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].blue];
        }
        else if ((var->led_model & WS2812B_INDICE_MASK) > 0)
        {
            var->p_buffer[0 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].green];
            var->p_buffer[1 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].red];
            var->p_buffer[2 + 3*var->ind_update_tx_buffer] = var->p_led_model_mapping[var->p_led[var->ind_update_tx_buffer].blue];
        }
        
        if (++var->ind_update_tx_buffer >= var->number_of_leds)
        {
            var->ind_update_tx_buffer = 0;
        }
    }
}

/*
 * Case delay = 0:
 * LED.red = LowestValue.red + (i x delta.red / N)
 * 
 * Case delay > 0:
 * LED.red = LED_CPY.red + ((LowestValue.red + (i x delta.red / N) - LED_CPY.red) x I / 100)
 * 
 * with:
 *   - LowestValue.red: The lowest red value between color1 red and color2 red
 *   - delta.red: The absolute difference red value between color1 red and color2 red
 *   - N: The number of concerned LED
 *   - i: The LED index (from 1 to N)
 *   - I: The intensity (only for delay > 0) (from 0 to 100)
 */
/*******************************************************************************
 * Function: 
 *      uint8_t pink_lady_set_segment_params(
 *                  pink_lady_params_t *var, 
 *                  PINK_LADY_MANAGER_IDENTIFIERS id,
 *                  uint16_t from, 
 *                  uint16_t to, 
 *                  rgbw_color_t color1, 
 *                  rgbw_color_t color2, 
 *                  PINK_LADY_RESOLUTIONS resolution, 
 *                  uint32_t deadline_to_appear)
 * 
 * Description:
 *      This routine is used to update a segment (part of leds on a bus) with
 *      basic parameters (gradient, led resolution, deadline to appear...).
 *      It returns its state machine index.
 * 
 * Parameters:
 *      *p_seg_params:      A PINK_LADY_SEGMENT_PARAMS pointer used by the 
 *                          user/driver to manage the segment.
 *      from:               The first LED index for which the parameters will be apply.
 *                          This "from" value should be always inferior or equal to "to" value.
 *      to:                 The last LED index for which the parameters will be apply.
 *                          This "to" value should be always superior or equal to "from" value.
 *      color1:             The first color (RGBW model).
 *      color2:             The second (and last) color (RGBW model).
 *      resolution:         The step between each LED (see PINK_LADY_RESOLUTIONS for
 *                          more details on available resolutions). For instance 
 *                          a value of LED_RESO_1_3 will lit one LED on three.
 *      deadline_to_appear: The total time for a dynamic change. Set this parameter
 *                          to zero if you want to apply instantly the new parameters
 *                          (without dynamic change).
 * 
 * Return:
 *      0:      Home (finish)
 *      >0:     On going
 ******************************************************************************/
uint8_t pink_lady_set_segment_params(pink_lady_params_t *var, PINK_LADY_MANAGER_IDENTIFIERS id, uint16_t from, uint16_t to, rgbw_color_t color1, rgbw_color_t color2, PINK_LADY_RESOLUTIONS resolution, uint32_t deadline_to_appear)
{
    
    if (pink_lady_manager_tab[var->spi_id][id].status != PL_SEGMENT_FINISHED)
    {
    
        switch (pink_lady_manager_tab[var->spi_id][id].sm.index)
        {
            case 0: // Home            

                //0. Get delta colors between color1 and color2
                pink_lady_manager_tab[var->spi_id][id].delta_color.red = (uint8_t) ((color1.red <= color2.red) ? (color2.red - color1.red) : (color1.red - color2.red));
                pink_lady_manager_tab[var->spi_id][id].delta_color.green = (uint8_t) ((color1.green <= color2.green) ? (color2.green - color1.green) : (color1.green - color2.green));
                pink_lady_manager_tab[var->spi_id][id].delta_color.blue = (uint8_t) ((color1.blue <= color2.blue) ? (color2.blue - color1.blue) : (color1.blue - color2.blue));
                pink_lady_manager_tab[var->spi_id][id].delta_color.white = (uint8_t) ((color1.white <= color2.white) ? (color2.white - color1.white) : (color1.white - color2.white));

                //1. Set color pointers on appropriate indices type (positive or negative)
                pink_lady_manager_tab[var->spi_id][id].p_ind_red = (uint16_t *) ((color1.red <= color2.red) ? &pink_lady_manager_tab[var->spi_id][id].ind_pos : &pink_lady_manager_tab[var->spi_id][id].ind_neg);
                pink_lady_manager_tab[var->spi_id][id].p_ind_green = (uint16_t *) ((color1.green <= color2.green) ? &pink_lady_manager_tab[var->spi_id][id].ind_pos : &pink_lady_manager_tab[var->spi_id][id].ind_neg);
                pink_lady_manager_tab[var->spi_id][id].p_ind_blue = (uint16_t *) ((color1.blue <= color2.blue) ? &pink_lady_manager_tab[var->spi_id][id].ind_pos : &pink_lady_manager_tab[var->spi_id][id].ind_neg);
                pink_lady_manager_tab[var->spi_id][id].p_ind_white = (uint16_t *) ((color1.white <= color2.white) ? &pink_lady_manager_tab[var->spi_id][id].ind_pos : &pink_lady_manager_tab[var->spi_id][id].ind_neg);            

                //2. Get the number of LED to lit
                pink_lady_manager_tab[var->spi_id][id].number_of_led = (uint16_t) (to - from + 1);

                //3. Set the indices
                pink_lady_manager_tab[var->spi_id][id].ind_pos = from;     
                pink_lady_manager_tab[var->spi_id][id].ind_neg = to;   

                pink_lady_manager_tab[var->spi_id][id].sm.index = (deadline_to_appear > 0) ? 3 : 1;
                pink_lady_manager_tab[var->spi_id][id].sm.tick = mGetTick();
                
                pink_lady_manager_tab[var->spi_id][id].status = PL_SEGMENT_BUSY;
                break;

            case 1:// Deadline_to_appear == 0 (part 0: initialization)

                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_red = (uint8_t *) ((color1.red <= color2.red) ? &color1.red : &color2.red);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_green = (uint8_t *) ((color1.green <= color2.green) ? &color1.green : &color2.green);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_blue = (uint8_t *) ((color1.blue <= color2.blue) ? &color1.blue : &color2.blue);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_white = (uint8_t *) ((color1.white <= color2.white) ? &color1.white : &color2.white);

                pink_lady_manager_tab[var->spi_id][id].sm.index = 2;

            case 2: // Deadline_to_appear == 0 (part 1: led update)

                pink_lady_manager_tab[var->spi_id][id].ind_ = pink_lady_manager_tab[var->spi_id][id].ind_pos - from;

                // (!(resolution & LED_RESO_JUMP) ? 0 : pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red)
                
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red       = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_red % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red : 0) : (uint32_t)(*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_red + pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.red / pink_lady_manager_tab[var->spi_id][id].number_of_led));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green   = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_green % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green : 0) : (uint32_t)(*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_green + pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.green / pink_lady_manager_tab[var->spi_id][id].number_of_led));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue     = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_blue % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue : 0) : (uint32_t)(*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_blue + pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.blue / pink_lady_manager_tab[var->spi_id][id].number_of_led));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white   = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_white % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white : 0) : (uint32_t)(*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_white + pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.white / pink_lady_manager_tab[var->spi_id][id].number_of_led));

                --pink_lady_manager_tab[var->spi_id][id].ind_neg;
                if (++pink_lady_manager_tab[var->spi_id][id].ind_pos > to)
                {
                    pink_lady_manager_tab[var->spi_id][id].sm.index = 0;
                    pink_lady_manager_tab[var->spi_id][id].status = PL_SEGMENT_FINISHED;
                }
                break;

            case 3:// Deadline_to_appear > 0 (part 0: initialization)
                
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_red = (uint8_t *) ((color1.red <= color2.red) ? &color1.red : &color2.red);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_green = (uint8_t *) ((color1.green <= color2.green) ? &color1.green : &color2.green);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_blue = (uint8_t *) ((color1.blue <= color2.blue) ? &color1.blue : &color2.blue);
                pink_lady_manager_tab[var->spi_id][id].p_lowest_value_white = (uint8_t *) ((color1.white <= color2.white) ? &color1.white : &color2.white);
                //1. Save current LEDs segment
                memcpy(&pink_lady_manager_tab[var->spi_id][id].p_led_copy[from], &pink_lady_manager_tab[var->spi_id][id].p_led[from], pink_lady_manager_tab[var->spi_id][id].number_of_led * sizeof(rgbw_color_t));            
                //2. Set time for intensity
                pink_lady_manager_tab[var->spi_id][id].time_between_increment = (uint32_t) (deadline_to_appear / 100);            
                //3. Reset intensity value
                pink_lady_manager_tab[var->spi_id][id].intensity = 0;            
                pink_lady_manager_tab[var->spi_id][id].sm.index = 4;

            case 4: // Deadline_to_appear > 0 (part 1: led update)

                pink_lady_manager_tab[var->spi_id][id].ind_ = pink_lady_manager_tab[var->spi_id][id].ind_pos - from;

                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red    = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_red % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red : 0) : (uint32_t)(pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red + (*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_red + (pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.red / pink_lady_manager_tab[var->spi_id][id].number_of_led) - pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_red].red) * pink_lady_manager_tab[var->spi_id][id].intensity / 100));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green  = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_green % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green : 0) : (uint32_t)(pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green + (*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_green + (pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.green / pink_lady_manager_tab[var->spi_id][id].number_of_led) - pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_green].green) * pink_lady_manager_tab[var->spi_id][id].intensity / 100));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue   = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_blue % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue : 0) : (uint32_t)(pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue + (*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_blue + (pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.blue / pink_lady_manager_tab[var->spi_id][id].number_of_led) - pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_blue].blue) * pink_lady_manager_tab[var->spi_id][id].intensity / 100));
                pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white  = (uint8_t) (((*pink_lady_manager_tab[var->spi_id][id].p_ind_white % (resolution & LED_RESO_1_255)) > 0) ? (((resolution & LED_RESO_JUMP) > 0) ? pink_lady_manager_tab[var->spi_id][id].p_led[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white : 0) : (uint32_t)(pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white + (*pink_lady_manager_tab[var->spi_id][id].p_lowest_value_white + (pink_lady_manager_tab[var->spi_id][id].ind_ * pink_lady_manager_tab[var->spi_id][id].delta_color.white / pink_lady_manager_tab[var->spi_id][id].number_of_led) - pink_lady_manager_tab[var->spi_id][id].p_led_copy[*pink_lady_manager_tab[var->spi_id][id].p_ind_white].white) * pink_lady_manager_tab[var->spi_id][id].intensity / 100));

                --pink_lady_manager_tab[var->spi_id][id].ind_neg;
                if (++pink_lady_manager_tab[var->spi_id][id].ind_pos > to)
                {
                    pink_lady_manager_tab[var->spi_id][id].sm.index = (pink_lady_manager_tab[var->spi_id][id].intensity == 100) ? 0 : 5;
                    if (pink_lady_manager_tab[var->spi_id][id].sm.index == 0)
                    {
                        pink_lady_manager_tab[var->spi_id][id].sm.index = 0;
                        pink_lady_manager_tab[var->spi_id][id].status = PL_SEGMENT_FINISHED;
                    }
                }
                break;

            case 5: // Deadline_to_appear > 0 (part 2: intensity update)

                if (mTickCompare(pink_lady_manager_tab[var->spi_id][id].sm.tick) > pink_lady_manager_tab[var->spi_id][id].time_between_increment)
                {
                    mUpdateTick_withCathingUpTime(pink_lady_manager_tab[var->spi_id][id].sm.tick, pink_lady_manager_tab[var->spi_id][id].time_between_increment);
                    pink_lady_manager_tab[var->spi_id][id].intensity++;
                    pink_lady_manager_tab[var->spi_id][id].sm.index = 4;
                    pink_lady_manager_tab[var->spi_id][id].ind_pos = from; 
                    pink_lady_manager_tab[var->spi_id][id].ind_neg = to;                       
                }
                break;

            default:
                break;
        }
        
    }
    
    return pink_lady_manager_tab[var->spi_id][id].sm.index;
}

PINK_LADY_MANAGER_STATUS pink_lady_get_segment_status(pink_lady_params_t var, PINK_LADY_MANAGER_IDENTIFIERS id)
{
    return pink_lady_manager_tab[var.spi_id][id].status;
}

void pink_lady_reset_segment(pink_lady_params_t var, PINK_LADY_MANAGER_IDENTIFIERS id)
{
    pink_lady_manager_tab[var.spi_id][id].status = PL_SEGMENT_FREE;
    pink_lady_manager_tab[var.spi_id][id].sm.index = 0;
}

void pink_lady_reset_all_segments(pink_lady_params_t var)
{
    uint8_t id = 0;
    
    for (id = 0 ; id < PL_ID_MAX ; id++)
    {
        pink_lady_manager_tab[var.spi_id][id].status = PL_SEGMENT_FREE;
        pink_lady_manager_tab[var.spi_id][id].sm.index = 0;
    }
}

/*
 * Timings _shift when executing the memcpy:
 * For 10 bytes (0..9): 1,48 us
 * For 100 bytes (0..99): 12,16 us
 * For 1000 bytes (0..999): 119 us
 */
uint8_t pink_lady_shift_pattern(pink_lady_shift_params_t *var)
{
    if (var->reset_requested)
    {
        var->reset_requested = false;
        var->current_iteration = 0;
        mUpdateTick(var->tick);
    }
    
    if (var->enable)
    {
        if (mTickCompare(var->tick) >= var->refresh_time)
        {            
            mUpdateTick_withCathingUpTime(var->tick, var->refresh_time);
            
            if (var->direction == PL_SHIFT_FROM_TO_TO)
            {
                rgbw_color_t t_rgbw = var->p_led[var->to];        
                memcpy(&var->p_led_copy[var->from], &var->p_led[var->from], (var->to - var->from + 1)*4);
                memcpy(&var->p_led[var->from + 1], &var->p_led_copy[var->from], (var->to - var->from)*4);
                var->p_led[var->from] = t_rgbw;
            }
            else
            {
                rgbw_color_t t_rgbw = var->p_led[var->from];        
                memcpy(&var->p_led_copy[var->from], &var->p_led[var->from], (var->to - var->from + 1)*4);
                memcpy(&var->p_led[var->from], &var->p_led_copy[var->from + 1], (var->to - var->from)*4);
                var->p_led[var->to] = t_rgbw;
            }
            
            if (var->number_of_iterations > 0)
            {
                var->current_iteration++;
                if (var->current_iteration >= var->number_of_iterations)
                {
                    var->current_iteration = 0;
                    var->enable = OFF;
                    return 0;
                }
            }
        }
    }
    else
    {
        return 2;
    }
    return 1;
}
