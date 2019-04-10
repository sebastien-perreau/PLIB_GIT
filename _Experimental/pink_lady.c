/*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		21/03/2019		- Initial release
* 
    WS2812B:    RGB colors
        65 FPS for 512 LEDs on ONE SPI bus.
        DC Voltage = 5V (led in IDLE mode = 400uA)
        I_red = 12mA / I_green = 12mA / I_blue = 12mA
        I_cyan = 24mA / I_orange = 24mA / I_purple = 24mA
        I_white = 36mA
        P_max (white color) = 180mW 
 
        5V 1A (5W)	: 27 leds
        5V 5A (25W)	: 135 leds
        5V 8A (40W)	: 220 leds
    
    SK6812RGBW: RGB+W colors
        53 FPS for 512 LEDs on ONE SPI bus.
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
void pink_lady_deamon(PINK_LADY_PARAMS *var)
{
    if (!var->is_init_done)
    {
        spi_init(   var->spi_id, 
                    NULL, 
                    IRQ_NONE, 
                    ((var->led_model & SK6812RGBW_INDICE_MASK) > 0) ? SK6812RGBW_TIMING : WS2812B_TIMING, 
                    SPI_CONF_MSTEN | SPI_CONF_FRMPOL_LOW | SPI_CONF_SMP_MIDDLE | SPI_CONF_MODE8 | SPI_CONF_CKP_HIGH | SPI_CONF_CKE_OFF);
        
        dma_init(   var->dma_id, 
                    NULL, 
                    DMA_CONT_PRIO_3 | DMA_CONT_AUTO_ENABLE, 
                    DMA_INT_NONE, 
                    DMA_EVT_START_TRANSFER_ON_IRQ, 
                    spi_get_tx_irq(var->spi_id), 
                    0xff);
        
        var->p_led_model_mapping = (uint32_t *)sk6812rgbw_ws2812b_mapping;
        var->dma_params.dst_start_addr = (void *) spi_get_tx_reg(var->spi_id);       
        dma_set_transfer(var->dma_id, &var->dma_params, true);
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

/*******************************************************************************
 * Function: 
 *      uint8_t pink_lady_set_segment_params(
 *                  PINK_LADY_SEGMENT_PARAMS *p_seg_params, 
 *                  uint16_t from, 
 *                  uint16_t to, 
 *                  RGBW_COLOR color1, 
 *                  RGBW_COLOR color2, 
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
uint8_t pink_lady_set_segment_params(PINK_LADY_SEGMENT_PARAMS *p_seg_params, uint16_t from, uint16_t to, RGBW_COLOR color1, RGBW_COLOR color2, PINK_LADY_RESOLUTIONS resolution, uint32_t deadline_to_appear)
{
    static uint16_t ind;
    
    switch (p_seg_params->sm.index)
    {
        case 0: // Home            
            
            //0. Get delta colors between color1 and color2
            p_seg_params->delta_color.red = (uint8_t) ((color1.red <= color2.red) ? (color2.red - color1.red) : (color1.red - color2.red));
            p_seg_params->delta_color.green = (uint8_t) ((color1.green <= color2.green) ? (color2.green - color1.green) : (color1.green - color2.green));
            p_seg_params->delta_color.blue = (uint8_t) ((color1.blue <= color2.blue) ? (color2.blue - color1.blue) : (color1.blue - color2.blue));
            p_seg_params->delta_color.white = (uint8_t) ((color1.white <= color2.white) ? (color2.white - color1.white) : (color1.white - color2.white));
            
            //1. Set color pointers on appropriate indices type (positive or negative)
            p_seg_params->p_ind_red = (uint16_t *) ((color1.red <= color2.red) ? &p_seg_params->ind_pos : &p_seg_params->ind_neg);
            p_seg_params->p_ind_green = (uint16_t *) ((color1.green <= color2.green) ? &p_seg_params->ind_pos : &p_seg_params->ind_neg);
            p_seg_params->p_ind_blue = (uint16_t *) ((color1.blue <= color2.blue) ? &p_seg_params->ind_pos : &p_seg_params->ind_neg);
            p_seg_params->p_ind_white = (uint16_t *) ((color1.white <= color2.white) ? &p_seg_params->ind_pos : &p_seg_params->ind_neg);            
            
            //2. Get the number of LED to lit
            p_seg_params->number_of_led = (uint16_t) (to - from + 1);
                
            //3. Set the indices
            p_seg_params->ind_pos = from;     
            p_seg_params->ind_neg = to;   
            
            p_seg_params->sm.index = (deadline_to_appear > 0) ? 3 : 1;
            p_seg_params->sm.tick = mGetTick();
            break;
            
        case 1:// Deadline_to_appear == 0 (part 0: initialization)
            
            p_seg_params->p_lowest_value_red = (uint8_t *) ((color1.red <= color2.red) ? &color1.red : &color2.red);
            p_seg_params->p_lowest_value_green = (uint8_t *) ((color1.green <= color2.green) ? &color1.green : &color2.green);
            p_seg_params->p_lowest_value_blue = (uint8_t *) ((color1.blue <= color2.blue) ? &color1.blue : &color2.blue);
            p_seg_params->p_lowest_value_white = (uint8_t *) ((color1.white <= color2.white) ? &color1.white : &color2.white);
            
            p_seg_params->sm.index = 2;
            
        case 2: // Deadline_to_appear == 0 (part 1: led update)
            
            ind = p_seg_params->ind_pos - from;
            
            p_seg_params->p_led[*p_seg_params->p_ind_red].red       = (uint8_t) (((*p_seg_params->p_ind_red % resolution) > 0) ? 0 : (uint32_t)(*p_seg_params->p_lowest_value_red + ind * p_seg_params->delta_color.red / p_seg_params->number_of_led));
            p_seg_params->p_led[*p_seg_params->p_ind_green].green   = (uint8_t) (((*p_seg_params->p_ind_green % resolution) > 0) ? 0 : (uint32_t)(*p_seg_params->p_lowest_value_green + ind * p_seg_params->delta_color.green / p_seg_params->number_of_led));
            p_seg_params->p_led[*p_seg_params->p_ind_blue].blue     = (uint8_t) (((*p_seg_params->p_ind_blue % resolution) > 0) ? 0 : (uint32_t)(*p_seg_params->p_lowest_value_blue + ind * p_seg_params->delta_color.blue / p_seg_params->number_of_led));
            p_seg_params->p_led[*p_seg_params->p_ind_white].white   = (uint8_t) (((*p_seg_params->p_ind_white % resolution) > 0) ? 0 : (uint32_t)(*p_seg_params->p_lowest_value_white + ind * p_seg_params->delta_color.white / p_seg_params->number_of_led));

            --p_seg_params->ind_neg;
            if (++p_seg_params->ind_pos > to)
            {
                p_seg_params->sm.index = 0;
            }
            break;
           
        case 3:// Deadline_to_appear > 0 (part 0: initialization)
            
            //1. Save current LEDs segment
            memcpy(&p_seg_params->p_led_copy[from], &p_seg_params->p_led[from], p_seg_params->number_of_led * sizeof(RGBW_COLOR));            
            //2. Set time for intensity
            p_seg_params->time_between_increment = (uint32_t) (deadline_to_appear / 100);            
            //3. Reset intensity value
            p_seg_params->intensity = 0;            
            p_seg_params->sm.index = 4;
            
        case 4: // Deadline_to_appear > 0 (part 1: led update)
            
            ind = p_seg_params->ind_pos - from;
            
            p_seg_params->p_led[*p_seg_params->p_ind_red].red    = (uint8_t) (((*p_seg_params->p_ind_red % resolution) > 0) ? 0 : (uint32_t)(p_seg_params->p_led_copy[*p_seg_params->p_ind_red].red + ((ind * p_seg_params->delta_color.red / p_seg_params->number_of_led) - p_seg_params->p_led_copy[*p_seg_params->p_ind_red].red) * p_seg_params->intensity / 100));
            p_seg_params->p_led[*p_seg_params->p_ind_green].green  = (uint8_t) (((*p_seg_params->p_ind_green % resolution) > 0) ? 0 : (uint32_t)(p_seg_params->p_led_copy[*p_seg_params->p_ind_green].green + ((ind * p_seg_params->delta_color.green / p_seg_params->number_of_led) - p_seg_params->p_led_copy[*p_seg_params->p_ind_green].green) * p_seg_params->intensity / 100));
            p_seg_params->p_led[*p_seg_params->p_ind_blue].blue   = (uint8_t) (((*p_seg_params->p_ind_blue % resolution) > 0) ? 0 : (uint32_t)(p_seg_params->p_led_copy[*p_seg_params->p_ind_blue].blue + ((ind * p_seg_params->delta_color.blue / p_seg_params->number_of_led) - p_seg_params->p_led_copy[*p_seg_params->p_ind_blue].blue) * p_seg_params->intensity / 100));
            p_seg_params->p_led[*p_seg_params->p_ind_white].white  = (uint8_t) (((*p_seg_params->p_ind_white % resolution) > 0) ? 0 : (uint32_t)(p_seg_params->p_led_copy[*p_seg_params->p_ind_white].white + ((ind * p_seg_params->delta_color.white / p_seg_params->number_of_led) - p_seg_params->p_led_copy[*p_seg_params->p_ind_white].white) * p_seg_params->intensity / 100));
      
            --p_seg_params->ind_neg;
            if (++p_seg_params->ind_pos > to)
            {
                p_seg_params->sm.index = (p_seg_params->intensity == 100) ? 0 : 5;
            }
            break;
            
        case 5: // Deadline_to_appear > 0 (part 2: intensity update)
            
            if (mTickCompare(p_seg_params->sm.tick) > p_seg_params->time_between_increment)
            {
                p_seg_params->sm.tick += p_seg_params->time_between_increment;
                if (++p_seg_params->intensity > 100)
                {
                    p_seg_params->sm.index = 0;
                }
                else
                {
                    p_seg_params->sm.index = 4;
                    p_seg_params->ind_pos = from; 
                    p_seg_params->ind_neg = to;   
                }
            }
            break;
            
        default:
            break;
    }
    
    return p_seg_params->sm.index;
}
