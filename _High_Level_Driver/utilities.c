/*********************************************************************
*	Utilities functions
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               03/09/2013		- Initial release
*               14/10/2013      - Bugs fixed for fUtilitiesLed and fUtilitiesLedRgb functions.
*               18/09/2015      - Remove function "cycle" and add function TiAdvance.
*                               - Implementation of multiple string functions.
*               26/09/2017      - Bug fixe for fUtilitiesLed when tUp or tDown = 0
*********************************************************************/

#include "../PLIB.h"

/*******************************************************************************
 * Function: 
 *      void fu_switch(SWITCH_PARAMS *var)
 * 
 * Description:
 *      This routine is used to manage a switch (debounce & type of push included).
 * 
 * Parameters:
 *      *var: The pointer of SWITCH_PARAMS.
 * 
 * Return:
 *      none
 ******************************************************************************/
void fu_switch(SWITCH_PARAMS *var)
{
    if (!var->is_initialization_done)
    {
        ports_reset_pin_input(var->io);
        var->is_initialization_done = true;
    }
    else
    {
        bool input_state = var->active_state ? ports_get_bit(var->io) : !ports_get_bit(var->io);
        
        if (input_state)
        {
            if (!var->is_debounce_protection_active)
            {
                var->indice++;
                var->is_updated = true;
                var->type_of_push = SIMPLE_PUSH;
                var->is_debounce_protection_active = true;
                var->tick_longpush = mGetTick();
            }
            if (mTickCompare(var->tick_longpush) >= TICK_1S)
            {
                var->type_of_push = LONG_PUSH;
            }
            var->tick_debounce = mGetTick();
        }
        else if (mTickCompare(var->tick_debounce) >= TICK_10MS)
        {
            var->is_debounce_protection_active = false;
        }
    }
}

/*******************************************************************************
 * Function: 
 *      void fu_encoder(ENCODER_PARAMS *var)
 * 
 * Description:
 *      This routine is used to manage an encoder. You can either use pull_up or 
 *      pull_down resistors on inputs (A & B) but you have to indicate which 
 *      active_state you want to use.
 * 
 * Parameters:
 *      *var: The pointer of ENCODER_PARAMS.
 * 
 * Return:
 *      none
 ******************************************************************************/
void fu_encoder(ENCODER_PARAMS *var)
{
    if (!var->is_initialization_done)
    {
        ports_reset_pin_input(var->io[0]);
        ports_reset_pin_input(var->io[1]);
        var->is_initialization_done = true;
    }
    else
    {
        bool input_a = var->active_state ? ports_get_bit(var->io[0]) : !ports_get_bit(var->io[0]);
        bool input_b = var->active_state ? ports_get_bit(var->io[1]) : !ports_get_bit(var->io[1]);

        if((var->a0 != input_a) && (var->b0 == input_b))
        {
            var->add = 1;
        }
        else if((var->a0 == input_a) && (var->b0 != input_b))
        {
            var->add = -1;
        }
        else if((var->a0 != input_a) && (var->b0 != input_b))
        {
            var->a0 = input_a;
            var->b0 = input_b;
            var->indice += var->add;
            var->last_direction = var->add;
        }
        else if((var->a0 == input_a) && (var->b0 == input_b))
        {
            var->add = 0;
        }
    }
}

/*******************************************************************************
 * Function: 
 *      bool fu_turn_indicator(bool enable, uint32_t time_on, uint32_t time_off)
 * 
 * Description:
 *      This routine is used to manage a turn indicator function.
 * 
 * Parameters:
 *      enable: enable or disable the function.
 *      time_on: time during state of ti is ON.
 *      time_off: time during state of ti is OFF.
 * 
 * Return:
 *      The current state of the turn indicator. Should be apply to an I/O pin.
 ******************************************************************************/
bool fu_turn_indicator(bool enable, uint32_t time_on, uint32_t time_off)
{
    static uint64_t tick_ti = 0;
    static bool state = 0;

    if(enable)
    {
        if(state)
        {
            if(mTickCompare(tick_ti) >= time_on)
            {
                tick_ti = mGetTick();
                state = 0;
            }
        }
        else
        {
            if(mTickCompare(tick_ti) >= time_off)
            {
                tick_ti = mGetTick();
                state = 1;
            }
        }
    }
    else
    {
        tick_ti = mGetTick();
        state = 0;
    }

    return state;
}

/*******************************************************************************
 * Function: 
 *      void fu_led(LED_PARAMS *var)
 * 
 * Description:
 *      This routine is used for managing a LED (pwm value, time up, time down...).
 * 
 * Parameters:
 *      *var: A pointer of LED_PARAMS.
 * 
 * Return:
 *      none.
 ******************************************************************************/
void fu_led(LED_PARAMS *var)
{
    if (var->enable)
    {
        if (*var->p_out < var->intensity)
        {
            if (mTickCompare(var->tick) >= var->t_up)
            {
                var->tick = mGetTick();
                (*(var->p_out))++;
            }
        }
        else if (*var->p_out > var->intensity)
        {
            if (mTickCompare(var->tick) >= var->t_down)
            {
                var->tick = mGetTick();
                (*(var->p_out))--;
            }
        }
    }
    else if (mTickCompare(var->tick) >= var->t_down)
    {
        var->tick = mGetTick();
        if (*var->p_out > 0)
        {
            (*(var->p_out))--;
        }
    }
}

/*******************************************************************************
 * Function: 
 *      HSV_COLOR fu_rgb_to_hsv(RGB_COLOR rgb_color)
 * 
 * Description:
 *      This routine is used to convert a RGB model to a HSV model.
 * 
 * Parameters:
 *      rgb_color: A RGB model.
 * 
 * Return:
 *      A HSV model. 
 ******************************************************************************/
HSV_COLOR fu_rgb_to_hsv(RGB_COLOR rgb_color)
{
    float min, max, delta, x;
    HSV_COLOR hsv_color = {0};
    
    min = (rgb_color.red < rgb_color.green) ? rgb_color.red : rgb_color.green;
    min = (min < rgb_color.blue) ? min : rgb_color.blue;
    max = (rgb_color.red > rgb_color.green) ? rgb_color.red : rgb_color.green;
    max = (max > rgb_color.blue) ? max : rgb_color.blue;
    delta = max - min;
    
    if(max > 0.0)
    {
        if(rgb_color.red >= max)
        {
            x = (255.0 * (rgb_color.green - rgb_color.blue) / delta);
        }
        else if(rgb_color.green >= max)
        {
            x = (510.0 + 255.0 * (rgb_color.blue - rgb_color.red) / delta);
        }
        else
        {
            x = (1020.0 + 255.0 * (rgb_color.red - rgb_color.green) / delta);
        }
        
        hsv_color.hue = (uint16_t) ((x >= 0.0) ? x : (x + 1529.0));
        hsv_color.saturation = (uint8_t) (delta * 255.0 / max);
        hsv_color.value = (uint8_t) max;
    }
    
    return hsv_color;
}

/*******************************************************************************
 * Function: 
 *      RGB_COLOR fu_hsv_to_rgb(HSV_COLOR hsv_color)
 * 
 * Description:
 *      This routine is used to convert a HSV model to a RGB model.
 * 
 * Parameters:
 *      hsv_color: A HSV model.
 * 
 * Return:
 *      A RGB model. 
 ******************************************************************************/
RGB_COLOR fu_hsv_to_rgb(HSV_COLOR hsv_color)
{
    uint8_t shade_index;
    RGB_COLOR rgb_color = {0};
    
    shade_index = (hsv_color.hue / 255);
    float f = (float) ((hsv_color.hue / 255.0) - shade_index);
    uint16_t l = hsv_color.value * (255 - hsv_color.saturation) / 255;
    uint16_t m = hsv_color.value * (255 - (float) (f * hsv_color.saturation)) / 255;
    uint16_t n = hsv_color.value * (255 - hsv_color.saturation + (float) (f * hsv_color.saturation)) / 255;
    
    switch (shade_index)
    {
        case 0:
            
            rgb_color.red = hsv_color.value;
            rgb_color.green = n;
            rgb_color.blue = l;
            break;
            
        case 1:
            
            rgb_color.red = m;
            rgb_color.green = hsv_color.value;
            rgb_color.blue = l;
            break;
            
        case 2:
            
            rgb_color.red = l;
            rgb_color.green = hsv_color.value;
            rgb_color.blue = n;
            break;
            
        case 3:
            
            rgb_color.red = l;
            rgb_color.green = m;
            rgb_color.blue = hsv_color.value;
            break;
            
        case 4:
            
            rgb_color.red = n;
            rgb_color.green = l;
            rgb_color.blue = hsv_color.value;
            break;
            
        case 5:
            
            rgb_color.red = hsv_color.value;
            rgb_color.green = l;
            rgb_color.blue = m;
            break;
            
        default:
            
            rgb_color.red = 0;
            rgb_color.green = 0;
            rgb_color.blue = 0;
            break;
            
    }
    
    return rgb_color;
}

/*******************************************************************************
  Function:
    void fUtilitiesSlider(LED_SLIDER_CONFIG *config)

  Description:
    This routine is used for managing a SLIDER.

  Parameters:
    *config     - Pointer containing all parameters of the slider (led ptr, timing, modes...).

  Returns:


  Example:
    <code>

    LED_CONFIG leds[] =
    {
        {OFF, &f_PWM1, 80, TICK_INIT, TICK_2MS, TICK_2MS},
        {OFF, &f_PWM2, 80, TICK_INIT, TICK_2MS, TICK_2MS},
        {OFF, &f_PWM3, 80, TICK_INIT, TICK_2MS, TICK_2MS}
    };
    LED_SLIDER_CONFIG slider = INIT_SLIDER(leds, DIR_LEFT, DIR_LEFT, TICK_200MS, TICK_200MS);

    while(1) {
    ...
    fUtilitiesSlider(&slider);

    fUtilitiesLed(&leds[0]);
    fUtilitiesLed(&leds[1]);
    fUtilitiesLed(&leds[2]);
    ...
    slider.enable = ON;
    }

    </code>
  *****************************************************************************/
//void fUtilitiesSlider(LED_SLIDER_CONFIG *config)
//{
//    BYTE i = 0;
//
//    if(config->enable)
//    {
//        switch(config->modeSlidingOn)
//        {
//            case DIR_RIGHT:
//                if(!config->previousState)
//                {
//                    config->currentIndice = 0;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice < (config->sizeTab - 1))
//                    {
//                        config->currentIndice++;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = ON;
//                break;
//            case DIR_LEFT:
//                if(!config->previousState)
//                {
//                    config->currentIndice = config->sizeTab - 1;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice > 0)
//                    {
//                        config->currentIndice--;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = ON;
//                break;
//            case DIR_EXTERNAL:
//                if(!config->previousState)
//                {
//                    config->currentIndice = (config->sizeTab / 2);
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice > 0)
//                    {
//                        config->currentIndice--;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = ON;
//                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = ON;
//                break;
//            case DIR_CENTER:
//                if(!config->previousState)
//                {
//                    config->currentIndice = 0;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice < config->sizeTab/2)
//                    {
//                        config->currentIndice++;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = ON;
//                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = ON;
//                break;
//            default:
//                break;
//        }
//        config->previousState = 1;
//    }
//    else
//    {
//        switch(config->modeSlidingOff)
//        {
//            case DIR_NONE:
//                while(i < config->sizeTab)
//                {
//                    (config->ptrLed)[i].enable = OFF;
//                    i++;
//                }
//                config->currentIndice = 0xFF;
//                break;
//            case DIR_RIGHT:
//                if(config->previousState)
//                {
//                    config->currentIndice = 0;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice < (config->sizeTab - 1))
//                    {
//                        config->currentIndice++;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = OFF;
//                break;
//            case DIR_LEFT:
//                if(config->previousState)
//                {
//                    config->currentIndice = config->sizeTab - 1;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice > 0)
//                    {
//                        config->currentIndice--;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = OFF;
//                break;
//            case DIR_EXTERNAL:
//                if(config->previousState)
//                {
//                    config->currentIndice = (config->sizeTab / 2);
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice > 0)
//                    {
//                        config->currentIndice--;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = OFF;
//                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = OFF;
//                break;
//            case DIR_CENTER:
//                if(config->previousState)
//                {
//                    config->currentIndice = 0;
//                    config->tickSlider = mGetTick();
//                }
//                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
//                {
//                    config->tickSlider = mGetTick();
//                    if(config->currentIndice < config->sizeTab/2)
//                    {
//                        config->currentIndice++;
//                    }
//                }
//                (config->ptrLed)[config->currentIndice].enable = OFF;
//                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = OFF;
//                break;
//            default:
//                break;
//        }
//        config->previousState = 0;
//    }
//}

/*******************************************************************************
 * Function: 
 *      void fu_adc_average(AVERAGE_PARAMS *var)
 * 
 * Description:
 *      This routine is used to get the average of an ADC acquisition.
 * 
 * Parameters:
 *      *var: The pointer of AVERAGE_PARAMS.
 * 
 * Return:
 *      true: if new acquisition and new average calculated.
 *      false: if no new acquisition.
 * 
 * Example:
 *      See. _EXAMPLE_NTC()
 ******************************************************************************/
bool fu_adc_average(AVERAGE_PARAMS *var)
{    
    if(mTickCompare(var->tick) >= var->period)
    {
        var->tick = mGetTick();
        var->sum_of_buffer -= var->buffer.p[var->index_buffer];
        var->buffer.p[var->index_buffer] = (float) adc10_read(var->adc_module);
        var->sum_of_buffer += var->buffer.p[var->index_buffer];
        var->average = var->sum_of_buffer / var->buffer.size;
        if (++var->index_buffer >= var->buffer.size)
        {
            var->index_buffer = 0;
        }
        return 1;
    }
    return 0;
}

/*******************************************************************************
 * Function: 
 *      bool fu_ntc(NTC_VAR *var)
 * 
 * Description:
 *      This routine is used to get the temperature of a NTC over an ADC.
 *      It uses the AVERAGE function for the acquisitions.
 * 
 * Parameters:
 *      *var: The pointer of NTC_VAR.
 * 
 * Return:
 *      true: if new temperature is calculated.
 *      false: if no new temperature (no new acquisition).
 * 
 * Example:
 *      See. _EXAMPLE_NTC()
 ******************************************************************************/
bool fu_ntc(NTC_VAR *var)
{
    bool ret = fu_adc_average(&var->average);
    if (ret)
    {
        var->temperature = (float) ((1.0/((1.0/(var->ntc_params.t0+273.15)) + ((1.0/var->ntc_params.b) * log(((float)(10000.0/((1024/(var->average.sum_of_buffer/var->average.buffer.size)) - 1.0)))/var->ntc_params.r0 )))) - 273.15); // °C
    }
    return ret;
}

/*******************************************************************************
 * Function: 
 *      void fu_bus_management_task(BUS_MANAGEMENT_VAR *dp)
 * 
 * Description:
 *      This routine is used to manage a multitude of devices on a same serial
 *      bus (SPI, UART, I2C). It gives hand to the device which have the greater 
 *      "last execution time" compare to its periodic time.
 *      This function is mandatory when you are using an external peripheral 
 *      (which use itself a serial BUS).
 * 
 * Parameters:
 *      *var: The pointer of BUS_MANAGEMENT_VAR.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      See. _EXAMPLE_EEPROM() or _EXAMPLE_MCP23S17()
 ******************************************************************************/
void fu_bus_management_task(BUS_MANAGEMENT_VAR *var)
{    
    uint8_t i, j;
    uint64_t greater_time = 0, diff_time = 0;
   
    for(i = 0, j = 255 ; i < var->number_of_params ; i++)
    {
        if(var->params[i]->is_running)
        {
            return;
        }
        else 
        {
            diff_time = mTickCompare(var->params[i]->tick);
            if(diff_time >= var->params[i]->waiting_period)
            {
                if ((diff_time - var->params[i]->waiting_period) >= greater_time)
                {
                    j = i;
                    greater_time = diff_time - var->params[i]->waiting_period;
                }
            }
        }
    }
    if (j != 255)
    {
        var->params[j]->is_running = true;
    }
}

/*******************************************************************************
 * Function: 
 *      uint16_t fu_crc_16_ibm(uint8_t *buffer, uint16_t length)
 * 
 * Description:
 *      This routine is used to calculate a CRC_16_IBM value from a data buffer.
 * 
 * Parameters:
 *      *buffer: The pointer of data buffer.
 *      length: The number of data to consider for CRC calculation.
 * 
 * Return:
 *      The CRC value.
 * 
 * Example:
 *      none
 ******************************************************************************/
uint16_t fu_crc_16_ibm(uint8_t *buffer, uint16_t length)
{
    uint16_t crc = 0;
    uint16_t l;
    
    while (length--)
    {
        crc ^= *buffer++;
        for (l = 0 ; l < 8 ; l++)
        {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xa001 : 0);
        }
    }
    
    return crc;
}

/*******************************************************************************
 * Function: 
 *      uint32_t fu_get_integer_value(float v)
 * 
 * Description:
 *      This routine returns the integer value from a float value.
 * 
 * Parameters:
 *      v: The float value.
 * 
 * Return:
 *      The integer result (0..4294967295).
 * 
 * Example:
 *      none
 ******************************************************************************/
uint32_t fu_get_integer_value(float v)
{
    return (uint32_t) v;
}

/*******************************************************************************
 * Function: 
 *      uint32_t fu_get_decimal_value(float v, uint8_t numbers_after_coma)
 * 
 * Description:
 *      This routine returns the decimal value (2 digits after the decimal point)
 *      from a float value.
 *      BECAREFUL !! THE RESULT IS APPROXIMATE WHEN > 3 DECADES AFTER COMA !!
 * 
 * Parameters:
 *      v: The float value.
 * 
 * Return:
 *      The decimal result (0..4294967295).
 * 
 * Example:
 *      none
 ******************************************************************************/
uint32_t fu_get_decimal_value(float v, uint8_t numbers_after_coma)
{
    uint32_t integer = (uint32_t) v;
    uint32_t mult = pow(10, numbers_after_coma);
    return (uint32_t) ((v - (float) integer) * (float) mult);
}

/*******************************************************************************
 * Function: 
 *      float fu_get_float_value(uint32_t integer, uint8_t decimal)
 * 
 * Description:
 *      This routine returns a float value from integer and decimal values.
 * 
 * Parameters:
 *      integer: The integer value (0..4294967295).
 *      decimal: The decimal value (0..99).
 * 
 * Return:
 *      The float result.
 * 
 * Example:
 *      none
 ******************************************************************************/
float fu_get_float_value(uint32_t integer, uint8_t decimal)
{
    return (float) (integer + decimal/100.0);
}

/*******************************************************************************
 * Function: 
 *      void background_tasks()
 * 
 * Description:
 *      This function must always be placed in the while main loop. It manages 
 *      all the background tasks such as led_status, acquisitions (ntc, current,
 *      volatage, an15) and cpu_load. 
 *      These features are only available for the PICAdapter development board.
 * 
 * Parameters:
 *      none
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void background_tasks(ACQUISITIONS_PARAMS *var)
{
    static uint8_t index_tab_speed = 0;
    static uint64_t tab_speed[20] = {0};
    static uint64_t sum_tab_speed = 0;
    static uint64_t tick_speed = 0;
    static uint8_t mux_sel = 0;
    static uint64_t tick_leds_status = 0;
    
    /*
     * Software reset with switch n°3
     */
    if (!mGetIO(SWITCH3))
    {
        SoftReset();
    } 
    
    /*
     * LED status
     */
    if (mTickCompare(tick_leds_status) >= TICK_200MS) 
    { 
        tick_leds_status = mGetTick(); 
        INV_BIT(_ledStatus, 7);
    }
    mLatIO(LED2) = (((_ledStatus >> 2) & 0x01) & ((~_ledStatus >> 3) & 0x01)) | (((~_ledStatus >> 2) & 0x01) & ((_ledStatus >> 3) & 0x01) & ((~_ledStatus >> 7) & 0x01)) | (((_ledStatus >> 2) & 0x01) & ((_ledStatus >> 3) & 0x01) & ((_ledStatus >> 7) & 0x01));
    mLatIO(LED3) = (((_ledStatus >> 0) & 0x01) & ((~_ledStatus >> 1) & 0x01)) | (((~_ledStatus >> 0) & 0x01) & ((_ledStatus >> 1) & 0x01) & ((~_ledStatus >> 7) & 0x01)) | (((_ledStatus >> 0) & 0x01) & ((_ledStatus >> 1) & 0x01) & ((_ledStatus >> 7) & 0x01));
                                        
    /*
     * PICAdapter acquisitions
     */
    switch(mux_sel)
    {
        case 0:     // NTC
            if (fu_ntc(&var->ntc))
            {
                var->current.tick = mGetTick();
                mux_sel = 1;
            }
            break;
        case 1:     // CURRENT
            if (fu_adc_average(&var->current))
            {
                var->current.average = (float) (var->current.average * 5.04 / 10000.0);    // ((((_ucAcquisitions.tabSumCurrent / 10) * 3.3) / 1023.0) / 6.4);
                var->voltage.tick = mGetTick();
                mux_sel = 2;
            }
            break;
        case 2:     // VOLTAGE
            if (fu_adc_average(&var->voltage))
            {
                var->voltage.average = (float) (var->voltage.average * 18.391 / 1000.0);    // ((((tabSumVoltage / 10) * 3.3) / 1023.0) / 0.1754);
                var->an15.tick = mGetTick();
                var->power_consumption = (float) (var->voltage.average * var->current.average);
                mux_sel = 3;
            }
            break;
        case 3:     // AN15
            if (fu_adc_average(&var->an15))
            {
                var->ntc.average.tick = mGetTick();
                mux_sel = 0;
            }
            break;
    }

    mLatIO(MUX0) = (mux_sel >> 0);
    mLatIO(MUX1) = (mux_sel >> 1);
    
    sum_tab_speed -= tab_speed[index_tab_speed];
    tab_speed[index_tab_speed] = mTickCompare(tick_speed);
    tick_speed = mGetTick();
    sum_tab_speed += tab_speed[index_tab_speed];           
    var->speed = (uint64_t) (sum_tab_speed / 20 / TICK_1US);
    if(++index_tab_speed >= 20)
    {
        index_tab_speed = 0;
    }
}
