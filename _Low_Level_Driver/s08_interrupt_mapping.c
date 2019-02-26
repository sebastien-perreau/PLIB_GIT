/*********************************************************************
*	Interrupt module
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               15/11/2013      - Initial release
*               14/11/2018      - Compatibility PLIB
*                               - No dependencies to xc32 library
*                               - Add comments   
*********************************************************************/

#include "../PLIB.h"

const IRQ_REGISTERS IrqTab[] =
{
    {   &IFS0,  &IEC0,  &IPC0,  _IFS0_CTIF_MASK,        _IPC0_CTIS_POSITION,    _IPC0_CTIP_POSITION     },  // Core Timer Interrupt

    {   &IFS0,  &IEC0,  &IPC0,  _IFS0_CS0IF_MASK,       _IPC0_CS0IS_POSITION,   _IPC0_CS0IP_POSITION    },  // Core Software Interrupt 0
    {   &IFS0,  &IEC0,  &IPC0,  _IFS0_CS1IF_MASK,       _IPC0_CS1IS_POSITION,   _IPC0_CS1IP_POSITION    },  // Core Software Interrupt 1

    {   &IFS0,  &IEC0,  &IPC0,  _IFS0_INT0IF_MASK,      _IPC0_INT0IS_POSITION,  _IPC0_INT0IP_POSITION   },  // External Interrupt 0
    {   &IFS0,  &IEC0,  &IPC1,  _IFS0_INT1IF_MASK,      _IPC1_INT1IS_POSITION,  _IPC1_INT1IP_POSITION   },  // External Interrupt 1
    {   &IFS0,  &IEC0,  &IPC2,  _IFS0_INT2IF_MASK,      _IPC2_INT2IS_POSITION,  _IPC2_INT2IP_POSITION   },  // External Interrupt 2
    {   &IFS0,  &IEC0,  &IPC3,  _IFS0_INT3IF_MASK,      _IPC3_INT3IS_POSITION,  _IPC3_INT3IP_POSITION   },  // External Interrupt 3
    {   &IFS0,  &IEC0,  &IPC4,  _IFS0_INT4IF_MASK,      _IPC4_INT4IS_POSITION,  _IPC4_INT4IP_POSITION   },  // External Interrupt 4

    {   &IFS0,  &IEC0,  &IPC1,  _IFS0_T1IF_MASK,        _IPC1_T1IS_POSITION,    _IPC1_T1IP_POSITION     },  // Timer 1
    {   &IFS0,  &IEC0,  &IPC2,  _IFS0_T2IF_MASK,        _IPC2_T2IS_POSITION,    _IPC2_T2IP_POSITION     },  // Timer 2
    {   &IFS0,  &IEC0,  &IPC3,  _IFS0_T3IF_MASK,        _IPC3_T3IS_POSITION,    _IPC3_T3IP_POSITION     },  // TImer 3
    {   &IFS0,  &IEC0,  &IPC4,  _IFS0_T4IF_MASK,        _IPC4_T4IS_POSITION,    _IPC4_T4IP_POSITION     },  // Timer 4
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_T5IF_MASK,        _IPC5_T5IS_POSITION,    _IPC5_T5IP_POSITION     },  // Timer 5

    {   &IFS0,  &IEC0,  &IPC1,  _IFS0_IC1IF_MASK,       _IPC1_IC1IS_POSITION,   _IPC1_IC1IP_POSITION    },  // Input Capture 1
    {   &IFS0,  &IEC0,  &IPC2,  _IFS0_IC2IF_MASK,       _IPC2_IC2IS_POSITION,   _IPC2_IC2IP_POSITION    },  // Input Capture 2
    {   &IFS0,  &IEC0,  &IPC3,  _IFS0_IC3IF_MASK,       _IPC3_IC3IS_POSITION,   _IPC3_IC3IP_POSITION    },  // Input Capture 3
    {   &IFS0,  &IEC0,  &IPC4,  _IFS0_IC4IF_MASK,       _IPC4_IC4IS_POSITION,   _IPC4_IC4IP_POSITION    },  // Input Capture 4
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_IC5IF_MASK,       _IPC5_IC5IS_POSITION,   _IPC5_IC5IP_POSITION    },  // Input Capture 5

    {   &IFS1,  &IEC1,  &IPC1,  _IFS1_IC1EIF_MASK,      _IPC1_IC1IS_POSITION,   _IPC1_IC1IP_POSITION    },  // Input Capture 1 Error
    {   &IFS1,  &IEC1,  &IPC2,  _IFS1_IC2EIF_MASK,      _IPC2_IC2IS_POSITION,   _IPC2_IC2IP_POSITION    },  // Input Capture 2 Error
    {   &IFS1,  &IEC1,  &IPC3,  _IFS1_IC3EIF_MASK,      _IPC3_IC3IS_POSITION,   _IPC3_IC3IP_POSITION    },  // Input Capture 3 Error
    {   &IFS2,  &IEC2,  &IPC4,  _IFS2_IC4EIF_MASK,      _IPC4_IC4IS_POSITION,   _IPC4_IC4IP_POSITION    },  // Input Capture 4 Error
    {   &IFS2,  &IEC2,  &IPC5,  _IFS2_IC5EIF_MASK,      _IPC5_IC5IS_POSITION,   _IPC5_IC5IP_POSITION    },  // Input Capture 5 Error

    {   &IFS0,  &IEC0,  &IPC1,  _IFS0_OC1IF_MASK,       _IPC1_OC1IS_POSITION,   _IPC1_OC1IP_POSITION    },  // Output Capture 1
    {   &IFS0,  &IEC0,  &IPC2,  _IFS0_OC2IF_MASK,       _IPC2_OC2IS_POSITION,   _IPC2_OC2IP_POSITION    },  // Output Capture 2
    {   &IFS0,  &IEC0,  &IPC3,  _IFS0_OC3IF_MASK,       _IPC3_OC3IS_POSITION,   _IPC3_OC3IP_POSITION    },  // Output Capture 3
    {   &IFS0,  &IEC0,  &IPC4,  _IFS0_OC4IF_MASK,       _IPC4_OC4IS_POSITION,   _IPC4_OC4IP_POSITION    },  // Output Capture 4
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_OC5IF_MASK,       _IPC5_OC5IS_POSITION,   _IPC5_OC5IP_POSITION    },  // Output Capture 5

    {   &IFS1,  &IEC1,  &IPC6,  _IFS1_CNIF_MASK,        _IPC6_CNIS_POSITION,    _IPC6_CNIP_POSITION     },  // Input Change

    {   &IFS0,  &IEC0,  &IPC5,  (_IFS0_SPI1EIF_MASK|_IFS0_SPI1TXIF_MASK|_IFS0_SPI1RXIF_MASK),     _IPC5_SPI1IS_POSITION,  _IPC5_SPI1IP_POSITION   },  // SPI 1
    {   &IFS1,  &IEC1,  &IPC7,  (_IFS1_SPI2EIF_MASK|_IFS1_SPI2TXIF_MASK|_IFS1_SPI2RXIF_MASK),     _IPC7_SPI2IS_POSITION,  _IPC7_SPI2IP_POSITION   },  // SPI 2
    {   &IFS0,  &IEC0,  &IPC6,  (_IFS0_SPI3EIF_MASK|_IFS0_SPI3TXIF_MASK|_IFS0_SPI3RXIF_MASK),     _IPC6_SPI3IS_POSITION,  _IPC6_SPI3IP_POSITION   },  // SPI 3
    {   &IFS1,  &IEC1,  &IPC8,  (_IFS1_SPI4EIF_MASK|_IFS1_SPI4TXIF_MASK|_IFS1_SPI4RXIF_MASK),     _IPC8_SPI4IS_POSITION,  _IPC8_SPI4IP_POSITION   },  // SPI 4
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_SPI1EIF_MASK,     _IPC5_SPI1IS_POSITION,  _IPC5_SPI1IP_POSITION   },      // SPI 1 Fault
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_SPI2EIF_MASK,     _IPC7_SPI2IS_POSITION,  _IPC7_SPI2IP_POSITION   },      // SPI 2 Fault
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_SPI3EIF_MASK,     _IPC6_SPI3IS_POSITION,  _IPC6_SPI3IP_POSITION   },      // SPI 3 Fault
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_SPI4EIF_MASK,     _IPC8_SPI4IS_POSITION,  _IPC8_SPI4IP_POSITION   },      // SPI 4 Fault
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_SPI1TXIF_MASK,     _IPC5_SPI1IS_POSITION,  _IPC5_SPI1IP_POSITION   },     // SPI 1 Transfer Done
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_SPI2TXIF_MASK,     _IPC7_SPI2IS_POSITION,  _IPC7_SPI2IP_POSITION   },     // SPI 2 Transfer Done
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_SPI3TXIF_MASK,     _IPC6_SPI3IS_POSITION,  _IPC6_SPI3IP_POSITION   },     // SPI 3 Transfer Done
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_SPI4TXIF_MASK,     _IPC8_SPI4IS_POSITION,  _IPC8_SPI4IP_POSITION   },     // SPI 4 Transfer Done
    {   &IFS0,  &IEC0,  &IPC5,  _IFS0_SPI1RXIF_MASK,     _IPC5_SPI1IS_POSITION,  _IPC5_SPI1IP_POSITION   },     // SPI 1 Receive Done
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_SPI2RXIF_MASK,     _IPC7_SPI2IS_POSITION,  _IPC7_SPI2IP_POSITION   },     // SPI 2 Receive Done
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_SPI3RXIF_MASK,     _IPC6_SPI3IS_POSITION,  _IPC6_SPI3IP_POSITION   },     // SPI 3 Receive Done
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_SPI4RXIF_MASK,     _IPC8_SPI4IS_POSITION,  _IPC8_SPI4IP_POSITION   },     // SPI 4 Receive Done
    
    {   &IFS0,  &IEC0,  &IPC6,  (_IFS0_I2C1BIF_MASK|_IFS0_I2C1SIF_MASK|_IFS0_I2C1MIF_MASK),     _IPC6_I2C1IS_POSITION,  _IPC6_I2C1IP_POSITION   },  // I2C 1
    {   &IFS1,  &IEC1,  &IPC8,  (_IFS1_I2C2BIF_MASK|_IFS1_I2C2SIF_MASK|_IFS1_I2C2MIF_MASK),     _IPC8_I2C2IS_POSITION,  _IPC8_I2C2IP_POSITION   },  // I2C 2
    {   &IFS0,  &IEC0,  &IPC6,  (_IFS0_I2C3BIF_MASK|_IFS0_I2C3SIF_MASK|_IFS0_I2C3MIF_MASK),     _IPC6_I2C3IS_POSITION,  _IPC6_I2C3IP_POSITION   },  // I2C 3
    {   &IFS1,  &IEC1,  &IPC7,  (_IFS1_I2C4BIF_MASK|_IFS1_I2C4SIF_MASK|_IFS1_I2C4MIF_MASK),     _IPC7_I2C4IS_POSITION,  _IPC7_I2C4IP_POSITION   },  // I2C 4
    {   &IFS1,  &IEC1,  &IPC8,  (_IFS1_I2C5BIF_MASK|_IFS1_I2C5SIF_MASK|_IFS1_I2C5MIF_MASK),     _IPC8_I2C5IS_POSITION,  _IPC8_I2C5IP_POSITION   },  // I2C 5
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C1BIF_MASK,     _IPC6_I2C1IS_POSITION,  _IPC6_I2C1IP_POSITION   },      // I2C 1 Bus Collision Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C2BIF_MASK,     _IPC8_I2C2IS_POSITION,  _IPC8_I2C2IP_POSITION   },      // I2C 2 Bus Collision Event
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C3BIF_MASK,     _IPC6_I2C3IS_POSITION,  _IPC6_I2C3IP_POSITION   },      // I2C 3 Bus Collision Event
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_I2C4BIF_MASK,     _IPC7_I2C4IS_POSITION,  _IPC7_I2C4IP_POSITION   },      // I2C 4 Bus Collision Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C5BIF_MASK,     _IPC8_I2C5IS_POSITION,  _IPC8_I2C5IP_POSITION   },      // I2C 5 Bus Collision Event
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C1SIF_MASK,     _IPC6_I2C1IS_POSITION,  _IPC6_I2C1IP_POSITION   },      // I2C 1 Slave Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C2SIF_MASK,     _IPC8_I2C2IS_POSITION,  _IPC8_I2C2IP_POSITION   },      // I2C 2 Slave Event
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C3SIF_MASK,     _IPC6_I2C3IS_POSITION,  _IPC6_I2C3IP_POSITION   },      // I2C 3 Slave Event
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_I2C4SIF_MASK,     _IPC7_I2C4IS_POSITION,  _IPC7_I2C4IP_POSITION   },      // I2C 4 Slave Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C5SIF_MASK,     _IPC8_I2C5IS_POSITION,  _IPC8_I2C5IP_POSITION   },      // I2C 5 Slave Event
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C1MIF_MASK,     _IPC6_I2C1IS_POSITION,  _IPC6_I2C1IP_POSITION   },      // I2C 1 Master Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C2MIF_MASK,     _IPC8_I2C2IS_POSITION,  _IPC8_I2C2IP_POSITION   },      // I2C 2 Master Event
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_I2C3MIF_MASK,     _IPC6_I2C3IS_POSITION,  _IPC6_I2C3IP_POSITION   },      // I2C 3 Master Event
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_I2C4MIF_MASK,     _IPC7_I2C4IS_POSITION,  _IPC7_I2C4IP_POSITION   },      // I2C 4 Master Event
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_I2C5MIF_MASK,     _IPC8_I2C5IS_POSITION,  _IPC8_I2C5IP_POSITION   },      // I2C 5 Master Event

    {   &IFS0,  &IEC0,  &IPC6,  (_IFS0_U1EIF_MASK|_IFS0_U1RXIF_MASK|_IFS0_U1TXIF_MASK),       _IPC6_U1IS_POSITION,    _IPC6_U1IP_POSITION     },  // UART 1
    {   &IFS1,  &IEC1,  &IPC8,  (_IFS1_U2EIF_MASK|_IFS1_U2RXIF_MASK|_IFS1_U2TXIF_MASK),       _IPC8_U2IS_POSITION,    _IPC8_U2IP_POSITION     },  // UART 2
    {   &IFS1,  &IEC1,  &IPC7,  (_IFS1_U3EIF_MASK|_IFS1_U3RXIF_MASK|_IFS1_U3TXIF_MASK),       _IPC7_U3IS_POSITION,    _IPC7_U3IP_POSITION     },  // UART 3
    {   &IFS2,  &IEC2,  &IPC12, (_IFS2_U4EIF_MASK|_IFS2_U4RXIF_MASK|_IFS2_U4TXIF_MASK),       _IPC12_U4IS_POSITION,   _IPC12_U4IP_POSITION    },  // UART 4
    {   &IFS2,  &IEC2,  &IPC12, (_IFS2_U5EIF_MASK|_IFS2_U5RXIF_MASK|_IFS2_U5TXIF_MASK),       _IPC12_U5IS_POSITION,   _IPC12_U5IP_POSITION    },  // UART 5
    {   &IFS2,  &IEC2,  &IPC12, (_IFS2_U6EIF_MASK|_IFS2_U6RXIF_MASK|_IFS2_U6TXIF_MASK),       _IPC12_U6IS_POSITION,   _IPC12_U6IP_POSITION    },  // UART 6
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_U1EIF_MASK,       _IPC6_U1IS_POSITION,    _IPC6_U1IP_POSITION     },  // UART 1 Error
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_U2EIF_MASK,       _IPC8_U2IS_POSITION,    _IPC8_U2IP_POSITION     },  // UART 2 Error
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_U3EIF_MASK,       _IPC7_U3IS_POSITION,    _IPC7_U3IP_POSITION     },  // UART 3 Error
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U4EIF_MASK,       _IPC12_U4IS_POSITION,   _IPC12_U4IP_POSITION    },  // UART 4 Error
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U5EIF_MASK,       _IPC12_U5IS_POSITION,   _IPC12_U5IP_POSITION    },  // UART 5 Error
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U6EIF_MASK,       _IPC12_U6IS_POSITION,   _IPC12_U6IP_POSITION    },  // UART 6 Error
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_U1RXIF_MASK,      _IPC6_U1IS_POSITION,    _IPC6_U1IP_POSITION     },  // UART 1 Receiver
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_U2RXIF_MASK,      _IPC8_U2IS_POSITION,    _IPC8_U2IP_POSITION     },  // UART 2 Receiver
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_U3RXIF_MASK,      _IPC7_U3IS_POSITION,    _IPC7_U3IP_POSITION     },  // UART 3 Receiver
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U4RXIF_MASK,      _IPC12_U4IS_POSITION,   _IPC12_U4IP_POSITION    },  // UART 4 Receiver
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U5RXIF_MASK,      _IPC12_U5IS_POSITION,   _IPC12_U5IP_POSITION    },  // UART 5 Receiver
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U6RXIF_MASK,      _IPC12_U6IS_POSITION,   _IPC12_U6IP_POSITION    },  // UART 6 Receiver
    {   &IFS0,  &IEC0,  &IPC6,  _IFS0_U1TXIF_MASK,      _IPC6_U1IS_POSITION,    _IPC6_U1IP_POSITION     },  // UART 1 Transmitter
    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_U2TXIF_MASK,      _IPC8_U2IS_POSITION,    _IPC8_U2IP_POSITION     },  // UART 2 Transmitter
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_U3TXIF_MASK,      _IPC7_U3IS_POSITION,    _IPC7_U3IP_POSITION     },  // UART 3 Transmitter
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U4TXIF_MASK,      _IPC12_U4IS_POSITION,   _IPC12_U4IP_POSITION    },  // UART 4 Transmitter
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U5TXIF_MASK,      _IPC12_U5IS_POSITION,   _IPC12_U5IP_POSITION    },  // UART 5 Transmitter
    {   &IFS2,  &IEC2,  &IPC12, _IFS2_U6TXIF_MASK,      _IPC12_U6IS_POSITION,   _IPC12_U6IP_POSITION    },  // UART 6 Transmitter

    {   &IFS1,  &IEC1,  &IPC6,  _IFS1_AD1IF_MASK,       _IPC6_AD1IS_POSITION,   _IPC6_AD1IP_POSITION    },  // ADC 1 Convert Done

    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_PMPIF_MASK,       _IPC7_PMPIS_POSITION,   _IPC7_PMPIP_POSITION    },  // Parallel Master Port
    {   &IFS2,  &IEC2,  &IPC7,  _IFS2_PMPEIF_MASK,      _IPC7_PMPIS_POSITION,   _IPC7_PMPIP_POSITION    },  // Parallel Master Port Error

    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_CMP1IF_MASK,      _IPC7_CMP1IS_POSITION,  _IPC7_CMP1IP_POSITION   },  // Comparator 1 Interrupt
    {   &IFS1,  &IEC1,  &IPC7,  _IFS1_CMP2IF_MASK,      _IPC7_CMP2IS_POSITION,  _IPC7_CMP2IP_POSITION   },  // Comparator 2 Interrupt

    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_FSCMIF_MASK,      _IPC8_FSCMIS_POSITION,  _IPC8_FSCMIP_POSITION   },  // Fail-safe Monitor

    {   &IFS1,  &IEC1,  &IPC11, _IFS1_FCEIF_MASK,       _IPC11_FCEIS_POSITION,  _IPC11_FCEIP_POSITION   },  // Flash Control Event

    {   &IFS1,  &IEC1,  &IPC8,  _IFS1_RTCCIF_MASK,      _IPC8_RTCCIS_POSITION,  _IPC8_RTCCIP_POSITION   },  // Real Time Clock

    {   &IFS1,  &IEC1,  &IPC9,  _IFS1_DMA0IF_MASK,      _IPC9_DMA0IS_POSITION,  _IPC9_DMA0IP_POSITION   },  // DMA Channel 0
    {   &IFS1,  &IEC1,  &IPC9,  _IFS1_DMA1IF_MASK,      _IPC9_DMA1IS_POSITION,  _IPC9_DMA1IP_POSITION   },  // DMA Channel 1
    {   &IFS1,  &IEC1,  &IPC9,  _IFS1_DMA2IF_MASK,      _IPC9_DMA2IS_POSITION,  _IPC9_DMA2IP_POSITION   },  // DMA Channel 2
    {   &IFS1,  &IEC1,  &IPC9,  _IFS1_DMA3IF_MASK,      _IPC9_DMA3IS_POSITION,  _IPC9_DMA3IP_POSITION   },  // DMA Channel 3
    {   &IFS1,  &IEC1,  &IPC10,  _IFS1_DMA4IF_MASK,     _IPC10_DMA4IS_POSITION, _IPC10_DMA4IP_POSITION   }, // DMA Channel 4
    {   &IFS1,  &IEC1,  &IPC10,  _IFS1_DMA5IF_MASK,     _IPC10_DMA5IS_POSITION, _IPC10_DMA5IP_POSITION   }, // DMA Channel 5
    {   &IFS1,  &IEC1,  &IPC10,  _IFS1_DMA6IF_MASK,     _IPC10_DMA6IS_POSITION, _IPC10_DMA6IP_POSITION   }, // DMA Channel 6
    {   &IFS1,  &IEC1,  &IPC10,  _IFS1_DMA7IF_MASK,     _IPC10_DMA7IS_POSITION, _IPC10_DMA7IP_POSITION   }, // DMA Channel 7

    {   &IFS1,  &IEC1,  &IPC11,  _IFS1_USBIF_MASK,      _IPC11_USBIS_POSITION,  _IPC11_USBIP_POSITION   },  // USB

    {   &IFS1,  &IEC1,  &IPC11,  _IFS1_CAN1IF_MASK,     _IPC11_CAN1IS_POSITION, _IPC11_CAN1IP_POSITION   }, // CAN1
    {   &IFS1,  &IEC1,  &IPC11,  _IFS1_CAN2IF_MASK,     _IPC11_CAN2IS_POSITION, _IPC11_CAN2IP_POSITION   }, // CAN2
    
    {   &IFS1,  &IEC1,  &IPC12,  _IFS1_ETHIF_MASK,      _IPC12_ETHIS_POSITION,  _IPC12_ETHIP_POSITION   }   // ETHERNET
};

/*******************************************************************************
 * Function: 
 *      void irq_clr_flag(IRQ_SOURCE source)
 * 
 * Description:
 *      This routine is used to clear the flag of a module (IRQ_SOURCE).
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void irq_clr_flag(IRQ_SOURCE source)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    p_irq->IFS[REG_CLR] = p_irq->MASK;
}

/*******************************************************************************
 * Function: 
 *      void irq_set_flag(IRQ_SOURCE source)
 * 
 * Description:
 *      This routine is used to set the flag of a module (IRQ_SOURCE).
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void irq_set_flag(IRQ_SOURCE source)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    p_irq->IFS[REG_SET] = p_irq->MASK;
}

/*******************************************************************************
 * Function: 
 *      void irq_get_flag(IRQ_SOURCE source)
 * 
 * Description:
 *      This routine is used to get the flag of a module (IRQ_SOURCE). The 
 *      position of the flag you want to read depends of the source. You can
 *      use this function like that: if (!irq_get_flag(IRQ_T2)) ...
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 * 
 * Return:
 *      An uint32_t value containing the flag status.
 * 
 * Example:
 *      none
 ******************************************************************************/
uint32_t irq_get_flag(IRQ_SOURCE source)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    return (p_irq->IFS[REG] & p_irq->MASK);
}

/*******************************************************************************
 * Function: 
 *      void irq_enable(IRQ_SOURCE source, bool enable)
 * 
 * Description:
 *      This routine is used to enable/disable the interruption(s) of your module.
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 *      enable: true to enable or false to disable interruption for your module.
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void irq_enable(IRQ_SOURCE source, bool enable)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    if(enable)
    {
        p_irq->IEC[REG_SET] = p_irq->MASK;
    }
    else
    {
        p_irq->IEC[REG_CLR] = p_irq->MASK;
    }
}

/*******************************************************************************
 * Function: 
 *      void irq_set_priority(IRQ_SOURCE source, uint32_t priority)
 * 
 * Description:
 *      This routine is used to set the priority value of your interruption
 *      module. 
 *      Important: the priority and sub-priority should have the same values 
 *      as the Interrupt Handler IPLxAUTO 
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 *      priority: The IRQ_PRIORITY you want (0..7).
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void irq_set_priority(IRQ_SOURCE source, IRQ_PRIORITY priority)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    p_irq->IPC[REG_CLR] = (7 << p_irq->PRI_POS);
    p_irq->IPC[REG_SET] = (priority << p_irq->PRI_POS);
}

/*******************************************************************************
 * Function: 
 *      IRQ_PRIORITY irq_get_priority(IRQ_SOURCE source)
 * 
 * Description:
 *      This routine is used to get the priority value of your interruption
 *      module.
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module.
 * 
 * Return:
 *      It returns the IRQ_PRIORITY value of your module.
 * 
 * Example:
 *      none
 ******************************************************************************/
IRQ_PRIORITY irq_get_priority(IRQ_SOURCE source)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    return ((p_irq->IPC[REG] >> p_irq->PRI_POS) & 7);
}

/*******************************************************************************
 * Function: 
 *      void irq_set_sub_priority(IRQ_SOURCE source, IRQ_SUB_PRIORITY sub_priority)
 * 
 * Description:
 *      This routine is used to set the sub-priority value of your interruption
 *      module. 
 *      Important: the priority and sub-priority should have the same values 
 *      as the Interrupt Handler IPLxAUTO 
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module. 
 *      sub-priority: The IRQ_SUB_PRIORITY you want (0..3).
 * 
 * Return:
 *      none
 * 
 * Example:
 *      none
 ******************************************************************************/
void irq_set_sub_priority(IRQ_SOURCE source, IRQ_SUB_PRIORITY sub_priority)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    p_irq->IPC[REG_CLR] = (3 << p_irq->SUB_PRI_POS);
    p_irq->IPC[REG_SET] = (sub_priority << p_irq->SUB_PRI_POS);
}

/*******************************************************************************
 * Function: 
 *      IRQ_SUB_PRIORITY irq_get_sub_priority(IRQ_SOURCE source)
 * 
 * Description:
 *      This routine is used to get the sub-priority value of your interruption
 *      module.
 * 
 * Parameters:
 *      source: The IRQ_SOURCE of the module.
 * 
 * Return:
 *      It returns the IRQ_SUB_PRIORITY value of your module.
 * 
 * Example:
 *      none
 ******************************************************************************/
IRQ_SUB_PRIORITY irq_get_sub_priority(IRQ_SOURCE source)
{
    IRQ_REGISTERS * p_irq = (IRQ_REGISTERS *)&IrqTab[source];
    return ((p_irq->IPC[REG] >> p_irq->SUB_PRI_POS) & 3);
}
