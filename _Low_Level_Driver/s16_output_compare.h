#ifndef __DEF_PWM
#define	__DEF_PWM

#define OC_ON                      (1 << _OC1CON_ON_POSITION)
#define OC_OFF                     (0)

#define OC_IDLE_STOP               (1 << _OC1CON_OCSIDL_POSITION)   /* stop in idle mode */
#define OC_IDLE_CON                (0)                              /* continue operation in idle mode */

#define OC_TIMER_MODE32             (1 << _OC1CON_OC32_POSITION)    /* use 32 bit mode */
#define OC_TIMER_MODE16             (0)                             /* use 16 bit mode */

#define OC_TIMER3_SRC               (1 << _OC1CON_OCTSEL_POSITION)  /* Timer3 is the clock source */
#define OC_TIMER2_SRC               (0)                             /* Timer2 is the clock source */

#define OC_PWM_FAULT_PIN_ENABLE  	(7 << _OC1CON_OCM_POSITION)		/* PWM Mode on OCx, fault pin enabled */
#define OC_PWM_FAULT_PIN_DISABLE    (6 << _OC1CON_OCM_POSITION)     /* PWM Mode on OCx, fault pin disabled */
#define OC_CONTINUE_PULSE           (5 << _OC1CON_OCM_POSITION)     /* Generates Continuous Output pulse on OCx Pin */
#define OC_SINGLE_PULSE             (4 << _OC1CON_OCM_POSITION)     /* Generates Single Output pulse on OCx Pin */
#define OC_TOGGLE_PULSE             (3 << _OC1CON_OCM_POSITION)     /* Compare1 toggels  OCx pin*/
#define OC_HIGH_LOW                 (2 << _OC1CON_OCM_POSITION)     /* Compare1 forces OCx pin Low*/
#define OC_LOW_HIGH                 (1 << _OC1CON_OCM_POSITION)     /* Compare1 forces OCx pin High*/
#define OC_MODE_OFF                 (0 << _OC1CON_OCM_POSITION)     /* OutputCompare x Off*/

typedef enum 
{
    PWM1 = 0,               // PWM Module 1 ID.
    PWM2,                   // PWM Module 2 ID.
    PWM3,                   // PWM Module 3 ID.
    PWM4,                   // PWM Module 4 ID.
    PWM5,                   // PWM Module 5 ID.
    PWM_NUMBER_OF_MODULES   // Number of available PWM modules.
} PWM_MODULE;

typedef union 
{
    struct 
    {
        unsigned OCM:3;
        unsigned OCTSEL:1;
        unsigned OCFLT:1;
        unsigned OC32:1;
        unsigned :7;
        unsigned SIDL:1;
        unsigned :1;
        unsigned OCPON:1;
    };
    struct 
    {
        unsigned OCM0:1;
        unsigned OCM1:1;
        unsigned OCM2:1;
    };
    struct 
    {
        unsigned :13;
        unsigned OCSIDL:1;
    };
    struct 
    {
        unsigned w:32;
    };
} __OCxCONbits_t;

typedef struct 
{
    union 
    {
        volatile UINT32 OCxCON;
        volatile __OCxCONbits_t OCxCONbits;
    };
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
} PWM_REGISTERS;

void PWMInit(PWM_MODULE mPwmModule, DWORD config);
void PWMDutyCycle(PWM_MODULE mPwmModule, double dc);
double PWMGetDutyCycleResolution(PWM_MODULE mPwmModule);
double PWMGetDutyCycle(PWM_MODULE mPwmModule);
double PWMGetFrequency(PWM_MODULE mPwmModule);

#endif

