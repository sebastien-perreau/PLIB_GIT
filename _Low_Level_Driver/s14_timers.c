/*********************************************************************
 *	TIMER modules (1, 2, 3, 4 et 5)
 *	Author : Sébastien PERREAU
 *
 *	Revision history	:
 *               15/11/2013      - Initial release
 *               06/10/2018      - Compatibility PLIB
 *                               - No dependencies to xc32 library
 *                               - Add comments   
 *               13/03/2019      - Bug fixed for get_period
 *                               - General improvements
 ********************************************************************/

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
static timer_event_handler_t timer_event_handler[TIMER_NUMBER_OF_MODULES] = {NULL};

/*******************************************************************************
  Function:
    void timer_init_2345_us(TIMER_MODULE id, timer_event_handler_t evt_handler, uint32_t config, float period_us)

  Description:
    This routine is used to initialize a timer module.

  Parameters:
    id          - The TIMER module you want to use.
    evt_handler - The handler (function) to call when an interruption occurs.
    config      - The TIMER configuration (TCON register).
    period_us   - Set the period in micro-seconds for the timer. Time before back 
                to zero and/or interrupt generation.
  *****************************************************************************/
void timer_init_2345_us(TIMER_MODULE id, timer_event_handler_t evt_handler, uint32_t config, float period_us)
{
    TIMER_REGISTERS * p_timer = (TIMER_REGISTERS *) TimerModules[id];
    float v_pr = 100000;
    uint16_t v_prescale = 1;
    
    timer_event_handler[id] = evt_handler;
    irq_init(IRQ_T1 + id, (evt_handler != NULL) ? IRQ_ENABLED : IRQ_DISABLED, irq_timer_priority(id));

    while(v_pr > 65535.0)
    {
        v_pr = (float)((period_us * (PERIPHERAL_FREQ / 1000000.0)) / v_prescale);
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
    
    p_timer->TCONCLR    = TMR_ON;
    p_timer->TMR        = 0x0000;
    p_timer->PR         = v_pr;
    p_timer->TCONSET    = config;
}

/*******************************************************************************
  Function:
    float timer_get_period_us(TIMER_MODULE id)

  Description:
    This routine returns the current period of a timer in micro-seconds.

  Parameters:
    id  - The TIMER module you want to use.
 
  Return:
    The value (float value) corresponding to the period in micro-seconds.
  *****************************************************************************/
float timer_get_period_us(TIMER_MODULE id)
{
    TIMER_REGISTERS * p_timer = (TIMER_REGISTERS *) TimerModules[id];

    if(id == TIMER1)
    {
        return (float) (p_timer->PR * pow(8, (p_timer->TCON >> 4) & 0x0003) * 1000000.0 / PERIPHERAL_FREQ);
    }
    else
    {
        if (((p_timer->TCON >> 4) & 7) < 7)
        {
            return (float) (p_timer->PR * pow(2, (p_timer->TCON >> 4) & 7) * 1000000.0 / PERIPHERAL_FREQ);
        }
        else
        {
            return (float) (p_timer->PR * 256.0 * 1000000 / PERIPHERAL_FREQ);
        }
    }
}

/*******************************************************************************
  Function:
    void timer_interrupt_handler(TIMER_MODULE id)

  Description:
    This routine is called when an interruption occurs. This interrupt 
    handler calls the user _event_handler (if existing) otherwise do nothing.

  Parameters:
    id  - The TIMER module you want to use.
  *****************************************************************************/
void timer_interrupt_handler(TIMER_MODULE id)
{
    if (timer_event_handler[id] != NULL)
    {
        (*timer_event_handler[id])(id);
    }
}
