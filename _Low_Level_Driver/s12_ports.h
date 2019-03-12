#ifndef __DEF_PORTS
#define	__DEF_PORTS

#define JTAGPortEnable(_enable)     (DDPCONbits.JTAGEN = (_enable))

#define _PORT(a, b)                 PORT ## a ## bits.R ## a ## b
#define mPORT(a)                    _PORT(a)
#define _LAT(a, b)                  LAT ## a ## bits.LAT ## a ## b
#define mLAT(a)                     _LAT(a)
#define _TRIS(a, b)                 TRIS ## a ## bits.TRIS ## a ## b
#define mTRIS(a)                    _TRIS(a)

#define mSetPinsDigitalIn(a)        (_TRIS(a) = BIT_IN, _PORT(a) = 0)
#define mSetPinsDigitalOut(a)       (_TRIS(a) = BIT_OUT, _PORT(a) = 0)
#define mInitIOAsInput(a)           (_TRIS(a) = BIT_IN, _PORT(a) = 0)
#define mInitIOAsOutput(a)          (_TRIS(a) = BIT_OUT, _PORT(a) = 0)
#define mGetIO(a)                   (_PORT(a))
#define mLatIO(a)                   (_LAT(a))
#define mSetIO(a)                   (_LAT(a) = 1)
#define mClrIO(a)                   (_LAT(a) = 0)
#define mInvIO(a)                   (_LAT(a) = !(_LAT(a)))
#define mToggleIO(a)                (_LAT(a) = !(_LAT(a)))

#define _XBR(a, b)              	bR ## a
#define _IND(a, b)                  b
#define f_PWMx_ENABLE(a)            _XBR(a)
#define f_PWMxDeamon(a)         	if(tick <= f_PWM ## a)   mSetIO(f_PWM ## a ## _EN); else    mClrIO(f_PWM ## a ## _EN);
#define f_PWMxDeamonInv(a)      	if(tick <= f_PWM ## a)   mClrIO(f_PWM ## a ## _EN); else    mSetIO(f_PWM ## a ## _EN);

#define CN_ON                       (1 << _CNCON_ON_POSITION)   /* CN ON */
#define CN_OFF                      (0)                         /* CN OFF */

#define CN21_PULLUP_ENABLE          (1 << _CNPUE_CNPUE21_POSITION)
#define CN20_PULLUP_ENABLE          (1 << _CNPUE_CNPUE20_POSITION)
#define CN19_PULLUP_ENABLE          (1 << _CNPUE_CNPUE19_POSITION)
#define CN18_PULLUP_ENABLE          (1 << _CNPUE_CNPUE18_POSITION)
#define CN17_PULLUP_ENABLE          (1 << _CNPUE_CNPUE17_POSITION)
#define CN16_PULLUP_ENABLE          (1 << _CNPUE_CNPUE16_POSITION)
#define CN15_PULLUP_ENABLE          (1 << _CNPUE_CNPUE15_POSITION)
#define CN14_PULLUP_ENABLE          (1 << _CNPUE_CNPUE14_POSITION)
#define CN13_PULLUP_ENABLE          (1 << _CNPUE_CNPUE13_POSITION)
#define CN12_PULLUP_ENABLE          (1 << _CNPUE_CNPUE12_POSITION)
#define CN11_PULLUP_ENABLE          (1 << _CNPUE_CNPUE11_POSITION)
#define CN10_PULLUP_ENABLE          (1 << _CNPUE_CNPUE10_POSITION)
#define CN9_PULLUP_ENABLE           (1 << _CNPUE_CNPUE9_POSITION)
#define CN8_PULLUP_ENABLE           (1 << _CNPUE_CNPUE8_POSITION)
#define CN7_PULLUP_ENABLE           (1 << _CNPUE_CNPUE7_POSITION)
#define CN6_PULLUP_ENABLE           (1 << _CNPUE_CNPUE6_POSITION)
#define CN5_PULLUP_ENABLE           (1 << _CNPUE_CNPUE5_POSITION)
#define CN4_PULLUP_ENABLE           (1 << _CNPUE_CNPUE4_POSITION)
#define CN3_PULLUP_ENABLE           (1 << _CNPUE_CNPUE3_POSITION)
#define CN2_PULLUP_ENABLE           (1 << _CNPUE_CNPUE2_POSITION)
#define CN1_PULLUP_ENABLE           (1 << _CNPUE_CNPUE1_POSITION)
#define CN0_PULLUP_ENABLE           (1 << _CNPUE_CNPUE0_POSITION)
#define CN_PULLUP_DISABLE_ALL       (0)             /* Default */

#define bR0                     	0
#define bRA                     	1
#define bRB                     	2
#define bRC                     	3
#define bRD                     	4
#define bRE                     	5
#define bRF                     	6
#define bRG                     	7

#define __0                         0, 0
// PORT A
#define __PA0                       A, 0
#define __PA1                       A, 1
#define __PA2                       A, 2
#define __PA3                       A, 3
#define __PA4                       A, 4
#define __PA5                       A, 5
#define __PA6                       A, 6
#define __PA7                       A, 7
#define __PA8                       A, 8
#define __PA9                       A, 9
#define __PA10                      A, 10
#define __PA11                      A, 11
#define __PA12                      A, 12
#define __PA13                      A, 13
#define __PA14                      A, 14
#define __PA15                      A, 15
// PORT B
#define __PB0                       B, 0
#define __PB1                       B, 1
#define __PB2                       B, 2
#define __PB3                       B, 3
#define __PB4                       B, 4
#define __PB5                       B, 5
#define __PB6                       B, 6
#define __PB7                       B, 7
#define __PB8                       B, 8
#define __PB9                       B, 9
#define __PB10                      B, 10
#define __PB11                      B, 11
#define __PB12                      B, 12
#define __PB13                      B, 13
#define __PB14                      B, 14
#define __PB15                      B, 15
// PORT C
#define __PC0                       C, 0
#define __PC1                       C, 1
#define __PC2                       C, 2
#define __PC3                       C, 3
#define __PC4                       C, 4
#define __PC5                       C, 5
#define __PC6                       C, 6
#define __PC7                       C, 7
#define __PC8                       C, 8
#define __PC9                       C, 9
#define __PC10                      C, 10
#define __PC11                      C, 11
#define __PC12                      C, 12
#define __PC13                      C, 13
#define __PC14                      C, 14
#define __PC15                      C, 15
// PORT D
#define __PD0                       D, 0
#define __PD1                       D, 1
#define __PD2                       D, 2
#define __PD3                       D, 3
#define __PD4                       D, 4
#define __PD5                       D, 5
#define __PD6                       D, 6
#define __PD7                       D, 7
#define __PD8                       D, 8
#define __PD9                       D, 9
#define __PD10                      D, 10
#define __PD11                      D, 11
#define __PD12                      D, 12
#define __PD13                      D, 13
#define __PD14                      D, 14
#define __PD15                      D, 15
// PORT E
#define __PE0                       E, 0
#define __PE1                       E, 1
#define __PE2                       E, 2
#define __PE3                       E, 3
#define __PE4                       E, 4
#define __PE5                       E, 5
#define __PE6                       E, 6
#define __PE7                       E, 7
#define __PE8                       E, 8
#define __PE9                       E, 9
#define __PE10                      E, 10
#define __PE11                      E, 11
#define __PE12                      E, 12
#define __PE13                      E, 13
#define __PE14                      E, 14
#define __PE15                      E, 15
// PORT F
#define __PF0                       F, 0
#define __PF1                       F, 1
#define __PF2                       F, 2
#define __PF3                       F, 3
#define __PF4                       F, 4
#define __PF5                       F, 5
#define __PF6                       F, 6
#define __PF7                       F, 7
#define __PF8                       F, 8
#define __PF9                       F, 9
#define __PF10                      F, 10
#define __PF11                      F, 11
#define __PF12                      F, 12
#define __PF13                      F, 13
#define __PF14                      F, 14
#define __PF15                      F, 15
// PORT G
#define __PG0                       G, 0
#define __PG1                       G, 1
#define __PG2                       G, 2
#define __PG3                       G, 3
#define __PG4                       G, 4
#define __PG5                       G, 5
#define __PG6                       G, 6
#define __PG7                       G, 7
#define __PG8                       G, 8
#define __PG9                       G, 9
#define __PG10                      G, 10
#define __PG11                      G, 11
#define __PG12                      G, 12
#define __PG13                      G, 13
#define __PG14                      G, 14
#define __PG15                      G, 15

typedef enum
{
    ACTIVE_LOW = 0,
    ACTIVE_HIGH
} _IO_ACTIVE_STATE;

typedef struct
{
    uint8_t             _port;
    uint8_t             _indice;
} _IO;

typedef struct
{
	volatile uint32_t	TRIS;
	volatile uint32_t	TRISCLR;
	volatile uint32_t	TRISSET;
	volatile uint32_t	TRISINV;
    
	volatile uint32_t	PORT;
	volatile uint32_t	PORTCLR;
	volatile uint32_t	PORTSET;
	volatile uint32_t	PORTINV;
    
	volatile uint32_t	LAT;
	volatile uint32_t	LATCLR;
	volatile uint32_t	LATSET;
	volatile uint32_t	LATINV;
} PORTS_REGISTERS;

typedef void (*ports_event_handler_t)();

void ports_change_notice_init(uint32_t cn_pull_up, uint32_t cn_pins_enable, ports_event_handler_t evt_handler);
void ports_reset_all_pins_input();
void ports_reset_pin_input(_IO io);
void ports_reset_pin_output(_IO io);
bool ports_get_bit(_IO io);
void ports_set_bit(_IO io);
void ports_clr_bit(_IO io);
void ports_toggle_bit(_IO io);
void ports_interrupt_handler();

#endif
