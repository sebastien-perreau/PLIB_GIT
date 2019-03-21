#ifndef __DEF_PWM
#define	__DEF_PWM

typedef enum 
{
    PWM1                        = 0,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM_NUMBER_OF_MODULES
} PWM_MODULE;

typedef enum
{
    PWM_NONE                    = 0x0000,
    
    PWM1_T2_ON                  = 0x0001,
    PWM1_T3_ON                  = 0x0002,
    
    PWM2_T2_ON                  = 0x0004,
    PWM2_T3_ON                  = 0x0008,
            
    PWM3_T2_ON                  = 0x0010,
    PWM3_T3_ON                  = 0x0020,
            
    PWM4_T2_ON                  = 0x0040,
    PWM4_T3_ON                  = 0x0080,
           
    PWM5_T2_ON                  = 0x0100,
    PWM5_T3_ON                  = 0x0200,
            
    PWM_T2_MASK                 = 0x0155,
    PWM_T3_MASK                 = 0x02aa
} PWM_MODULE_ENABLE;

typedef enum
{
    OC_ON                       = (1 << _OC1CON_ON_POSITION),
    OC_OFF                      = (0),
            
    OC_IDLE_STOP                = (1 << _OC1CON_OCSIDL_POSITION),   /* Stop in idle mode */
    OC_IDLE_CON                 = (0),                              /* Continue operation in idle mode */
            
    OC_TIMER_MODE32             = (1 << _OC1CON_OC32_POSITION),     /* Use 32 bit mode */
    OC_TIMER_MODE16             = (0),                              /* Use 16 bit mode */
            
    OC_TIMER3_SRC               = (1 << _OC1CON_OCTSEL_POSITION),   /* Timer3 is the clock source */
    OC_TIMER2_SRC               = (0),                              /* Timer2 is the clock source */
            
    OC_PWM_FAULT_PIN_ENABLE     = (7 << _OC1CON_OCM_POSITION),		/* PWM Mode on OCx, fault pin enabled */
    OC_PWM_FAULT_PIN_DISABLE    = (6 << _OC1CON_OCM_POSITION),      /* PWM Mode on OCx, fault pin disabled */
    OC_CONTINUE_PULSE           = (5 << _OC1CON_OCM_POSITION),      /* Generates Continuous Output pulse on OCx Pin */
    OC_SINGLE_PULSE             = (4 << _OC1CON_OCM_POSITION),      /* Generates Single Output pulse on OCx Pin */
    OC_TOGGLE_PULSE             = (3 << _OC1CON_OCM_POSITION),      /* Compare1 toggels  OCx pin*/
    OC_HIGH_LOW                 = (2 << _OC1CON_OCM_POSITION),      /* Compare1 forces OCx pin Low*/
    OC_LOW_HIGH                 = (1 << _OC1CON_OCM_POSITION),      /* Compare1 forces OCx pin High*/
    OC_MODE_OFF                 = (0 << _OC1CON_OCM_POSITION)       /* OutputCompare x Off*/
} PWM_OCXCON_REGISTER;

typedef struct 
{
    volatile UINT32 OCxCON;
    volatile UINT32 OCxCONCLR;
    volatile UINT32 OCxCONSET;
    volatile UINT32 OCxCONINV;

    volatile UINT32 OCxR;
    volatile UINT32 OCxRCLR;
    volatile UINT32 OCxRSET;
    volatile UINT32 OCxRINV;

    volatile UINT32 OCxRS;
    volatile UINT32 OCxRSCLR;
    volatile UINT32 OCxRSSET;
    volatile UINT32 OCxRSINV;
} OUTPUT_COMPARE_REGISTERS;

void pwm_init(PWM_MODULE_ENABLE pwm_ids, uint32_t t2_freq_hz, uint32_t t3_freq_hz);
void pwm_set_duty_cycle(PWM_MODULE pwm_id, uint8_t dc);

#endif

