#ifndef __DEF_DEFINES
#define __DEF_DEFINES

    #define PERIPHERAL_FREQ             (80000000L)

    #define SWITCH1_PULLUP_ENABLE       CN15_PULLUP_ENABLE
    #define SWITCH2_PULLUP_ENABLE       CN16_PULLUP_ENABLE
    #define SWITCH3_PULLUP_ENABLE       CN19_PULLUP_ENABLE

    #define SWITCH1                     __PD6
    #define SWITCH2                     __PD7
    #define SWITCH3                     __PD13
	
    #define LED1                        __PD0   // LED1 is used for SDO1
    #define LED2                        __PD1
    #define LED3                        __PD2

    #define ADC_MUX                     AN15
    #define MUX0                        __PD5
    #define MUX1                        __PD9

    #define CAN1_ENABLE_PIN             __PC1
    #define CAN2_ENABLE_PIN             __PC2
    #define LIN_ENABLE_PIN              __PC13

    // level: OFF/ON/BLINK/BLINK_INV
    #define mToggleLedStatusD2()        INV_BIT(_ledStatus, 2)
    #define mToggleLedStatusD3()        INV_BIT(_ledStatus, 0)

    #define mUpdateLedStatusD2(level)   _ledStatus = ((_ledStatus & 0xf3) | ((level & 0x03) << 2))
    #define mUpdateLedStatusD3(level)   _ledStatus = ((_ledStatus & 0xfc) | ((level & 0x03) << 0))

    #define m_init_hardware_picadapter()    {                                   \
                                                mInitIOAsOutput(LED2);          \
                                                mInitIOAsOutput(LED3);          \
                                                mInitIOAsInput(SWITCH1);        \
                                                mInitIOAsInput(SWITCH2);        \
                                                mInitIOAsInput(SWITCH3);        \
                                                mInitIOAsOutput(MUX0);          \
                                                mInitIOAsOutput(MUX1);          \
                                            }

    #define PTR_BASE                    unsigned long
	
    #define bool                        BOOL
    #define true                        TRUE
    #define false                       FALSE
    #define BIT_IN                      (1)
    #define BIT_OUT                     (0)
    #define ON                          (1)
    #define OFF                         (0)
    #define ENABLE                      (1)
    #define DISABLE                     (0)
    #define ALL                         (0xFFFFFFFF)
    #define ALL_ON                      (0xFFFFFFFF)
    #define ALL_OFF                     (0x00000000)
    #define BLINK                       (2)
    #define BLINK_INV                   (3)

    #define SET_BIT(p, n)               ((p) |= ((1) << (n)))
    #define CLR_BIT(p, n)               ((p) &= ~((1) << (n)))
    #define INV_BIT(p, n)               ((p) ^= ((1) << (n)))
    #define GET_BIT(p, n)               (((p) >> (n)) & 0x01)

    #define swap_word(val)              (((val&0xff00)>>8)|((val&0x00ff)<<8))
    #define swap_dword(val)             (((val&0xff000000)>>24)|((val&0x00ff0000)>>8)|((val&0x0000ff00)<<8)|((val&0x000000ff)<<24))

    #define CONCAT_2(p1, p2)            CONCAT_2_(p1, p2)
    #define CONCAT_2_(p1, p2)           p1##p2

    #define COUNT_ARGUMENTS(...)        COUNT_ARGUMENTS_(__VA_ARGS__,PP_RSEQ_N())
    #define COUNT_ARGUMENTS_(...)       PP_ARG_N(__VA_ARGS__)
    #define PP_ARG_N(       _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
                            _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
                            _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
                            _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
                            _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
                            _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
                            _61,_62,_63,N,...) N
    #define PP_RSEQ_N()     63,62,61,60,                   \
                            59,58,57,56,55,54,53,52,51,50, \
                            49,48,47,46,45,44,43,42,41,40, \
                            39,38,37,36,35,34,33,32,31,30, \
                            29,28,27,26,25,24,23,22,21,20, \
                            19,18,17,16,15,14,13,12,11,10, \
                            9,8,7,6,5,4,3,2,1,0

    // Generic Structure
    typedef struct
    {
        uint8_t     index;
        uint64_t    tick;
    } state_machine_t;
    
    typedef struct
    {
        bool        *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_BOOL;
    
    typedef struct
    {
        uint8_t     *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_BYTE;
    
    typedef struct
    {
        uint16_t    *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_WORD;
    
    typedef struct
    {
        uint32_t    *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_DWORD;
    
    typedef struct
    {
        uint64_t    *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_QWORD;
    
    typedef struct
    {
        float       *p;
        uint16_t    size;
        uint16_t    index;
    } DYNAMIC_TAB_FLOAT;
	
#endif
