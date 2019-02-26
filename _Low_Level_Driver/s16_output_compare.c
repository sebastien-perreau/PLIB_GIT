/*********************************************************************
*	PWM modules (OC1, OC2, OC3, OC4 et OC5)
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		14/11/2013		- Initial release
*********************************************************************/

#include "../PLIB.h"

extern const PWM_REGISTERS * pwmModules[];
const PWM_REGISTERS * pwmModules[] =
{
#ifdef _OCMP1
    (PWM_REGISTERS *)_OCMP1_BASE_ADDRESS,
#endif
#ifdef _OCMP2
    (PWM_REGISTERS *)_OCMP2_BASE_ADDRESS,
#endif
#ifdef _OCMP3
    (PWM_REGISTERS *)_OCMP3_BASE_ADDRESS,
#endif
#ifdef _OCMP4
    (PWM_REGISTERS *)_OCMP4_BASE_ADDRESS,
#endif
#ifdef _OCMP5
    (PWM_REGISTERS *)_OCMP5_BASE_ADDRESS
#endif
};

/*******************************************************************************
  Function:
    void PWMInit(PWM_MODULE mPwmModule, DWORD config);

  Description:
    This routine initialise the PWM module.

  Parameters:
    module      - Identifies the desired PWM module.

    config      - configuration of the PWM module.

  Returns:
    None.

  Example:
    <code>

    PWMInit(PWM3, (OC_ON | OC_IDLE_CON | OC_TIMER_MODE16 | OC_PWM_FAULT_PIN_DISABLE | OC_TIMER3_SRC));

    </code>
  *****************************************************************************/
void PWMInit(PWM_MODULE mPwmModule, DWORD config)
{
    if(config&_OC1CON_ON_MASK)
    {
        PWM_REGISTERS * pwmRegister = (PWM_REGISTERS *)pwmModules[mPwmModule];

        pwmRegister->OCxR = 0X0000;
        pwmRegister->OCxRS = 0x0000;
        pwmRegister->OCxCON = config;
    }
}

/*******************************************************************************
  Function:
    void PWMDutyCycle(PWM_MODULE mPwmModule, double dc);

  Description:
    This routine set the desire duty cycle.

  Parameters:
    module      - Identifies the desired PWM module.

    config      - Duty cycle of the PWM (0.0 to 100.0).

  Returns:
    None.

  Example:
    <code>

 PWMDutyCycle(PWM2, 47.8);

    </code>
  *****************************************************************************/
void PWMDutyCycle(PWM_MODULE mPwmModule, double dc)
{
    PWM_REGISTERS * pwmRegister = (PWM_REGISTERS *)pwmModules[mPwmModule];

    if(dc >= 100.0)
    {
        dc = 101.0;
    }
    if(pwmRegister->OCxCON&_OC1CON_OCTSEL_MASK)
    {
        pwmRegister->OCxRS = (double)(dc*PR3/100.0);
    }
    else
    {
        pwmRegister->OCxRS = (double)(dc*PR2/100.0);
    }
}

/*******************************************************************************
  Function:
    double PWMGetDutyCycleResolution(PWM_MODULE mPwmModule);

  Description:
    This routine return the resolution of the duty cycle for the
    desire PWM module.

  Parameters:
    module      - Identifies the desired PWM module.

  Returns:
    double      - Desire duty cycle resolution (in bits).

  Example:
    <code>

 ret = PWMGetDutyCycleResolution(PWM4);

    </code>
  *****************************************************************************/
double PWMGetDutyCycleResolution(PWM_MODULE mPwmModule)
{
    PWM_REGISTERS * pwmRegister = (PWM_REGISTERS *)pwmModules[mPwmModule];

    if(pwmRegister->OCxCON&_OC1CON_OCTSEL_MASK)
    {
        return (log10(PR3+1)/log10(2));
    }
    else
    {
        return (log10(PR2+1)/log10(2));
    }
}

/*******************************************************************************
  Function:
    double PWMGetDutyCycle(PWM_MODULE mPwmModule);

  Description:
    This routine return the current duty cycle for the
    desire PWM module.

  Parameters:
    module      - Identifies the desired PWM module.

  Returns:
    double      - Desire duty cycle (in %).

  Example:
    <code>

 ret = PWMGetDutyCycle(PWM4);

    </code>
  *****************************************************************************/
double PWMGetDutyCycle(PWM_MODULE mPwmModule)
{
    PWM_REGISTERS * pwmRegister = (PWM_REGISTERS *)pwmModules[mPwmModule];

    if(pwmRegister->OCxCON&_OC1CON_OCTSEL_MASK)
    {
        return (double)(pwmRegister->OCxR*100.0/PR3);
    }
    else
    {
        return (double)(pwmRegister->OCxR*100.0/PR2);
    }
}

/*******************************************************************************
  Function:
    double PWMGetFrequency(PWM_MODULE mPwmModule);

  Description:
    This routine return the current frequency for the
    desire PWM module.

  Parameters:
    module      - Identifies the desired PWM module.

  Returns:
    double      - Desire frequency (in Hz).

  Example:
    <code>

 ret = PWMGetFrequency(PWM4);

    </code>
  *****************************************************************************/
double PWMGetFrequency(PWM_MODULE mPwmModule)
{
    PWM_REGISTERS * pwmRegister = (PWM_REGISTERS *)pwmModules[mPwmModule];

    if(pwmRegister->OCxCON&_OC1CON_OCTSEL_MASK)
    {
        return (double)(PERIPHERAL_FREQ/(PR3*pow(2, T3CONbits.TCKPS)));
    }
    else
    {
        return (double)(PERIPHERAL_FREQ/(PR2*pow(2, T2CONbits.TCKPS)));
    }
}
