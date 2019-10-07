#ifndef __DEF_TIMERS
#define __DEF_TIMERS

volatile uint64_t getTick;
volatile uint64_t getLastCompare;
volatile uint64_t getTime;

#define mGetTick()                  (getTick += TMR1, (TMR1 = 12), getTick)
#define mTickCompare(var)           (getLastCompare = (mGetTick() - var))
#define mUpdateTick(var)            (var = mGetTick())
#define mUpdateTick_withCathingUpTime(var, time)     (var = (uint64_t) (mGetTick() - (getLastCompare - time)))
#define Delay_ms(v)                 {                                                       \
                                        uint64_t vDelay = mGetTick();                       \
                                        do {} while(mTickCompare(vDelay) < (v*TICK_1MS));   \
                                    }
#define mResetTime()                mUpdateTick(getTime)
#define mGetTime()                  ((mGetTick() - getTime) / TICK_1US)
#define TICK_INIT                   (0ul)
#define TICK_0                      (0ul)

#define TIMER1_CLR_FLAG()           (IFS0CLR = _IFS0_T1IF_MASK)
#define TIMER2_CLR_FLAG()           (IFS0CLR = _IFS0_T2IF_MASK)
#define TIMER3_CLR_FLAG()           (IFS0CLR = _IFS0_T3IF_MASK)
#define TIMER4_CLR_FLAG()           (IFS0CLR = _IFS0_T4IF_MASK)
#define TIMER5_CLR_FLAG()           (IFS0CLR = _IFS0_T5IF_MASK)
        
#if (PERIPHERAL_FREQ == 48000000L)
#define TICK_1US                    (48ul)
#define TICK_10US                   (480ul)
#define TICK_20US                   (960ul)
#define TICK_100US                  (4800ul)
#define TICK_300US                  (14400ul)
#define TICK_500US                  (24000ul)
#define TICK_1MS                    (48000ul)
#define TICK_2MS                    (96000ul)
#define TICK_3MS                    (144000ul)
#define TICK_4MS                    (192000ul)
#define TICK_5MS                    (240000ul)
#define TICK_6MS                    (288000ul)
#define TICK_7MS                    (336000ul)
#define TICK_8MS                    (384000ul)
#define TICK_9MS                    (432000ul)
#define TICK_10MS                   (480000ul)
#define TICK_20MS                   (960000ul)
#define TICK_30MS                   (1440000ul)
#define TICK_40MS                   (1920000ul)
#define TICK_50MS                   (2400000ul)
#define TICK_60MS                   (2880000ul)
#define TICK_70MS                   (3360000ul)
#define TICK_80MS                   (3840000ul)
#define TICK_90MS                   (4320000ul)
#define TICK_100MS                  (4800000ul)
#define TICK_200MS                  (9600000ul)
#define TICK_300MS                  (14400000ul)
#define TICK_400MS                  (19200000ul)
#define TICK_500MS                  (24000000ul)
#define TICK_600MS                  (28800000ul)
#define TICK_700MS                  (33600000ul)
#define TICK_800MS                  (38400000ul)
#define TICK_900MS                  (43200000ul)
#define TICK_1S                     (48000000ul)
#define TICK_1_5S                   (72000000ul)
#define TICK_2S                     (96000000ul)
#define TICK_3S                     (144000000ul)
#define TICK_4S                     (192000000ul)
#define TICK_5S                     (240000000ul)
#define TICK_10S                    (480000000ul)
#elif (PERIPHERAL_FREQ == 80000000L)
#define TICK_1US                    (80ul)
#define TICK_10US                   (800ul)
#define TICK_20US                   (1600ul)
#define TICK_100US                  (8000ul)
#define TICK_200US                  (16000ul)
#define TICK_300US                  (24000ul)
#define TICK_400US                  (32000ul)
#define TICK_500US                  (40000ul)
#define TICK_1MS                    (80000ul)
#define TICK_2MS                    (160000ul)
#define TICK_3MS                    (240000ul)
#define TICK_4MS                    (320000ul)
#define TICK_5MS                    (400000ul)
#define TICK_6MS                    (480000ul)
#define TICK_7MS                    (560000ul)
#define TICK_8MS                    (640000ul)
#define TICK_9MS                    (720000ul)
#define TICK_10MS                   (800000ul)
#define TICK_20MS                   (1600000ul)
#define TICK_30MS                   (2400000ul)
#define TICK_40MS                   (3200000ul)
#define TICK_50MS                   (4000000ul)
#define TICK_60MS                   (4800000ul)
#define TICK_70MS                   (5600000ul)
#define TICK_80MS                   (6400000ul)
#define TICK_90MS                   (7200000ul)
#define TICK_100MS                  (8000000ul)
#define TICK_200MS                  (16000000ul)
#define TICK_300MS                  (24000000ul)
#define TICK_400MS                  (32000000ul)
#define TICK_500MS                  (40000000ul)
#define TICK_600MS                  (48000000ul)
#define TICK_700MS                  (56000000ul)
#define TICK_800MS                  (64000000ul)
#define TICK_900MS                  (72000000ul)
#define TICK_1S                     (80000000ul)
#define TICK_1_5S                   (120000000ul)
#define TICK_2S                     (160000000ul)
#define TICK_3S                     (240000000ul)
#define TICK_4S                     (320000000ul)
#define TICK_5S                     (400000000ul)
#define TICK_10S                    (800000000ul)
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

typedef enum 
{
    TIMER1                          = 0,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER_NUMBER_OF_MODULES
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

typedef void (*timer_event_handler_t)(uint8_t id);

void timer_init_2345_us(TIMER_MODULE id, timer_event_handler_t evt_handler, uint32_t config, float period_us);
float timer_get_period_us(TIMER_MODULE id);

#define timer_init_2345_hz(id, evt_handler, config, freq_hz)        (timer_init_2345_us(id, evt_handler, config, (float)(1000000.0/(freq_hz))))
#define timer_get_frequency_hz(id)                                  (float)(1000000.0/timer_get_period_us(id))


void timer_interrupt_handler(TIMER_MODULE id);
	
#endif
