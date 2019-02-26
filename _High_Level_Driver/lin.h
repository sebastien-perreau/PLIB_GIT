#ifndef __DEF_LIN
#define	__DEF_LIN

// ----------------------------------------
// ---------- Defines of typedef ----------
// ----------------------------------------
typedef enum
{
    LIN1,
    LIN2,
    LIN3,
    LIN4,
    LIN5,
    LIN6,
    LIN_NUMBER_OF_MODULES
}LIN_MODULE;

typedef enum
{
    LIN_DEVICE_ENABLE = 0,
    LIN_DEVICE_DISABLE
}LIN_ENABLE;

typedef enum
{
    LIN_VERSION_1_3 = 0,
    LIN_VERSION_2_0,
    LIN_VERSION_2_1
}LIN_VERSION;

typedef struct
{
    BOOL busy;                      // Ready for a new transmission.
    BOOL sleep;                     // Bus is in sleep mode.
    BOOL readyForNextTransmission;  // Bus in wainting for the next transmission.
}LIN_STATUS_BITS;

typedef struct
{
    BOOL requestType;               // Tx or Rx request.
    BOOL requestReadBack;           // Need to compare the previous data sent with the data return by LIN tranceiver.
}LIN_STATE_BITS;

typedef struct
{
    BOOL readBackBit;               // Error detected after readBack.
    BOOL cheksumBit;                // Error between the chksm receive and the chksm calculated with data received.
}LIN_ERROR_BITS;

typedef struct
{
    BYTE id;
    BYTE dlc;
    BYTE data[8];
}LIN_FRAME;

typedef struct
{
    BYTE version;                   // Version of standard lin used (1.3/2.0 or 2.1).
    BYTE stateMachine;              // break, sync, id, data, chksm.
    BYTE readBack;

    LIN_STATUS_BITS statusBits;
    LIN_STATE_BITS stateBits;
    LIN_ERROR_BITS errorBits;
    
    LIN_FRAME frame;
    WORD cheksum;
    WORD frameTime;
    WORD busTime;

}LIN_REGISTERS;

#define LIN_MESSAGE_WAKE_UP             0
#define LIN_MESSAGE_HEADER_BREAK        1
#define LIN_MESSAGE_HEADER_SYNC         2
#define LIN_MESSAGE_HEADER_ID           3
#define LIN_MESSAGE_TX_DATA             4
#define LIN_MESSAGE_RX_DATA             5
#define LIN_MESSAGE_TX_CHEKSUM          6
#define LIN_MESSAGE_RX_CHEKSUM          7
#define LIN_MESSAGE_DEFAULT             10

#define LIN_TRANSMISSION_REQUEST        0
#define LIN_RECEPTION_REQUEST           1

#define LIN_BUS_ACTIVITY                1300
#define TIME_BEFORE_NEXT_TRANSMISSION   TICK_3MS

#endif

void LINInit(UART_MODULE id, BYTE version);
BYTE LINSetIdWithParity(BYTE id);
LIN_STATUS_BITS *LINGetStatusBitsAdress(UART_MODULE mUartModule);
LIN_STATE_BITS *LINGetStateBitsAdress(UART_MODULE mUartModule);
LIN_ERROR_BITS *LINGetErrorBitsAdress(UART_MODULE mUartModule);
LIN_FRAME *LINGetFrameAdress(UART_MODULE mUartModule);
BYTE LINGetVersion(UART_MODULE mUartModule);
WORD LINGetCheksum(UART_MODULE mUartModule);
void LINCleanup(UART_MODULE mUartModule);
void LINFlush(UART_MODULE mUartModule, BOOL requestType);
void LINTimeUpdate(UART_MODULE mUartModule);
void LINDeamonMaster(UART_MODULE mUartModule, BYTE UartDataReceive);
BOOL LINIsCheksumCorrect(UART_MODULE mUartModule);
