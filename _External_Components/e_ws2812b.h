#ifndef __DEF_WS2812B
#define	__DEF_WS2812B

#define FIRST_LED   0
#define LAST_LED    65535

#define COLOR_RED       (TSV_COLOR) {0, 100, 100}
#define COLOR_ORANGE    (TSV_COLOR) {50, 100, 100}
#define COLOR_GREEN     (TSV_COLOR) {200, 100, 100}
#define COLOR_CYAN      (TSV_COLOR) {300, 100, 100}
#define COLOR_BLUE      (TSV_COLOR) {400, 100, 100}
#define COLOR_PURPLE    (TSV_COLOR) {500, 100, 100}
#define COLOR_WHITE     (TSV_COLOR) {0, 0, 100}
#define COLOR_OFF       (TSV_COLOR) {0, 0, 0}

typedef enum 
{
    LED_ALL     = 1,
    LED_1_2     = 2,
    LED_1_3     = 3,
    LED_1_4     = 4,
    LED_1_5     = 5,
    LED_1_6     = 6,
    LED_1_7     = 7,
    LED_1_8     = 8,
    LED_1_9     = 9,
    LED_1_255   = 255
} WS2812B_STEP_LED;

//                MIXED     EFFECT_1    EFFECT_2
//               ___        ___         ___
//  TSV actuel      \___       |____       \___
//                   ___         ___         ___
//  TSV nouveau  ___/       ____/       ____|

typedef enum
{
    WS2812B_EFFECT_NONE             = (0 << 0),
    WS2812B_EFFECT_TRIANGLE         = (1 << 0),
    WS2812B_EFFECT_SAWTOOTH         = (2 << 0),
    WS2812B_EFFECT_SAWTOOTH_INV     = (3 << 0),
    WS2812B_EFFECT_GAUSSIAN         = (4 << 0),
            
    WS2812B_LED_EFFECT_MIXED        = (0 << 3),     // NONE: only when no effects is apply
    WS2812B_LED_EFFECT_1            = (1 << 3),     // NONE: only when no effects is apply
    WS2812B_LED_EFFECT_2            = (2 << 3),     // NONE: only when no effects is apply
            
    WS2812B_RESTORE_COLOR           = (1 << 6),     // OTHERS: restore the initial color (the color before the effect)
    WS2812B_SUPERPOSE_EFFECT        = (1 << 7),     // OTHERS: superpose the new color with the initial color during the effect
            
    WS2812B_REPETITION_INFINITE     = (1 << 8)
}WS2812B_EFFECTS;

typedef enum
{
    WS2812B_ANIM_CHENILLARD_END_LOOP_TFWTF = 0,     // attente fin du chenillard avant un nouveau cycle (si numberOfRepetition > 0)         TO -> FROM -> attente extinction des leds -> TO -> FROM ...
    WS2812B_ANIM_CHENILLARD_END_LOOP_TFTF = 1,      // continu le chenillard à 'to' lorsqu'il arrive à 'from' (si numberOfRepetition > 0)   TO -> FROM -> TO -> FROM ...
    WS2812B_ANIM_CHENILLARD_END_LOOP_TFWFT = 2,     // continu le chenillard en sens inverse (si numberOfRepetition > 0)                    TO -> FROM -> attente extinction des leds -> FROM -> TO -> attente... 
    WS2812B_ANIM_TRACEUR_END_LOOP_ON = 0,           // allumage traceur selon 'ledEffectHeader' jusqu'à 'to' puis reste allumé (quelque soit nomberOfRepetition)
    WS2812B_ANIM_TRACEUR_END_LOOP_TFwOffTF = 1,     // allumage traceur selon 'ledEffectHeader' jusqu'à 'to' puis tout s'éteind durant un temps 'timeKeepOff' selon 'ledEffectQueue' puis on recommence...
    WS2812B_ANIM_TRACEUR_END_LOOP_TFwONwOffTF = 2   // allumage traceur selon 'ledEffectHeader' jusqu'à 'to' puis tout reste allumé durant 'timeKeepOn' puis tout s'éteind durant un temps 'timeKeepOff' selon 'ledEffectQueue' puis on recommence...
}WS2812B_ANIM_END_LOOP;

typedef union
{
    struct
    {
        unsigned CHANGEMENT_EFFECT: 3;          // NONE: effet sur le changement de couleur (Mixed, effet_1, effet_2...)
        
        unsigned NUMBER_OF_REPETITION:8;        // OTHERS
        unsigned INFINITE_REPETITION:1;         // OTHERS
        unsigned RESTORE_PREVIOUS_PARAMS:1;     // OTHERS
        unsigned SUPERPOSED_PREVIOUS_PARAMS:1;  // OTHERS

        unsigned COUNTER:9;                     // BOTH: compteur 9 bits
        unsigned MAX_INDICE:9;                  // BOTH: indice max pour le changement de couleur
        unsigned TYPE_OF_EFFECT:3;              // BOTH: cf. WS2812B_EFFECTS
        unsigned EFFECT_IN_PROGRESS:1;          // BOTH
    };
    
    struct 
    {
        QWORD qw;
    };
}__EFFECTbits;

typedef struct
{
    BOOL        typeOfAnimation;            // 0: chenillard, 1: traceur
    BYTE        segment_indice;
    WORD        from;
    WORD        to;
    WORD        number_of_led;              // uniquement utilisé pour le chenillard
    WORD        number_of_led_on;           // uniquement utilisé pour le chenillard
    TSV_COLOR   tsv_params1;
    TSV_COLOR   tsv_params2;
    WORD        number_of_repetition;
    BYTE        options;
    BOOL        isInverted;
    WORD        indice;
    DWORD       execution_time_led;
    DWORD       execution_time_animation;
    DWORD       execution_time_keep_on;     // uniquement utilisé pour le traceur
    DWORD       execution_time_keep_off;    // uniquement utilisé pour le traceur
    QWORD       tick;
    QWORD       tick_keep;                  // uniquement utilisé pour le traceur
}WS2812B_ANIMATION;

typedef struct
{
    TSV_COLOR       output_params;
    TSV_COLOR       previous_params;
    TSV_COLOR       next_params;
    TSV_COLOR       save_params;            
    __EFFECTbits    effect;
    DWORD           time_execution;
    DWORD           time_execution2;
    DWORD           tick_synchro; 
    QWORD           tick;
}WS2812B_LED;

typedef struct
{
    uint8_t             spi_module;
    _IO                 chip_select;
    bool                is_chip_select_init;
    DYNAMIC_TAB_WORD    segments;
    WS2812B_LED         *leds;
    DYNAMIC_TAB_BYTE    buffer;
    uint64_t            tickRefresh;
}WS2812B_PARAMS;

#define WS2812B_INSTANCE(_spi_module, _io_port, _io_indice, c, d, e, f)          \
{                                                   \
	.spi_module = _spi_module,                      \
	.chip_select = { _io_port, _io_indice },        \
    .is_chip_select_init = false,                   \
	.segments = {c, sizeof(c)/sizeof(uint16_t), 0},	\
	.leds = d,                                      \
	.buffer = {e, f, 0},                            \
	.tickRefresh = 0,                               \
}

#define WS2812B_DEF(_name, _spi_module, _cs_pin, _number_total_of_leds, ...)                                \
static uint16_t _name ## _segment_ram_allocation[] = {0, __VA_ARGS__ };                                     \
static uint8_t _name ## _buffer_ram_allocation[_number_total_of_leds*9] = {0};                              \
static WS2812B_LED _name ## _led_ram_allocation[_number_total_of_leds] = {0};                               \
static WS2812B_PARAMS _name = WS2812B_INSTANCE(_spi_module, _XBR(_cs_pin), _IND(_cs_pin), _name ## _segment_ram_allocation, _name ## _led_ram_allocation, _name ## _buffer_ram_allocation, _number_total_of_leds)	

#define __WS2812BIntensity                  var->leds[i[var->spi_module]].output_params.intensity
#define __WS2812BTick                       var->leds[i[var->spi_module]].tick
#define __WS2812BTickSynchro                var->leds[i[var->spi_module]].tick_synchro
#define __WS2812BExecutionTime              var->leds[i[var->spi_module]].time_execution
#define __WS2812BExecutionTime2             var->leds[i[var->spi_module]].time_execution2
#define EndOfEffect()                       __WS2812BIntensity = 0; var->leds[i[var->spi_module]].effect.EFFECT_IN_PROGRESS = 0; if(!var->leds[i[var->spi_module]].effect.INFINITE_REPETITION) { if(var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION > 0) { var->leds[i[var->spi_module]].effect.NUMBER_OF_REPETITION--; } else { var->leds[i[var->spi_module]].effect.COUNTER = 0; } }

#define eWS2812BIsLedUpdated(segmentIndice, ledIndice, var)         eWS2812BIsSegmentUpdated(segmentIndice, ledIndice, ledIndice, var)
#define eWS2812BIsAnimationFinished(anim)                           ((anim.number_of_repetition > 0) ? false : true)
#define eWS2812BGetNumberOfTotalLed(var)                            (var.buffer.size/9)
#define eWS2812BGetNumberOfLedInSegment(segmentIndice, var)         (var.segments.p[segmentIndice+1] - var.segments.p[segmentIndice])



uint8_t eWS2812BPutSegment(uint16_t segmentIndice, uint16_t from, uint16_t to, WS2812B_STEP_LED step, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, WS2812B_EFFECTS effectParams, uint16_t numberOfRepetition, uint64_t executionTime, WS2812B_PARAMS *var);
bool eWS2812BIsSegmentUpdated(uint16_t segmentIndice, uint16_t from, uint16_t to, WS2812B_PARAMS var);
void eWS2812BSetAnimationParams(bool animationType, uint16_t segmentIndice, uint16_t from, uint16_t to, uint16_t numberOfLed, uint32_t timeKeepOn, uint32_t timeKeepOff, TSV_COLOR tsvParams1, TSV_COLOR tsvParams2, uint8_t loopOption, uint8_t ledEffectHeader, uint8_t ledEffectQueue, uint16_t numberOfRepetition, uint64_t ledTimming, uint64_t speedScrolling, WS2812B_ANIMATION *anim, WS2812B_PARAMS var);
uint8_t eWS2812BAnimation(WS2812B_ANIMATION *anim, WS2812B_PARAMS *var);
uint8_t eWS2812BFlush(uint64_t periodRefresh, WS2812B_PARAMS *var);



#define ws2812b_put_color(_var, _segment, _step, _color)                                                            eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color, _color, 0, 0, 0, &_var)
#define ws2812b_put_color_delay(_var, _segment, _step, _color, _delay)                                              eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color, _color, 0, 0, _delay, &_var)
#define ws2812b_put_color_from_to(_var, _segment, _step, _color, _from, _to)                                        eWS2812BPutSegment(_segment, _from, _to, _step, _color, _color, 0, 0, 0, &_var)
#define ws2812b_put_color_from_to_delay(_var, _segment, _step, _color, _from, _to, _delay)                          eWS2812BPutSegment(_segment, _from, _to, _step, _color, _color, 0, 0, _delay, &_var)

#define ws2812b_put_color_effect(_var, _segment, _step, _color, _effect, _repetition, _speed)                       eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color, _color, _effect, _repetition, _speed, &_var)
#define ws2812b_put_color_effect_from_to(_var, _segment, _step, _color, _from, _to, _effect, _repetition, _speed)   eWS2812BPutSegment(_segment, _from, _to, _step, _color, _color, _effect, _repetition, _speed, &_var)

#define ws2812b_put_gradient(_var, _segment, _step, _color1, _color2)                                               eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color1, _color2, 0, 0, 0, &_var)
#define ws2812b_put_gradient_delay(_var, _segment, _step, _color1, _color2, _delay)                                 eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color1, _color2, 0, 0, _delay, &_var)
#define ws2812b_put_gradient_from_to(_var, _segment, _step, _color1, _color2, _from, _to)                           eWS2812BPutSegment(_segment, _from, _to, _step, _color1, _color2, 0, 0, 0, &_var)
#define ws2812b_put_gradient_from_to_delay(_var, _segment, _step, _color1, _color2, _from, _to, _delay)             eWS2812BPutSegment(_segment, _from, _to, _step, _color1, _color2, 0, 0, _delay, &_var)

#define ws2812b_put_gradient_effect(_var, _segment, _step, _color1, _color2, _effect, _repetition, _speed)                      eWS2812BPutSegment(_segment, FIRST_LED, LAST_LED, _step, _color1, _color2, _effect, _repetition, _speed, &_var)
#define ws2812b_put_gradient_effect_from_to(_var, _segment, _step, _color1, _color2, _from, _to, _effect, _repetition, _speed)  eWS2812BPutSegment(_segment, _from, _to, _step, _color1, _color2, _effect, _repetition, _speed, &_var)

#define ws2812b_is_segment_updated(_var, _segment)                                                                  eWS2812BIsSegmentUpdated(_segment, FIRST_LED, LAST_LED, _var)
#define ws2812b_is_segment_updated_from_to(_var, _segment, _from, _to)                                              eWS2812BIsSegmentUpdated(_segment, _from, _to, _var)



// Next functions are issues !!!
#define eWS2812BSetParamsChenillard(segInd, from, to, nol, tsv1, tsv2, loopOption, leh, leq, nor, ledTimming, speedScrolling, anim, var)             eWS2812BSetAnimationParams(0, segInd, from, to, nol, 0, 0, tsv1, tsv2, loopOption, leh, leq, nor, ledTimming, speedScrolling, anim, var)
#define eWS2812BSetParamsTraceur(segInd, from, to, tkOn, tkOff, tsv1, tsv2, loopOption, leh, leq, nor, ledTimming, speedScrolling, anim, var)        eWS2812BSetAnimationParams(1, segInd, from, to, 0, tkOn, tkOff, tsv1, tsv2, loopOption, leh, leq, nor, ledTimming, speedScrolling, anim, var)

#endif
