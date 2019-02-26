/*********************************************************************
*	TIMER modules (1, 2, 3, 4 et 5)
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               15/11/2013      - Initial release
*               06/10/2018      - Compatibility PLIB
*                               - No dependencies to xc32 library
*                               - Add comments   
*********************************************************************/

#include "../PLIB.h"

extern const TIMER_REGISTERS * TimerModules[];
const TIMER_REGISTERS * TimerModules[] =
{
    (TIMER_REGISTERS *)_TMR1_BASE_ADDRESS,
    (TIMER_REGISTERS *)_TMR2_BASE_ADDRESS,
    (TIMER_REGISTERS *)_TMR3_BASE_ADDRESS,
    (TIMER_REGISTERS *)_TMR4_BASE_ADDRESS,
    (TIMER_REGISTERS *)_TMR5_BASE_ADDRESS
};
static basic_event_handler_t timer_event_handler[TIMER_NUMBER_OF_MODULES] = {NULL};

/*******************************************************************************
 * Function: 
 *      void timer_init_2345_us(TIMER_MODULE id, uint32_t config, double period_us)
 * 
 * Description:
 *      This routine is used to initialize a timer module.
 * 
 * Parameters:
 *      id: The desire TIMER_MODULE.
 *      evt_handler: The handler (function) to call when an interruption occurs.
 *      config: The desire timer configuration (TCON register).
 *      period_us: Set the period in micro-seconds for the timer. Time before 
 *                  back to zero and/or interrupt generation.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      See. cfg_pwm() in "config.c"
 ******************************************************************************/
void timer_init_2345_us(TIMER_MODULE id, basic_event_handler_t evt_handler, uint32_t config, uint32_t period_us)
{
    TIMER_REGISTERS * p_timer = (TIMER_REGISTERS *) TimerModules[id];
    uint32_t v_pr = 100000;
    uint16_t v_prescale = 1;

    while(v_pr > 65535)
    {
        v_pr = (uint32_t)((period_us * (PERIPHERAL_FREQ / 1000000L)) / v_prescale);
        if(v_prescale == 1) 
        {
            v_prescale = 2;
            config |= TMR_2345_PS_1_1;
        }
        else if(v_prescale == 2) 
        {
            v_prescale = 4;
            config |= TMR_2345_PS_1_2;
        }
        else if(v_prescale == 4) 
        {
            v_prescale = 8;
            config |= TMR_2345_PS_1_4;
        }
        else if(v_prescale == 8) 
        {
            v_prescale = 16;
            config |= TMR_2345_PS_1_8;
        }
        else if(v_prescale == 16) 
        {
            v_prescale = 32;
            config |= TMR_2345_PS_1_16;
        }
        else if(v_prescale == 32) 
        {
            v_prescale = 64;
            config |= TMR_2345_PS_1_32;
        }
        else if(v_prescale == 64) 
        {
            v_prescale = 256;
            config |= TMR_2345_PS_1_64;
        }
        else if(v_prescale == 256) 
        {
            v_prescale = 512;
            config |= TMR_2345_PS_1_256;
        }
        else
        {
            break;
        }
    }
    
    timer_event_handler[id] = evt_handler;
    
    p_timer->TCONCLR    = TMR_ON;
    p_timer->TMR        = 0x0000;
    p_timer->PR         = v_pr;
    p_timer->TCONSET    = config;
}

/*******************************************************************************
 * Function: 
 *      double timer_get_period_us(TIMER_MODULE id)
 * 
 * Description:
 *      This routine returns the current period of a timer in micro-seconds.
 * 
 * Parameters:
 *      id: The desire TIMER_MODULE.
 * 
 * Return:
 *      The value as a 'double' variable corresponding to the period in micro-seconds.
 * 
 * Example:
 *      none
 ******************************************************************************/
double timer_get_period_us(TIMER_MODULE id)
{
    TIMER_REGISTERS * p_timer = (TIMER_REGISTERS *) TimerModules[id];

    if(id == TIMER1)
    {
        return (double)((p_timer->PR*pow(2, (p_timer->TCON >> 4) & 0x0003))*(1000000/PERIPHERAL_FREQ));
    }
    else
    {
        if(((p_timer->TCON >> 4) & 0x0007) < 3)
        {
            return (double)((p_timer->PR*pow(8, (p_timer->TCON >> 4) & 0x0007))*(1000000/PERIPHERAL_FREQ));
        }
        else
        {
            return (double)((p_timer->PR*256.0)*(1000000/PERIPHERAL_FREQ));
        }
    }
}

/*******************************************************************************
 * Function: 
 *      void timer_interrupt_handler(TIMER_MODULE id)
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      id: The TIMER module you want to use.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void timer_interrupt_handler(TIMER_MODULE id)
{
    if (timer_event_handler[id] != NULL)
    {
        (*timer_event_handler[id])(id);
    }
}
