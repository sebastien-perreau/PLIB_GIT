/*********************************************************************
*	External intelligent LED WS2812B
*	Author : Sébastien PERREAU
*
*   Characteristics:
*   --------------- 
*   DC Voltage = 5V (led in IDLE mode = 400uA)
*   I_red = 12mA / I_green = 12mA / I_blue = 12mA
*   I_cyan = 24mA / I_orange = 24mA / I_purple = 24mA
*   I_white = 36mA
*   P_max (white color) = 180mW 
* 
*   Number of led by power supply:
*   ----------------------------- 
*   5V 1A (5W)	: 27 leds
*   5V 5A (25W)	: 135 leds
*   5V 8A (40W)	: 220 leds
*
*	Revision history	:
*		30/08/2016		- Initial release 1.00
*       10/10/2016      - Release 1.10  
*                           + Only one malloc at the initialization
*                           + Add code error
*                           + Many functions improved
*                           + Add comments 
*       20/09/2018      - Release 1.20
*                           + New DEF & INSTANCE for variable declaration (no more malloc needed)
*                           + Remove unsupported functions (init & add segment) 
*                           Example - New variable in main.c:
*                           WS2812B_DEF(ledsOnSPI3A, SPI3A, csRA1, TOTAL_NUMBER_OF_LEDS, LED_END_SEG1, LED_END_SEG2, LED_END_SEG3, LED_END_SEG4); 
*********************************************************************/

#include "../PLIB.h"

#warning "e_ws2812b.c - SPI Frequency should be equal to 2,4MHz & SPI interruption should be disable"

/*******************************************************************************
  Function:
    BYTE eWS2812BPutSegment(WORD segmentIndice, WORD from, WORD to, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, WS2812B_EFFECTS effectParams, WORD numberOfRepetition, QWORD executionTime, WS2812B_PARAMS *var);

  Description:
    This routine is used to put params into a segment. These params are defines
    in 'Parameters' section below.
    There are two ways for driving LED(s) in a segment and this one is the first. 
    You are able to put simple or gradient colors and simple basics animations such
    as triangle, sawsooth and gaussian.
    The second method is an advance routine used for animations.
 
    For instance (All LEDs attach to the segment 2 will have a SAWTOOTH_INV animation (56 iterations and then a restoration of previous colors) with a gradient and a superposition with the previous colors: 
    eWS2812BPutSegment(2, FIRST_LED, LAST_LED, myCustomColor1, myCustomColor2, WS2812B_EFFECT_SAWTOOTH_INV | WS2812B_SUPERPOSE_EFFECT | WS2812B_RESTORE_COLOR, 56, TICK_1S, &leds);
 
  Return:
    0: if the routine is correctly finish
    1: if the routine fail
    
  Parameters:
    segmentIndice       - The indice of the segment you want to manage.
    from                - The first LED for which you will assign params. This param can be greater than to.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    to                  - The last LED for which you will assign params. This param can be smaller than from.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    step                - The "pitch" between leds:
                        1: 1 by 1
                        2: 1 by 2
                        3: 1 by 3 ...
    tsvParams1          - The first TSV color. If gradient whishes then this param is assign to the from's LED.
    tsvParams2          - The second TSV color. If gradient whishes then this param is assign to the to's LED. If you want a united color than tsvParams2 = tsvParams1.
    effectParams        - The desire effect:
                        1.0. WS2812B_EFFECT_NONE            -> use to put linear or gradient color between 'from' and 'to'.
                        1.1. WS2812B_EFFECT_TRIANGLE        -> simple triangle animation with linear or gradient color between 'from' and 'to'.
                        1.2. WS2812B_EFFECT_SAWTOOTH        -> simple sawtooth animation with linear or gradient color between 'from' and 'to'.
                        1.3. WS2812B_EFFECT_SAWTOOTH_INV    -> simple sawtooth inverse animation with linear or gradient color between 'from' and 'to'.
                        1.4. WS2812B_EFFECT_GAUSSIAN        -> simple gaussian animation with linear or gradient color between 'from' and 'to'.
                        2.0. WS2812B_RESTORE_COLOR          -> will restore the previous TSV color for each LED when the simple animation has been terminated.
                                                            if not used then at the end of the simple animation, each LED (assign to animation) will be turn off.
                        2.1. WS2812B_SUPERPOSE_EFFECT       -> will swap between the previous TSV color for each LED and the tsv color define by this routine.
                                                            if not used then the color(s) define for the simple animation will vary between its params and off.
    numberOfRepetition  - Only used for the simples animations (triangle, sawsooth and gaussian). The values can be 1..127. 
                        If you use WS2812B_REPETITION_INFINITE flag then the animation will never be terminated (except if the user modify the params of the LED(s)).
    executionTime       - This param represent the execution time:
                        1. if EFFECT_NONE then it is the transition time between the old params and the new params.
                        2. if !EFFECT_NONE then this param represent the period of the selected animation.
    *var                - The variable assign to the WS2812B LED array.
  *****************************************************************************/
uint8_t eWS2812BPutSegment(uint16_t segmentIndice, uint16_t from, uint16_t to, WS2812B_STEP_LED step, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, WS2812B_EFFECTS effectParams, uint16_t numberOfRepetition, uint64_t executionTime, WS2812B_PARAMS *var)
{    
    if(segmentIndice <= (var->segments.size-1))
    {
        uint16_t tTo = to;
        (from > to)?(to = from, from = tTo):0;

        if(to == LAST_LED)
        {
            to = (var->segments.p[segmentIndice+1] - var->segments.p[segmentIndice] - 1);
        }

        if(to < (var->segments.p[segmentIndice+1] - var->segments.p[segmentIndice]))
        {
            uint16_t ledIndice;
            for(ledIndice = from ; ledIndice <= to ; ledIndice+=step)
            {
                var->leds[var->segments.p[segmentIndice]+ledIndice].effect.qw = 0;
                var->leds[var->segments.p[segmentIndice]+ledIndice].effect.TYPE_OF_EFFECT = (effectParams >> 0) & 0x07;
                if(!var->leds[var->segments.p[segmentIndice]+ledIndice].effect.TYPE_OF_EFFECT)
                {
                    var->leds[var->segments.p[segmentIndice]+ledIndice].next_params = fUtilitiesGetMiddleTsvColor(tsvParams1, tsvParams2, (ledIndice-from), (to-from));
                    if(executionTime == TICK_0)
                    {
                        var->leds[var->segments.p[segmentIndice]+ledIndice].output_params = var->leds[var->segments.p[segmentIndice]+ledIndice].next_params;
                    }
                    var->leds[var->segments.p[segmentIndice]+ledIndice].effect.CHANGEMENT_EFFECT = (effectParams >> 3) & 0x07;
                }
                else
                {
                    var->leds[var->segments.p[segmentIndice]+ledIndice].save_params = var->leds[var->segments.p[segmentIndice]+ledIndice].output_params;
                    var->leds[var->segments.p[segmentIndice]+ledIndice].next_params = fUtilitiesGetMiddleTsvColor(tsvParams1, tsvParams2, (ledIndice-from), (to-from));
                    var->leds[var->segments.p[segmentIndice]+ledIndice].effect.NUMBER_OF_REPETITION = (numberOfRepetition & 0xff);
                    var->leds[var->segments.p[segmentIndice]+ledIndice].effect.INFINITE_REPETITION = ((numberOfRepetition >> 8) & 0x01);
                    var->leds[var->segments.p[segmentIndice]+ledIndice].effect.RESTORE_PREVIOUS_PARAMS = (effectParams >> 6) & 0x01;
                    var->leds[var->segments.p[segmentIndice]+ledIndice].effect.SUPERPOSED_PREVIOUS_PARAMS = (effectParams >> 7) & 0x01;
                    if(var->leds[var->segments.p[segmentIndice]+ledIndice].effect.SUPERPOSED_PREVIOUS_PARAMS)
                    {
                        var->leds[var->segments.p[segmentIndice]+ledIndice].previous_params = var->leds[var->segments.p[segmentIndice]+ledIndice].save_params;
                    }
                    else
                    {
                        var->leds[var->segments.p[segmentIndice]+ledIndice].previous_params = var->leds[var->segments.p[segmentIndice]+ledIndice].next_params;
                        var->leds[var->segments.p[segmentIndice]+ledIndice].previous_params.intensity = 0;
                    }
                }
                var->leds[var->segments.p[segmentIndice]+ledIndice].time_execution = executionTime;
            }
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}

/*******************************************************************************
  Function:
    BOOL eWS2812BIsSegmentUpdated(WORD segmentIndice, WORD from, WORD to, WS2812B_PARAMS var);

  Description:
    This routine is used to check the status of LED(s) in a segment. We cannot use this routine 
    to check the status of an animation (chenillard or traceur) but only the effect's status define
    by the eWS2812BPutSegment routine.
 
  Return:
    0: the LEDs in the segment are being updated (not finished)
    1: the LEDs in the segment is updated
    
  Parameters:
    segmentIndice       - The indice of the segment you want to manage.
    from                - The first LED for which you will assign params. This param can be greater than to.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    to                  - The last LED for which you will assign params. This param can be smaller than from.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    *var                - The variable assign to the WS2812B LED array.
  *****************************************************************************/
bool eWS2812BIsSegmentUpdated(uint16_t segmentIndice, uint16_t from, uint16_t to, WS2812B_PARAMS var)
{
    if(segmentIndice <= (var.segments.size-1))
    {
        uint16_t tTo = to;
        (from > to)?(to = from, from = tTo):0;

        if(to == LAST_LED)
        {
            to = (var.segments.p[segmentIndice+1] - var.segments.p[segmentIndice] - 1);
        }

        if(to < (var.segments.p[segmentIndice+1] - var.segments.p[segmentIndice]))
        {
            uint16_t ledIndice = 0;
            for(ledIndice = from ; ledIndice <= to ; ledIndice++)
            {
                if(var.leds[var.segments.p[segmentIndice]+ledIndice].effect.TYPE_OF_EFFECT == 0)
                {
                    uint32_t out = (var.leds[var.segments.p[segmentIndice]+ledIndice].output_params.shade << 0) | (var.leds[var.segments.p[segmentIndice]+ledIndice].output_params.saturation << 16) | (var.leds[var.segments.p[segmentIndice]+ledIndice].output_params.intensity << 24);
                    uint32_t next = (var.leds[var.segments.p[segmentIndice]+ledIndice].next_params.shade << 0) | (var.leds[var.segments.p[segmentIndice]+ledIndice].next_params.saturation << 16) | (var.leds[var.segments.p[segmentIndice]+ledIndice].next_params.intensity << 24);
                    if(out != next)
                    {
                        return 0;
                    }
                }
                else
                {
                    if((var.leds[var.segments.p[segmentIndice]+ledIndice].effect.NUMBER_OF_REPETITION > 0) || (var.leds[var.segments.p[segmentIndice]+ledIndice].effect.INFINITE_REPETITION > 0))
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
  Function:
    void eWS2812BSetAnimationParams(BOOL animationType, WORD segmentIndice, WORD from, WORD to, WORD numberOfLed, DWORD timeKeepOn, DWORD timeKeepOff, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, BYTE loopOption, BYTE ledEffectHeader, BYTE ledEffectQueue, WORD numberOfRepetition, QWORD ledTimming, QWORD speedScrolling, WS2812B_ANIMATION *anim, WS2812B_PARAMS var);

  Description:
    This routine is used to put params into the animation variable. It is not directly called because
    it is a generic method. You have to call it by using eWS2812BSetParamsChenillard and
    eWS2812BSetParamsTraceur functions. This is the only way for creating an animation like 
    'chenillard' or 'traceur'. The 'animation variable' is then read/write by the animation deamon
    'eWS2812BAnimation'.

    For instance:
    eWS2812BSetParamsChenillard(0, 19, 0, 4, myCustomColor1, myCustomColor2, WS2812B_ANIM_CHENILLARD_END_LOOP_TFWTF, WS2812B_LED_EFFECT_MIXED, WS2812B_LED_EFFECT_MIXED, 1, -1, TICK_1S*2/3, &anim1, leds);
 
  Return:
    
  Parameters for eWS2812BSetParamsChenillard:
    segmentIndice       - The indice of the segment you want to manage.
    from                - The first LED for which you will assign params. This param can be greater than to.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    to                  - The last LED for which you will assign params. This param can be smaller than from.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    numberOfLed         - The number of LED you want to have in your 'chenillard'.
    tsvParams1          - The first TSV color. If gradient whishes then this param is assign to the from's LED.
    tsvParams2          - The second TSV color. If gradient whishes then this param is assign to the to's LED. If you want a united color than tsvParams2 = tsvParams1.
    loopOption          - If a loop is define then this param describe what to do:
                        WS2812B_ANIM_CHENILLARD_END_LOOP_TFWTF: wait the end of 'chenillard' before re-launch a new cycle (if numberOfRepetition > 0) TO -> FROM -> wait leds extinction -> TO -> FROM ...
                        WS2812B_ANIM_CHENILLARD_END_LOOP_TFTF: continue the 'chenillard' at 'to' when the first LED arriving at 'from' (if numberOfRepetition > 0) TO -> FROM -> TO -> FROM ...
                        WS2812B_ANIM_CHENILLARD_END_LOOP_TFWFT: wait the end of 'chenillard' before re-launch a new cycle in the opposite direction (if numberOfRepetition > 0) TO -> FROM -> wait leds extinction -> FROM -> TO ...
    ledEffectHeader     - The wishes LED effect to apply on the header animation. WS2812B_LED_EFFECT_MIXED, WS2812B_LED_EFFECT_1, WS2812B_LED_EFFECT_2 (cf. e_ws2812b.h to have more details).
    ledEffectQueue      - The wishes LED effect to apply on the queue animation. WS2812B_LED_EFFECT_MIXED, WS2812B_LED_EFFECT_1, WS2812B_LED_EFFECT_2 (cf. e_ws2812b.h to have more details).
    numberOfRepetition  - The values can be 1..127. 
                        If you use WS2812B_REPETITION_INFINITE flag then the animation will never be terminated (except if the user modify the animation params).
    ledTimming          - This param represent the dynamic of each LED (delays turn on/turn off and/or the changement inter-colors).
                        It can be TICK_1MS or any other time value.
                        A special value '-1' can be used to have a tenth part (1/10) of the speed crolling value.
    speedScrolling      - It represents the time for the animation to get 'from' from 'to'. 
    *anim               - The variable assign to your animation.
    var                 - The variable assign to the WS2812B LED array.
 
 * (segInd, from, to, tkOn, tkOff, tsv1, tsv2, loopOption, leh, leq, nor, ledTimming, speedScrolling, anim, var)
  Parameters for eWS2812BSetParamsTraceur:
    segmentIndice       - The indice of the segment you want to manage.
    from                - The first LED for which you will assign params. This param can be greater than to.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    to                  - The last LED for which you will assign params. This param can be smaller than from.
                        You can use the FIRST_LED or LAST_LED defines as you wish.
    timeKeepOn          - Timing param useful for 'loopOption'.
    timeKeepOff         - Timing param useful for 'loopOption'. 
    tsvParams1          - The first TSV color. If gradient whishes then this param is assign to the from's LED.
    tsvParams2          - The second TSV color. If gradient whishes then this param is assign to the to's LED. If you want a united color than tsvParams2 = tsvParams1.
    loopOption          - If a loop is define then this param describe what to do:
                        WS2812B_ANIM_TRACEUR_END_LOOP_ON: set 'traceur' ON according to 'ledEffectHeader' and up to 'to'. Then stay ON regardless of 'numberOfRepetition'.
                        WS2812B_ANIM_TRACEUR_END_LOOP_TFwOffTF: set 'traceur' ON according to 'ledEffectHeader' and up to 'to'. Then all turn OFF during 'timeKeepOff' according to 'ledEffectQueue' and launch again (if numberOfRepetition > 0).
                        WS2812B_ANIM_TRACEUR_END_LOOP_TFwONwOffTF: set 'traceur' ON according to 'ledEffectHeader' and up to 'to'. Then all keep ON during 'timeKeepOn' then turn OFF during 'timeKeepOff' according to 'ledEffectQueue' and launch again (if numberOfRepetition > 0).
    ledEffectHeader     - The wishes LED effect to apply on the header animation. WS2812B_LED_EFFECT_MIXED, WS2812B_LED_EFFECT_1, WS2812B_LED_EFFECT_2 (cf. e_ws2812b.h to have more details).
    ledEffectQueue      - The wishes LED effect to apply on the queue animation. WS2812B_LED_EFFECT_MIXED, WS2812B_LED_EFFECT_1, WS2812B_LED_EFFECT_2 (cf. e_ws2812b.h to have more details).
    numberOfRepetition  - The values can be 1..127. 
                        If you use WS2812B_REPETITION_INFINITE flag then the animation will never be terminated (except if the user modify the animation params).
    ledTimming          - This param represent the dynamic of each LED (delays turn on/turn off and/or the changement inter-colors).
                        It can be TICK_1MS or any other time value.
                        A special value '-1' can be used to have a tenth part (1/10) of the speed crolling value.
    speedScrolling      - It represents the time for the animation to get 'from' from 'to'. 
    *anim               - The variable assign to your animation.
    var                 - The variable assign to the WS2812B LED array.
  *****************************************************************************/
void eWS2812BSetAnimationParams(bool animationType, uint16_t segmentIndice, uint16_t from, uint16_t to, uint16_t numberOfLed, uint32_t timeKeepOn, uint32_t timeKeepOff, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, uint8_t loopOption, uint8_t ledEffectHeader, uint8_t ledEffectQueue, uint16_t numberOfRepetition, uint64_t ledTimming, uint64_t speedScrolling, WS2812B_ANIMATION *anim, WS2812B_PARAMS var)
{    
    anim->typeOfAnimation = animationType;
    if(from < to)
    {
        anim->from = from;
        anim->to = to;
        anim->isInverted = false;
    }
    else
    {
        anim->from = to;
        anim->to = from;
        anim->isInverted = true;
    }
    if(anim->to == LAST_LED)
    {
        anim->to = (var.segments.p[segmentIndice+1] - var.segments.p[segmentIndice] - 1);
    }
    anim->segment_indice = segmentIndice;
    anim->tsv_params1 = tsvParams1;
    anim->tsv_params2 = tsvParams2;
    anim->number_of_repetition = numberOfRepetition;
    anim->number_of_led = numberOfLed;
    anim->number_of_led_on = 0;
    anim->options = ((ledEffectQueue&0x07) << 5) | ((ledEffectHeader&0x07) << 2) | ((loopOption&0x03) << 0);
    anim->indice = 0;
    anim->execution_time_animation = speedScrolling / (anim->to - anim->from + 1);
    if(ledTimming == -1)
    {
        anim->execution_time_led = speedScrolling/10;
    }
    else
    {
        anim->execution_time_led = ledTimming; 
    }
    anim->execution_time_keep_on = timeKeepOn;
    anim->execution_time_keep_off = timeKeepOff;
    anim->tick = 0;
}

/*******************************************************************************
  Function:
    BYTE eWS2812BAnimation(WS2812B_ANIMATION *anim, WS2812B_PARAMS *var);

  Description:
    This routine is the deamon for an animation (pass as parameter).
    It must be called as more as possible in the main routine. It is interresting to 
    check the value return by the routine (more important 0, 2 and 4). See details below.
 
  Return:
    255: animation is in progress (nothing special)
    0: animation fully finished
    1: first step, the animation is in progress (from -> to) and do not reach the other side
    2: end of the first step (first LED reaches the other side)
    3: second step, the animation is still in progress and is in its last step
    4: animation is finish (on its current cycle) but there are others repetitions to come up

  Parameters:
    *anim           - The variable assign to the animation (contains all parameters).
    *var            - The variable assign to the WS2812B LED array.
  *****************************************************************************/
uint8_t eWS2812BAnimation(WS2812B_ANIMATION *anim, WS2812B_PARAMS *var)
{
    BYTE ret = 255;
    if(anim->segment_indice < var->segments.size)
    {
        if(anim->number_of_repetition > 0)
        {                 
            if(mTickCompare(anim->tick) >= anim->execution_time_animation)
            {
                anim->tick = mGetTick();

                WORD currentIndice;

                if(!anim->typeOfAnimation)
                {
                    if(anim->indice <= (anim->to - anim->from))
                    {       
                        if(anim->isInverted)
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->to - anim->indice; 
                        }
                        else
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->from + anim->indice;
                        }

                        var->leds[currentIndice].save_params = var->leds[currentIndice].output_params;
                        var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice, (anim->to - anim->from));
                        var->leds[currentIndice].effect.qw = 0;
                        var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                        var->leds[currentIndice].time_execution = anim->execution_time_led;
                        anim->number_of_led_on++;

                        if(anim->number_of_led_on > anim->number_of_led)
                        {
                            if(anim->isInverted)
                            {
                                currentIndice = var->segments.p[anim->segment_indice] + anim->to - anim->indice + anim->number_of_led;
                            }
                            else
                            {
                                currentIndice = var->segments.p[anim->segment_indice] + anim->from + anim->indice - anim->number_of_led;
                            }       

                            var->leds[currentIndice].next_params = var->leds[currentIndice].save_params;
                            var->leds[currentIndice].effect.qw = 0;
                            var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 5)&0x07;
                            var->leds[currentIndice].time_execution = anim->execution_time_led;
                            anim->number_of_led_on--;
                        } 
                        if(anim->indice == (anim->to - anim->from))
                        {
                            ret = 2;
                        }
                        else
                        {
                            ret = 1;
                        }
                    }
                    else if(anim->indice <= (anim->to - anim->from + anim->number_of_led))
                    {
                        if(anim->isInverted)
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->to - anim->indice + anim->number_of_led; 
                        }
                        else
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->from + anim->indice - anim->number_of_led;
                        }

                        var->leds[currentIndice].next_params = var->leds[currentIndice].save_params;
                        var->leds[currentIndice].effect.qw = 0;
                        var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 5)&0x07;
                        var->leds[currentIndice].time_execution = anim->execution_time_led;
                        anim->number_of_led_on--;

                        if(((anim->options&0x03) == WS2812B_ANIM_CHENILLARD_END_LOOP_TFTF) && (anim->number_of_repetition > 1))
                        {
                            if(anim->isInverted)
                            {
                                currentIndice = var->segments.p[anim->segment_indice] + anim->to - (anim->indice - (anim->to - anim->from + 1));
                            }
                            else
                            {
                                currentIndice = var->segments.p[anim->segment_indice] + anim->from + (anim->indice - (anim->to - anim->from + 1));
                            }

                            var->leds[currentIndice].save_params = var->leds[currentIndice].output_params; 
                            var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice - (anim->to - anim->from), (anim->to - anim->from));
                            var->leds[currentIndice].effect.qw = 0;
                            var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                            var->leds[currentIndice].time_execution = anim->execution_time_led;
                            anim->number_of_led_on++;
                        }
                        ret = 3;
                    }
                    else
                    {
                        if(!GET_BIT(anim->number_of_repetition, 8) && (anim->number_of_repetition > 0))
                        {
                            anim->number_of_repetition--;
                        }

                        if(anim->number_of_repetition > 0)
                        {
                            if((anim->options&0x03) == WS2812B_ANIM_CHENILLARD_END_LOOP_TFWTF)
                            {           
                                anim->indice = 0;

                                if(anim->isInverted)
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->to; 
                                }
                                else
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->from;
                                } 

                                var->leds[currentIndice].save_params = var->leds[currentIndice].output_params;
                                var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice, (anim->to - anim->from));
                                var->leds[currentIndice].effect.qw = 0;
                                var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                                var->leds[currentIndice].time_execution = anim->execution_time_led;
                                anim->number_of_led_on++;
                            }
                            else if((anim->options&0x03) == WS2812B_ANIM_CHENILLARD_END_LOOP_TFTF)
                            {
                                anim->indice = anim->number_of_led;

                                if(anim->isInverted)
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->to - anim->number_of_led; 
                                }
                                else
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->from + anim->number_of_led;
                                } 

                                var->leds[currentIndice].save_params = var->leds[currentIndice].output_params;
                                var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice, (anim->to - anim->from));
                                var->leds[currentIndice].effect.qw = 0;
                                var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                                var->leds[currentIndice].time_execution = anim->execution_time_led;
                                anim->number_of_led_on++;

                                if(anim->number_of_led_on > anim->number_of_led)
                                {        
                                    if(anim->isInverted)
                                    {
                                        currentIndice = var->segments.p[anim->segment_indice] + anim->to; 
                                    }
                                    else
                                    {
                                        currentIndice = var->segments.p[anim->segment_indice] + anim->from;
                                    } 

                                    var->leds[currentIndice].next_params = var->leds[currentIndice].save_params;
                                    var->leds[currentIndice].effect.qw = 0;
                                    var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 5)&0x07;
                                    var->leds[currentIndice].time_execution = anim->execution_time_led;
                                    anim->number_of_led_on--;
                                } 
                            }
                            else if((anim->options&0x03) == WS2812B_ANIM_CHENILLARD_END_LOOP_TFWFT)
                            {
                                anim->indice = 0;
                                anim->isInverted = !anim->isInverted;

                                if(anim->isInverted)
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->to; 
                                }
                                else
                                {
                                    currentIndice = var->segments.p[anim->segment_indice] + anim->from;
                                } 

                                var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice, (anim->to - anim->from));
                                var->leds[currentIndice].effect.qw = 0;
                                var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                                var->leds[currentIndice].time_execution = anim->execution_time_led;
                                anim->number_of_led_on++;
                            }
                            ret = 4;
                        }
                        else
                        {
                            ret = 0;
                            anim->indice--;
                        }
                    }
                }
                else
                {
                    if(anim->indice <= (anim->to - anim->from))
                    {       
                        if(anim->isInverted)
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->to - anim->indice; 
                        }
                        else
                        {
                            currentIndice = var->segments.p[anim->segment_indice] + anim->from + anim->indice;
                        }

                        var->leds[currentIndice].save_params = var->leds[currentIndice].output_params;
                        var->leds[currentIndice].next_params = fUtilitiesGetMiddleTsvColor(anim->tsv_params1, anim->tsv_params2, anim->indice, (anim->to - anim->from));
                        var->leds[currentIndice].effect.qw = 0;
                        var->leds[currentIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 2)&0x07;
                        var->leds[currentIndice].time_execution = anim->execution_time_led;
                        if(anim->indice == (anim->to - anim->from))
                        {
                            ret = 2;
                        }
                        else
                        {
                            ret = 1;
                        }
                    }
                    else if(anim->indice <= (anim->to - anim->from + 1))
                    {
                        if((anim->options&0x03) == 1)
                        {
                            WORD ledIndice;
                            for(ledIndice = anim->from ; ledIndice <= anim->to ; ledIndice++)
                            {                                    
                                var->leds[var->segments.p[anim->segment_indice]+ledIndice].next_params = var->leds[var->segments.p[anim->segment_indice]+ledIndice].save_params;
                                var->leds[var->segments.p[anim->segment_indice]+ledIndice].effect.qw = 0;
                                var->leds[var->segments.p[anim->segment_indice]+ledIndice].effect.CHANGEMENT_EFFECT = (anim->options >> 5)&0x07;
                                var->leds[var->segments.p[anim->segment_indice]+ledIndice].time_execution = anim->execution_time_led;
                            }
                           }
                        else if((anim->options&0x03) == 2)
                        {

                        }
                        anim->tick_keep = mGetTick();
                        ret = 3;
                    }
                    else
                    {
                        ret = 4;
                        if((anim->options&0x03) == 0)
                        {
                            ret = 0;
                            anim->number_of_repetition = 0;
                        }
                        else if((anim->options&0x03) == 1)
                        {
                            if(mTickCompare(anim->tick_keep) >= anim->execution_time_keep_off)
                            {
                                anim->indice = -1;

                                if(!GET_BIT(anim->number_of_repetition, 8) && (anim->number_of_repetition > 0))
                                {
                                    anim->number_of_repetition--;
                                }
                            }
                            else
                            {
                                anim->indice--;
                            }
                        }
                        else if((anim->options&0x03) == 2)
                        {

                        }
                    }
                }
                anim->indice++;
            }
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}

/*******************************************************************************
  Function:
    BYTE eWS2812BFlush(QWORD periodRefresh, WS2812B_PARAMS *var);

  Description:
    This routine is the deamon for the WS2812B LED.
    It must be called as more as possible in the main routine. The data is transmit
    on the SDO pin (using DMA). The CLK, SDI and CS pins are not used.
 
  Return:
    1: data not being send on the SPI bus.
    0: request for data transmission send. The DMA module manage the transmission without
    any external intervention. All the buffer begin to be transmitted.

  Parameters:
    periodRefresh   - The period between each transmission on the SPI bus.
    *var            - The variable assign to the WS2812B LED array.
  *****************************************************************************/
uint8_t eWS2812BFlush(uint64_t periodRefresh, WS2812B_PARAMS *var)
{
    uint8_t ret = 1;
    static uint16_t i[4] = {0};
    
    if (!var->is_chip_select_init)
    {
        SPIInitIOAsChipSelect(var->chip_select);
        var->is_chip_select_init = true;
    }

    switch(var->leds[i[var->spi_module]].effect.TYPE_OF_EFFECT)
    {
        case WS2812B_EFFECT_NONE:
            if(!var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS)
            {
                if(!__WS2812BExecutionTime)
                {
                    var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].next_params;
                }
                else
                {
                    if(var->leds[i[var->spi_module]].effect.CHANGEMENT_EFFECT == 1)
                    {
                        var->leds[i[var->spi_module]].effect.CHANGEMENT_EFFECT = 0;
                        var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].next_params;
                        var->leds[i[var->spi_module]].output_params.intensity = 0;
                    }
                    else if(var->leds[i[var->spi_module]].effect.CHANGEMENT_EFFECT == 2)
                    {
                        var->leds[i[var->spi_module]].save_params = var->leds[i[var->spi_module]].next_params;
                        var->leds[i[var->spi_module]].next_params = var->leds[i[var->spi_module]].output_params;
                        var->leds[i[var->spi_module]].next_params.intensity = 0;
                    }
                    
                    var->leds[i[var->spi_module]].effect.MAX_INDICE = fUtilitiesGetNumberOfStep(var->leds[i[var->spi_module]].output_params, var->leds[i[var->spi_module]].next_params);
                    if(var->leds[i[var->spi_module]].effect.MAX_INDICE > 0)
                    {
                        var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 1;
                        var->leds[i[var->spi_module]].previous_params = var->leds[i[var->spi_module]].output_params;
                        var->leds[i[var->spi_module]].effect.COUNTER = 0;
                        __WS2812BExecutionTime2 = __WS2812BExecutionTime / var->leds[i[var->spi_module]].effect.MAX_INDICE;
                        __WS2812BTick = mGetTick();
                        __WS2812BTickSynchro = 0;
                    }
                }
            }
            else
            {
                if((mTickCompare(__WS2812BTick) + __WS2812BTickSynchro) >= __WS2812BExecutionTime2)
                {
                    QWORD get_tick = mGetTick();
                    WORD inc = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) / __WS2812BExecutionTime2;
                    __WS2812BTickSynchro = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) % __WS2812BExecutionTime2;
                    __WS2812BTick = get_tick;
                    WORD cpt = (var->leds[i[var->spi_module]].effect.COUNTER + inc);

                    if(cpt <= var->leds[i[var->spi_module]].effect.MAX_INDICE)
                    {
                        var->leds[i[var->spi_module]].effect.COUNTER = cpt;
                        var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].previous_params, var->leds[i[var->spi_module]].next_params, cpt, var->leds[i[var->spi_module]].effect.MAX_INDICE);
                    }
                    else
                    {
                        var->leds[i[var->spi_module]].effect.COUNTER = 0;
                        var->leds[i[var->spi_module]].effect.MAX_INDICE = 0;
                        var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].next_params;
                        if(var->leds[i[var->spi_module]].effect.CHANGEMENT_EFFECT == 2)
                        {
                            var->leds[i[var->spi_module]].effect.CHANGEMENT_EFFECT = 0;
                            var->leds[i[var->spi_module]].next_params = var->leds[i[var->spi_module]].save_params;
                            var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].next_params;
                        }
                        var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0;
                    }
                }
            }
            break;
        case WS2812B_EFFECT_TRIANGLE:
            if(((var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) || (var->leds[i[var->spi_module]].effect.INFINITE_REPETITION > 0)) > 0)
            {
                if(!var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS)
                {
                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 1;
                    var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                    var->leds[i[var->spi_module]].effect.COUNTER = 0;
                    var->leds[i[var->spi_module]].effect.MAX_INDICE = 200;
                    __WS2812BExecutionTime2 = __WS2812BExecutionTime/var->leds[i[var->spi_module]].effect.MAX_INDICE;
                    __WS2812BTick = mGetTick();
                    __WS2812BTickSynchro = 0;
                }
                else
                {
                    if((mTickCompare(__WS2812BTick) + __WS2812BTickSynchro) >= __WS2812BExecutionTime2)
                    {
                        QWORD get_tick = mGetTick();
                        WORD inc = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) / __WS2812BExecutionTime2;
                        __WS2812BTickSynchro = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) % __WS2812BExecutionTime2;
                        __WS2812BTick = get_tick;
                        WORD cpt = (var->leds[i[var->spi_module]].effect.COUNTER + inc);
                        
                        if(cpt <= var->leds[i[var->spi_module]].effect.MAX_INDICE)
                        {
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt;
                            if(cpt <= (var->leds[i[var->spi_module]].effect.MAX_INDICE / 2))
                            {
                                var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].previous_params, var->leds[i[var->spi_module]].next_params, cpt, var->leds[i[var->spi_module]].effect.MAX_INDICE/2);
                            }
                            else
                            {
                                var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].previous_params, var->leds[i[var->spi_module]].next_params, var->leds[i[var->spi_module]].effect.MAX_INDICE - cpt, var->leds[i[var->spi_module]].effect.MAX_INDICE/2);
                            }
                        }
                        else
                        {
                            var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt - var->leds[i[var->spi_module]].effect.MAX_INDICE;
                            if(!var->leds[i[var->spi_module]].effect.INFINITE_REPETITION) 
                            { 
                                if(var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) 
                                { 
                                    var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION--; 
                                } 
                                else
                                {
                                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0;
                                }
                            }
                        }
                    }
                }
            }
            else if(var->leds[i[var->spi_module]].effect.RESTORE_PREVIOUS_PARAMS)
            {
                var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].save_params;
            }
            break;
        case WS2812B_EFFECT_SAWTOOTH:
            if(((var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) || (var->leds[i[var->spi_module]].effect.INFINITE_REPETITION > 0)) > 0)
            {
                if(!var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS)
                {
                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 1;
                    var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                    var->leds[i[var->spi_module]].effect.COUNTER = 0;
                    var->leds[i[var->spi_module]].effect.MAX_INDICE = 100;
                    __WS2812BExecutionTime2 = __WS2812BExecutionTime/var->leds[i[var->spi_module]].effect.MAX_INDICE;
                    __WS2812BTick = mGetTick();
                    __WS2812BTickSynchro = 0;
                }
                else
                {
                    if((mTickCompare(__WS2812BTick) + __WS2812BTickSynchro) >= __WS2812BExecutionTime2)
                    {
                        QWORD get_tick = mGetTick();
                        WORD inc = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) / __WS2812BExecutionTime2;
                        __WS2812BTickSynchro = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) % __WS2812BExecutionTime2;
                        __WS2812BTick = get_tick;
                        WORD cpt = (var->leds[i[var->spi_module]].effect.COUNTER + inc);

                        if(cpt <= var->leds[i[var->spi_module]].effect.MAX_INDICE)
                        {
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt;
                            var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].previous_params, var->leds[i[var->spi_module]].next_params, cpt, var->leds[i[var->spi_module]].effect.MAX_INDICE);
                        }
                        else
                        {
                            var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt - var->leds[i[var->spi_module]].effect.MAX_INDICE;
                            if(!var->leds[i[var->spi_module]].effect.INFINITE_REPETITION) 
                            { 
                                if(var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) 
                                { 
                                    var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION--; 
                                } 
                                else
                                {
                                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0;
                                }
                            }
                        }
                    }
                }
            }
            else if(var->leds[i[var->spi_module]].effect.RESTORE_PREVIOUS_PARAMS)
            {
                var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].save_params;
            }
            break;
        case WS2812B_EFFECT_SAWTOOTH_INV:
            if(((var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) || (var->leds[i[var->spi_module]].effect.INFINITE_REPETITION > 0)) > 0)
            {
                if(!var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS)
                {
                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 1;
                    var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].next_params;
                    var->leds[i[var->spi_module]].effect.COUNTER = 0;
                    var->leds[i[var->spi_module]].effect.MAX_INDICE = 100;
                    __WS2812BExecutionTime2 = __WS2812BExecutionTime/var->leds[i[var->spi_module]].effect.MAX_INDICE;
                    __WS2812BTick = mGetTick();
                    __WS2812BTickSynchro = 0;
                }
                else
                {
                    if((mTickCompare(__WS2812BTick) + __WS2812BTickSynchro) >= __WS2812BExecutionTime2)
                    {
                        QWORD get_tick = mGetTick();
                        WORD inc = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) / __WS2812BExecutionTime2;
                        __WS2812BTickSynchro = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) % __WS2812BExecutionTime2;
                        __WS2812BTick = get_tick;
                        WORD cpt = (var->leds[i[var->spi_module]].effect.COUNTER + inc);

                        if(cpt <= var->leds[i[var->spi_module]].effect.MAX_INDICE)
                        {
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt;
                            var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].next_params, var->leds[i[var->spi_module]].previous_params, cpt, var->leds[i[var->spi_module]].effect.MAX_INDICE);
                        }
                        else
                        {
                            var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt - var->leds[i[var->spi_module]].effect.MAX_INDICE;
                            if(!var->leds[i[var->spi_module]].effect.INFINITE_REPETITION) 
                            { 
                                if(var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) 
                                { 
                                    var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION--; 
                                } 
                                else
                                {
                                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0;
                                }
                            }
                        }
                    }
                }
            }
            else if(var->leds[i[var->spi_module]].effect.RESTORE_PREVIOUS_PARAMS)
            {
                var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].save_params;
            }
            break;
        case WS2812B_EFFECT_GAUSSIAN:
            if(((var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) || (var->leds[i[var->spi_module]].effect.INFINITE_REPETITION > 0)) > 0)
            {
                if(!var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS)
                {
                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 1;
                    var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                    var->leds[i[var->spi_module]].effect.COUNTER = 0;
                    var->leds[i[var->spi_module]].effect.MAX_INDICE = 200;
                    __WS2812BExecutionTime2 = __WS2812BExecutionTime/var->leds[i[var->spi_module]].effect.MAX_INDICE;
                    __WS2812BTick = mGetTick();
                    __WS2812BTickSynchro = 0;
                }
                else
                {
                    if((mTickCompare(__WS2812BTick) + __WS2812BTickSynchro) >= __WS2812BExecutionTime2)
                    {                        
                        QWORD get_tick = mGetTick();
                        WORD inc = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) / __WS2812BExecutionTime2;
                        __WS2812BTickSynchro = ((get_tick - __WS2812BTick) + __WS2812BTickSynchro) % __WS2812BExecutionTime2;
                        __WS2812BTick = get_tick;
                        WORD cpt = (var->leds[i[var->spi_module]].effect.COUNTER + inc);
                        double x = (double) (cpt*6.0/var->leds[i[var->spi_module]].effect.MAX_INDICE);
                        double retValue = (double) ((var->leds[i[var->spi_module]].effect.MAX_INDICE*251.0/100.0)/pow(2.0*M_PI, 0.5))*exp(-0.5*pow((double) (x-3.0), 2.0));
                        
                        
                        if(cpt <= var->leds[i[var->spi_module]].effect.MAX_INDICE)
                        {
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt;
                            var->leds[i[var->spi_module]].output_params = fUtilitiesGetMiddleTsvColor(var->leds[i[var->spi_module]].previous_params, var->leds[i[var->spi_module]].next_params, retValue, var->leds[i[var->spi_module]].effect.MAX_INDICE);
                        }
                        else
                        {
                            var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].previous_params;
                            var->leds[i[var->spi_module]].effect.COUNTER = cpt - var->leds[i[var->spi_module]].effect.MAX_INDICE;
                            if(!var->leds[i[var->spi_module]].effect.INFINITE_REPETITION) 
                            { 
                                if(var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) 
                                { 
                                    var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION--; 
                                } 
                                else
                                {
                                    var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0;
                                }
                            }
                        }
                    }
                }
            }
            else if(var->leds[i[var->spi_module]].effect.RESTORE_PREVIOUS_PARAMS)
            {
                var->leds[i[var->spi_module]].output_params = var->leds[i[var->spi_module]].save_params;
            }
            break;
    }

    RGB_COLOR rgbColor = fUtilitiesTSVtoRGB(var->leds[i[var->spi_module]].output_params);

    rgbColor.red *= 2.55;
    rgbColor.green *= 2.55;
    rgbColor.blue *= 2.55;
    // GREEN
    var->buffer.p[0+i[var->spi_module]*9] = 0x92 | ((rgbColor.green & 0x80) >> 1) | ((rgbColor.green & 0x40) >> 3) | ((rgbColor.green & 0x20) >> 5);
    var->buffer.p[1+i[var->spi_module]*9] = 0x49 | ((rgbColor.green & 0x10) << 1) | ((rgbColor.green & 0x08) >> 1);
    var->buffer.p[2+i[var->spi_module]*9] = 0x24 | ((rgbColor.green & 0x04) << 5) | ((rgbColor.green & 0x02) << 3) | ((rgbColor.green & 0x01) << 1);                 
    // RED
    var->buffer.p[3+i[var->spi_module]*9] = 0x92 | ((rgbColor.red & 0x80) >> 1) | ((rgbColor.red & 0x40) >> 3) | ((rgbColor.red & 0x20) >> 5);
    var->buffer.p[4+i[var->spi_module]*9] = 0x49 | ((rgbColor.red & 0x10) << 1) | ((rgbColor.red & 0x08) >> 1);
    var->buffer.p[5+i[var->spi_module]*9] = 0x24 | ((rgbColor.red & 0x04) << 5) | ((rgbColor.red & 0x02) << 3) | ((rgbColor.red & 0x01) << 1);
    // BLUE
    var->buffer.p[6+i[var->spi_module]*9] = 0x92 | ((rgbColor.blue & 0x80) >> 1) | ((rgbColor.blue & 0x40) >> 3) | ((rgbColor.blue & 0x20) >> 5);
    var->buffer.p[7+i[var->spi_module]*9] = 0x49 | ((rgbColor.blue & 0x10) << 1) | ((rgbColor.blue & 0x08) >> 1);
    var->buffer.p[8+i[var->spi_module]*9] = 0x24 | ((rgbColor.blue & 0x04) << 5) | ((rgbColor.blue & 0x02) << 3) | ((rgbColor.blue & 0x01) << 1);

    if(++i[var->spi_module] >= var->segments.p[var->segments.size-1])
    {
        i[var->spi_module] = 0;
    }
            
    if(mTickCompare(var->tickRefresh) >= periodRefresh)
    { 
        if(!SPIWriteAndStoreByteArray(var->spi_module, var->chip_select, (void*)var->buffer.p, NULL, var->segments.p[var->segments.size-1]*9))
        {
            var->tickRefresh = mGetTick();
            ret = 0;
        }
    }
    return ret;
}
