#ifndef __DEF_PORTS
#define	__DEF_PORTS

#define JTAGPortEnable(_enable)     (DDPCONbits.JTAGEN = (_enable))

#define _PORT(a, b)                 PORT ## a(b)
#define mPORT(a)                    _PORT(a)
#define _LAT(a, b)                  LAT ## a(b)
#define mLAT(a)                     _LAT(a)
#define _TRIS(a, b)                 TRIS ## a(b)
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

#define __PORT(a, b)              	a
#define __INDICE(a, b)              b

#define PORT1(a)                    PORTAbits.RA ## a
#define PORT2(a)                    PORTBbits.RB ## a
#define PORT3(a)                    PORTCbits.RC ## a
#define PORT4(a)                    PORTDbits.RD ## a
#define PORT5(a)                    PORTEbits.RE ## a
#define PORT6(a)                    PORTFbits.RF ## a
#define PORT7(a)                    PORTGbits.RG ## a

#define TRIS1(a)                    TRISAbits.TRISA ## a
#define TRIS2(a)                    TRISBbits.TRISB ## a
#define TRIS3(a)                    TRISCbits.TRISC ## a
#define TRIS4(a)                    TRISDbits.TRISD ## a
#define TRIS5(a)                    TRISEbits.TRISE ## a
#define TRIS6(a)                    TRISFbits.TRISF ## a
#define TRIS7(a)                    TRISGbits.TRISG ## a

#define LAT1(a)                     LATAbits.LATA ## a
#define LAT2(a)                     LATBbits.LATB ## a
#define LAT3(a)                     LATCbits.LATC ## a
#define LAT4(a)                     LATDbits.LATD ## a
#define LAT5(a)                     LATEbits.LATE ## a
#define LAT6(a)                     LATFbits.LATF ## a
#define LAT7(a)                     LATGbits.LATG ## a

#define __0                         0, 0
// PORT A
#define __PA0                       1, 0
#define __PA1                       1, 1
#define __PA2                       1, 2
#define __PA3                       1, 3
#define __PA4                       1, 4
#define __PA5                       1, 5
#define __PA6                       1, 6
#define __PA7                       1, 7
#define __PA8                       1, 8
#define __PA9                       1, 9
#define __PA10                      1, 10
#define __PA11                      1, 11
#define __PA12                      1, 12
#define __PA13                      1, 13
#define __PA14                      1, 14
#define __PA15                      1, 15
// PORT B
#define __PB0                       2, 0
#define __PB1                       2, 1
#define __PB2                       2, 2
#define __PB3                       2, 3
#define __PB4                       2, 4
#define __PB5                       2, 5
#define __PB6                       2, 6
#define __PB7                       2, 7
#define __PB8                       2, 8
#define __PB9                       2, 9
#define __PB10                      2, 10
#define __PB11                      2, 11
#define __PB12                      2, 12
#define __PB13                      2, 13
#define __PB14                      2, 14
#define __PB15                      2, 15
// PORT C
#define __PC0                       3, 0
#define __PC1                       3, 1
#define __PC2                       3, 2
#define __PC3                       3, 3
#define __PC4                       3, 4
#define __PC5                       3, 5
#define __PC6                       3, 6
#define __PC7                       3, 7
#define __PC8                       3, 8
#define __PC9                       3, 9
#define __PC10                      3, 10
#define __PC11                      3, 11
#define __PC12                      3, 12
#define __PC13                      3, 13
#define __PC14                      3, 14
#define __PC15                      3, 15
// PORT D
#define __PD0                       4, 0
#define __PD1                       4, 1
#define __PD2                       4, 2
#define __PD3                       4, 3
#define __PD4                       4, 4
#define __PD5                       4, 5
#define __PD6                       4, 6
#define __PD7                       4, 7
#define __PD8                       4, 8
#define __PD9                       4, 9
#define __PD10                      4, 10
#define __PD11                      4, 11
#define __PD12                      4, 12
#define __PD13                      4, 13
#define __PD14                      4, 14
#define __PD15                      4, 15
// PORT E
#define __PE0                       5, 0
#define __PE1                       5, 1
#define __PE2                       5, 2
#define __PE3                       5, 3
#define __PE4                       5, 4
#define __PE5                       5, 5
#define __PE6                       5, 6
#define __PE7                       5, 7
#define __PE8                       5, 8
#define __PE9                       5, 9
#define __PE10                      5, 10
#define __PE11                      5, 11
#define __PE12                      5, 12
#define __PE13                      5, 13
#define __PE14                      5, 14
#define __PE15                      5, 15
// PORT F
#define __PF0                       6, 0
#define __PF1                       6, 1
#define __PF2                       6, 2
#define __PF3                       6, 3
#define __PF4                       6, 4
#define __PF5                       6, 5
#define __PF6                       6, 6
#define __PF7                       6, 7
#define __PF8                       6, 8
#define __PF9                       6, 9
#define __PF10                      6, 10
#define __PF11                      6, 11
#define __PF12                      6, 12
#define __PF13                      6, 13
#define __PF14                      6, 14
#define __PF15                      6, 15
// PORT G
#define __PG0                       7, 0
#define __PG1                       7, 1
#define __PG2                       7, 2
#define __PG3                       7, 3
#define __PG4                       7, 4
#define __PG5                       7, 5
#define __PG6                       7, 6
#define __PG7                       7, 7
#define __PG8                       7, 8
#define __PG9                       7, 9
#define __PG10                      7, 10
#define __PG11                      7, 11
#define __PG12                      7, 12
#define __PG13                      7, 13
#define __PG14                      7, 14
#define __PG15                      7, 15

typedef enum
{
    ACTIVE_LOW = 0,
    ACTIVE_HIGH
} _IO_ACTIVE_STATE;

typedef struct
{
    uint8_t             _port;
    uint8_t             _indice;
} _io_t;

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
} ports_registers_t;

typedef void (*ports_event_handler_t)();

void ports_change_notice_init(uint32_t cn_pull_up, uint32_t cn_pins_enable, ports_event_handler_t evt_handler);
void ports_reset_all_pins_input();
void ports_reset_pin_input(_io_t io);
void ports_reset_pin_output(_io_t io);
bool ports_get_bit(_io_t io);
void ports_set_bit(_io_t io);
void ports_clr_bit(_io_t io);
void ports_toggle_bit(_io_t io);
void ports_interrupt_handler();

#endif
