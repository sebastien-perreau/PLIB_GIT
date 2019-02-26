#ifndef __DEF_TIMERS
#define __DEF_TIMERS

volatile QWORD getTick;
volatile QWORD temporary_tmr1_value;

#define TMR1_GET_INT_FLAG           (IFS0bits.T1IF)
#define mGetTick()                  ((!(temporary_tmr1_value = TMR1) | TMR1_GET_INT_FLAG)?(temporary_tmr1_value + (getTick += (TMR1_GET_INT_FLAG << 16)) + (TMR1_GET_INT_FLAG = 0)):(temporary_tmr1_value + getTick))
#define mTickCompare(var)           (mGetTick() - var)
#define TICK_INIT                   (0ul)
#define TICK_0                      (0ul)
        
#if (PERIPHERAL_FREQ == 48000000L)
#define TICK_1US                    (6ul)
#define TICK_10US                   (60ul)
#define TICK_100US                  (600ul)
#define TICK_300US                  (1800ul)
#define TICK_500US                  (3000ul)
#define TICK_1MS                    (6000ul)
#define TICK_2MS                    (12000ul)
#define TICK_3MS                    (18000ul)
#define TICK_4MS                    (24000ul)
#define TICK_5MS                    (30000ul)
#define TICK_6MS                    (36000ul)
#define TICK_8MS                    (48000ul)
#define TICK_10MS                   (60000ul)
#define TICK_20MS                   (120000ul)
#define TICK_30MS                   (180000ul)
#define TICK_40MS                   (240000ul)
#define TICK_50MS                   (300000ul)
#define TICK_60MS                   (360000ul)
#define TICK_70MS                   (420000ul)
#define TICK_80MS                   (480000ul)
#define TICK_90MS                   (540000ul)
#define TICK_100MS                  (600000ul)
#define TICK_200MS                  (1200000ul)
#define TICK_300MS                  (1800000ul)
#define TICK_400MS                  (2400000ul)
#define TICK_500MS                  (3000000ul)
#define TICK_1S                     (6000000ul)
#define TICK_1_5S                   (9000000ul)
#define TICK_2S                     (12000000ul)
#define TICK_3S                     (18000000ul)
#define TICK_4S                     (24000000ul)
#define TICK_5S                     (30000000ul)
#define TICK_10S                    (60000000ul)
#elif (PERIPHERAL_FREQ == 80000000L)
#define TICK_1US                    (10ul)
#define TICK_10US                   (100ul)
#define TICK_100US                  (1000ul)
#define TICK_200US                  (2000ul)
#define TICK_300US                  (3000ul)
#define TICK_400US                  (4000ul)
#define TICK_500US                  (5000ul)
#define TICK_1MS                    (10000ul)
#define TICK_2MS                    (20000ul)
#define TICK_3MS                    (30000ul)
#define TICK_4MS                    (40000ul)
#define TICK_5MS                    (50000ul)
#define TICK_6MS                    (60000ul)
#define TICK_8MS                    (80000ul)
#define TICK_10MS                   (100000ul)
#define TICK_20MS                   (200000ul)
#define TICK_30MS                   (300000ul)
#define TICK_40MS                   (400000ul)
#define TICK_50MS                   (500000ul)
#define TICK_60MS                   (600000ul)
#define TICK_70MS                   (700000ul)
#define TICK_80MS                   (800000ul)
#define TICK_90MS                   (900000ul)
#define TICK_100MS                  (1000000ul)
#define TICK_200MS                  (2000000ul)
#define TICK_300MS                  (3000000ul)
#define TICK_400MS                  (4000000ul)
#define TICK_500MS                  (5000000ul)
#define TICK_1S                     (10000000ul)
#define TICK_1_5S                   (15000000ul)
#define TICK_2S                     (20000000ul)
#define TICK_3S                     (30000000ul)
#define TICK_4S                     (40000000ul)
#define TICK_5S                     (50000000ul)
#define TICK_10S                    (100000000ul)
#else
#error "timer.h - value not define for getTick"
#endif

#define TMR_ON                      (1 << _T1CON_ON_POSITION)       /* Timer ON */
#define TMR_OFF                     (0)                             /* Timer OFF */
#define TMR_IDLE_STOP               (1 << _T1CON_SIDL_POSITION)     /* stop during idle */
#define TMR_IDLE_CON                (0)                             /* operate during idle */
#define TMR_TMWDIS_ON               (1 << _T1CON_TWDIS_POSITION)    /* Asynchronous Write Disable */
#define TMR_TMWDIS_OFF              (0)
#define TMR_GATE_ON                 (1 << _T1CON_TGATE_POSITION)    /* Timer Gate accumulation mode ON */
#define TMR_GATE_OFF                (0)                             /* Timer Gate accumulation mode OFF */
#define TMR_SYNC_EXT_ON             (1 << _T1CON_TSYNC_POSITION)    /* Synch external clk input */
#define TMR_SYNC_EXT_OFF            (0)
#define TMR_SOURCE_EXT              (1 << _T1CON_TCS_POSITION)      /* External clock source */
#define TMR_SOURCE_INT              (0)                             /* Internal clock source */
#define TMR_32BIT_MODE_ON           (1 << _T2CON_T32_POSITION)      /* Enable 32-bit mode */
#define TMR_32BIT_MODE_OFF          (0)                             /* Separate 16-bit timer */

#define TMR_2345_PS_1_256           (7 << _T2CON_TCKPS_POSITION)  
#define TMR_2345_PS_1_64            (6 << _T2CON_TCKPS_POSITION)   
#define TMR_2345_PS_1_32            (5 << _T2CON_TCKPS_POSITION)  
#define TMR_2345_PS_1_16            (4 << _T2CON_TCKPS_POSITION)   
#define TMR_2345_PS_1_8             (3 << _T2CON_TCKPS_POSITION)   
#define TMR_2345_PS_1_4             (2 << _T2CON_TCKPS_POSITION)   
#define TMR_2345_PS_1_2             (1 << _T2CON_TCKPS_POSITION)   
#define TMR_2345_PS_1_1             (0)		
#define TMR_1_PS_1_8                (1 << _T1CON_TCKPS_POSITION)

typedef enum {
    TIMER1 = 0,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER_NUMBER_OF_MODULES   // Number of available TMR modules.
} TIMER_MODULE;


typedef struct 
{
	volatile UINT32 TCON;
    volatile UINT32 TCONCLR;
    volatile UINT32 TCONSET;
    volatile UINT32 TCONINV;

    volatile UINT32 TMR;
    volatile UINT32 TMRCLR;
    volatile UINT32 TMRSET;
    volatile UINT32 TMRINV;

    volatile UINT32 PR;
    volatile UINT32 PRCLR;
    volatile UINT32 PRSET;
    volatile UINT32 PRINV;
} TIMER_REGISTERS;

void timer_init_2345_us(TIMER_MODULE id, basic_event_handler_t evt_handler, uint32_t config, uint32_t period_us);
#define timer_init_2345_hz(id, evt_handler, config, freq_hz)        (timer_init_2345_us(id, evt_handler, config, (uint32_t)(1000000.0/freq_hz)))
double timer_get_period_us(TIMER_MODULE id);
#define timer_get_frequency_hz(id)                                  (uint32_t)(1000000.0/timer_get_period_us(id));


void timer_interrupt_handler(TIMER_MODULE id);
	
#endif
