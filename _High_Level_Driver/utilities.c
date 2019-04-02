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
 *      void fu_switch(SWITCH_VAR *var)
 * 
 * Description:
 *      This routine is used to manage a switch (debounce & type of push included).
 * 
 * Parameters:
 *      *var: The pointer of SWITCH_VAR.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      See. _EXAMPLE_SWITCH()
 ******************************************************************************/
void fu_switch(SWITCH_VAR *var)
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
 *      void fu_encoder(ENCODER_VAR *var)
 * 
 * Description:
 *      This routine is used to manage an encoder. You can either use pull_up or 
 *      pull_down resistors on inputs (A & B) but you have to indicate which 
 *      active_state you want to use.
 * 
 * Parameters:
 *      *var: The pointer of ENCODER_VAR.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      See. _EXAMPLE_ENCODER()
 ******************************************************************************/
void fu_encoder(ENCODER_VAR *var)
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
  Function:
    void fUtilitiesLedRgb(LED_RGB_CONFIG *config);

  Description:
    This routine is used for managing a RGB LED from a RGB model.

  Parameters:
    *config     - Pointer containing all parameters of the rgb led (pwm value red/green/blue, time up, time down...).

  Returns:


  Example:
    <code>

    LED_RGB_CONFIG led = INIT_LED_RGB(f_PWM1, f_PWM2, f_PWM3, OFF, 100, TICK_20MS, TICK_20MS);
    while(1) {
    ...
    led.enable = ON;
    led.intensity = 70;
    led.red = 100; led.green = 70; led.blue = 10;
    fUtilitiesLedRgb(&led);
    ...
    }

    </code>
  *****************************************************************************/
void fUtilitiesLedRgb(LED_RGB_CONFIG *config)
{
    char red = ((config->rgbParams.red*config->intensity)/100);
    char green = ((config->rgbParams.green*config->intensity)/100);
    char blue = ((config->rgbParams.blue*config->intensity)/100);
    
    if(config->enable)
    {
        if((mGetTick() - config->tick) > config->tUp)
        {
            config->tick = mGetTick();

            if(*(config->pwmRedOut) < red)
            {
                (*(config->pwmRedOut))++;
            }
            else if(*(config->pwmRedOut) > red)
            {
                (*(config->pwmRedOut))--;
            }

            if(*(config->pwmGreenOut) < green)
            {
                (*(config->pwmGreenOut))++;
            }
            else if(*(config->pwmGreenOut) > green)
            {
                (*(config->pwmGreenOut))--;
            }

            if(*(config->pwmBlueOut) < blue)
            {
                (*(config->pwmBlueOut))++;
            }
            else if(*(config->pwmBlueOut) > blue)
            {
                (*(config->pwmBlueOut))--;
            }
        }
    }
    else
    {
        if((mGetTick() - config->tick) > config->tDown)
        {
            config->tick = mGetTick();

            if(*(config->pwmRedOut) > 0)
            {
                (*(config->pwmRedOut))--;
            }
            if(*(config->pwmGreenOut) > 0)
            {
                (*(config->pwmGreenOut))--;
            }
            if(*(config->pwmBlueOut) > 0)
            {
                (*(config->pwmBlueOut))--;
            }
        }
    }
}

/*******************************************************************************
  Function:
    void fUtilitiesLedTsv(LED_TSV_CONFIG *config);

  Description:
    This routine is used for managing a RGB LED from a TSV model.

  Parameters:
    *config     - Pointer containing all parameters of the TSV model (shade, saturation, intensity, time up, time down...).

  Returns:


  Example:
    <code>

    LED_TSV_CONFIG led = INIT_LED_TSV(f_PWM1, f_PWM2, f_PWM3, ON, 100, TICK_20MS, TICK_20MS);
    while(1) {
    ...
    led.enable = ON;
    led.shade = 450; led.saturation = 70; led.intensity = 100;
    fUtilitiesLedTsv(&led);
    ...
    }

    </code>
  *****************************************************************************/
void fUtilitiesLedTsv(LED_TSV_CONFIG *config)
{    
    RGB_COLOR temp = fUtilitiesTSVtoRGB(config->tsvParams);

    if(config->enable)
    {
        if((mGetTick() - config->tick) > config->tUp)
        {
            config->tick = mGetTick();

            if(*(config->pwmRedOut) < temp.red)
            {
                (*(config->pwmRedOut))++;
            }
            else if(*(config->pwmRedOut) > temp.red)
            {
                (*(config->pwmRedOut))--;
            }

            if(*(config->pwmGreenOut) < temp.green)
            {
                (*(config->pwmGreenOut))++;
            }
            else if(*(config->pwmGreenOut) > temp.green)
            {
                (*(config->pwmGreenOut))--;
            }

            if(*(config->pwmBlueOut) < temp.blue)
            {
                (*(config->pwmBlueOut))++;
            }
            else if(*(config->pwmBlueOut) > temp.blue)
            {
                (*(config->pwmBlueOut))--;
            }
        }
    }
    else
    {
        if((mGetTick() - config->tick) > config->tDown)
        {
            config->tick = mGetTick();

            if(*(config->pwmRedOut) > 0)
            {
                (*(config->pwmRedOut))--;
            }
            if(*(config->pwmGreenOut) > 0)
            {
                (*(config->pwmGreenOut))--;
            }
            if(*(config->pwmBlueOut) > 0)
            {
                (*(config->pwmBlueOut))--;
            }
        }
    }
}

/*******************************************************************************
  Function:
    TSV_COLOR fUtilitiesRGBtoTSV(RGB_COLOR rgbColor);

  Description:
    This routine is used for getting the TSV model from the RGB model.

  Parameters:
    The RGB model.

  Returns:
    The TSV model.
  *****************************************************************************/
TSV_COLOR fUtilitiesRGBtoTSV(RGB_COLOR rgbColor)
{
    TSV_COLOR ret;
    float min, max, delta, tShade;
    
    min = (rgbColor.red < rgbColor.green) ? rgbColor.red : rgbColor.green;
    min = (min < rgbColor.blue) ? min : rgbColor.blue;
    max = (rgbColor.red > rgbColor.green) ? rgbColor.red : rgbColor.green;
    max = (max > rgbColor.blue) ? max : rgbColor.blue;
    delta = max - min;
    
    if(max > 0.0)
    {
        if(rgbColor.red >= max)
        {
            tShade = (100.0 * ((rgbColor.green - rgbColor.blue) / delta));
        }
        else if(rgbColor.green >= max)
        {
            tShade = (100.0 * (2.0 + ((rgbColor.blue - rgbColor.red) / delta)));
        }
        else
        {
            tShade = (100.0 * (4.0 + ((rgbColor.red - rgbColor.green) / delta)));
        }
        
        ret.shade = (tShade >= 0.0 ? ((WORD) tShade) : ((WORD) (tShade + 599.0)));
        ret.saturation = (WORD) ((delta / max) * 100.0);
        ret.intensity = (BYTE) max;
    }
    else
    {
        ret.shade = 0;
        ret.saturation = 100;
        ret.intensity = 0;
    }
    
    return ret;
}

/*******************************************************************************
  Function:
    RGB_COLOR fUtilitiesTSVtoRGB(TSV_COLOR tsvColor);

  Description:
    This routine is used for getting the RGB model from the TSV model.

  Parameters:
    The TSV model.

  Returns:
    The RGB model.
  *****************************************************************************/
RGB_COLOR fUtilitiesTSVtoRGB(TSV_COLOR tsvColor)
{
    RGB_COLOR ret;
    BYTE tempShade = ((tsvColor.shade / 100) % 6);
    float f = (float) ((float)(tsvColor.shade / 100.0) - (float)tempShade);
    int l = tsvColor.intensity * (100 - tsvColor.saturation) / 100;
    int m = tsvColor.intensity * (100 - (f * tsvColor.saturation)) / 100;
    int n = tsvColor.intensity * (100 - tsvColor.saturation + (f * tsvColor.saturation)) / 100;

    if((tsvColor.shade > 600) || (tsvColor.saturation > 100) || (tsvColor.intensity > 100))
    {
        ret.red = 0;
        ret.green = 0;
        ret.blue = 0;
    }
    else
    {
        switch (tempShade)
        {
            case 0:
                ret.red = tsvColor.intensity;
                ret.green = n;
                ret.blue = l;
                break;
            case 1:
                ret.red = m;
                ret.green = tsvColor.intensity;
                ret.blue = l;
                break;
            case 2:
                ret.red = l;
                ret.green = tsvColor.intensity;
                ret.blue = n;
                break;
            case 3:
                ret.red = l;
                ret.green = m;
                ret.blue = tsvColor.intensity;
                break;
            case 4:
                ret.red = n;
                ret.green = l;
                ret.blue = tsvColor.intensity;
                break;
            case 5:
                ret.red = tsvColor.intensity;
                ret.green = l;
                ret.blue = m;
                break;
        }
    }
    
    return ret;
}

WORD fUtilitiesGetNumberOfStep(TSV_COLOR color1, TSV_COLOR color2)
{
    WORD ret = 0;
    WORD shade_threshold = abs((int) (color1.shade - color2.shade));
    WORD saturation_threshold = abs((int) (color1.saturation - color2.saturation));
    WORD intensity_threshold = abs((int) (color1.intensity - color2.intensity));
    (shade_threshold > 300)?(shade_threshold = (600-saturation_threshold)):0;
    ret = shade_threshold;
    (ret < saturation_threshold)?(ret = saturation_threshold):0;
    (ret < intensity_threshold)?(ret = intensity_threshold):0;
    return ret;
}

TSV_COLOR fUtilitiesGetMiddleTsvColor(TSV_COLOR color1, TSV_COLOR color2, WORD indice, WORD thresholdToFrom)
{
    TSV_COLOR ret;
    
    if((thresholdToFrom != indice) && ((color1.shade != color2.shade) || (color1.saturation != color2.saturation) || (color1.intensity != color2.intensity)))
    {
        float temp = 0.0;
        WORD shade_threshold = 0;
        BOOL sens = 0;      // --
        
        if(color1.shade > color2.shade)
        {
            shade_threshold = color1.shade - color2.shade;
            if(shade_threshold > 300)
            {
                shade_threshold = 600 - shade_threshold;
                sens = 1;   // ++
            }
        }
        else if(color1.shade < color2.shade)
        {
            shade_threshold = color2.shade - color1.shade;
            if(shade_threshold > 300)
            {
                shade_threshold = 600 - shade_threshold;
            }
            else
            {
                sens = 1;
            }
        }
        else
        {
            ret.shade = color1.shade;
        }

        if(color1.shade != color2.shade)
        {
            if(sens)
            {
                temp = (float) (color1.shade + indice*shade_threshold/thresholdToFrom);    
                if(temp >= 600.0)
                {
                    temp -= 600.0;
                }
            }
            else
            {
                temp = (float) (600.0 + color1.shade - indice*shade_threshold/thresholdToFrom);    
                if(temp >= 600.0)
                {
                    temp -= 600.0;
                }
            }
            ret.shade = (WORD) temp;
        }

        if(color1.saturation >= color2.saturation)
        {
            temp = (float) (color1.saturation - indice*(color1.saturation - color2.saturation)/thresholdToFrom);
        }
        else
        {
            temp = (float) (color1.saturation + indice*(color2.saturation - color1.saturation)/thresholdToFrom);
        }
        ret.saturation = (BYTE) temp;
        
        if(color1.intensity >= color2.intensity)
        {
            temp = (float) (color1.intensity - indice*(color1.intensity - color2.intensity)/thresholdToFrom);
        }
        else
        {
            temp = (float) (color1.intensity + indice*(color2.intensity - color1.intensity)/thresholdToFrom);
        }
        ret.intensity = (BYTE) temp;
    }
    else
    {
        ret = color2;
    }
    
    return ret;
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
void fUtilitiesSlider(LED_SLIDER_CONFIG *config)
{
    BYTE i = 0;

    if(config->enable)
    {
        switch(config->modeSlidingOn)
        {
            case DIR_RIGHT:
                if(!config->previousState)
                {
                    config->currentIndice = 0;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice < (config->sizeTab - 1))
                    {
                        config->currentIndice++;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = ON;
                break;
            case DIR_LEFT:
                if(!config->previousState)
                {
                    config->currentIndice = config->sizeTab - 1;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice > 0)
                    {
                        config->currentIndice--;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = ON;
                break;
            case DIR_EXTERNAL:
                if(!config->previousState)
                {
                    config->currentIndice = (config->sizeTab / 2);
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice > 0)
                    {
                        config->currentIndice--;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = ON;
                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = ON;
                break;
            case DIR_CENTER:
                if(!config->previousState)
                {
                    config->currentIndice = 0;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOn/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice < config->sizeTab/2)
                    {
                        config->currentIndice++;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = ON;
                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = ON;
                break;
            default:
                break;
        }
        config->previousState = 1;
    }
    else
    {
        switch(config->modeSlidingOff)
        {
            case DIR_NONE:
                while(i < config->sizeTab)
                {
                    (config->ptrLed)[i].enable = OFF;
                    i++;
                }
                config->currentIndice = 0xFF;
                break;
            case DIR_RIGHT:
                if(config->previousState)
                {
                    config->currentIndice = 0;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice < (config->sizeTab - 1))
                    {
                        config->currentIndice++;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = OFF;
                break;
            case DIR_LEFT:
                if(config->previousState)
                {
                    config->currentIndice = config->sizeTab - 1;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice > 0)
                    {
                        config->currentIndice--;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = OFF;
                break;
            case DIR_EXTERNAL:
                if(config->previousState)
                {
                    config->currentIndice = (config->sizeTab / 2);
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice > 0)
                    {
                        config->currentIndice--;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = OFF;
                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = OFF;
                break;
            case DIR_CENTER:
                if(config->previousState)
                {
                    config->currentIndice = 0;
                    config->tickSlider = mGetTick();
                }
                if(mTickCompare(config->tickSlider) >= (config->tSliderOff/(config->sizeTab - 1)))
                {
                    config->tickSlider = mGetTick();
                    if(config->currentIndice < config->sizeTab/2)
                    {
                        config->currentIndice++;
                    }
                }
                (config->ptrLed)[config->currentIndice].enable = OFF;
                (config->ptrLed)[config->sizeTab - config->currentIndice - 1].enable = OFF;
                break;
            default:
                break;
        }
        config->previousState = 0;
    }
}

/*******************************************************************************
 * Function: 
 *      void fu_adc_average(AVERAGE_VAR *var)
 * 
 * Description:
 *      This routine is used to get the average of an ADC acquisition.
 * 
 * Parameters:
 *      *var: The pointer of AVERAGE_VAR.
 * 
 * Return:
 *      true: if new acquisition and new average calculated.
 *      false: if no new acquisition.
 * 
 * Example:
 *      See. _EXAMPLE_NTC()
 ******************************************************************************/
bool fu_adc_average(AVERAGE_VAR *var)
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
void background_tasks(ACQUISITIONS_VAR *var)
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
