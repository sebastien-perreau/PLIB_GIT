/*********************************************************************
*	TIMER modules (1, 2, 3, 4 et 5)
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               15/11/2013              - Initial release
*********************************************************************/

#include "../PLIB.h"

static LIN_REGISTERS linRegs[LIN_NUMBER_OF_MODULES];
static QWORD tickLin[LIN_NUMBER_OF_MODULES];

static void lin_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_UART_ERROR:
            
            break;
            
        case IRQ_UART_RX:
            
            linRegs[id].busTime = LIN_BUS_ACTIVITY;
            if(linRegs[id].statusBits.busy)
            {
                if(linRegs[id].stateBits.requestReadBack)
                {
                    if(data != linRegs[id].readBack)
                    {
                        linRegs[id].errorBits.readBackBit = 1;
                    }
                    linRegs[id].stateBits.requestReadBack = 0;
                }
                LINDeamonMaster(id, data);
            }
            break;
            
        case IRQ_UART_TX:
            
            break;
            
    }
}

static void lin_timing_event_handler(uint8_t id)
{
    if (id == TIMER4)
    {
        LINTimeUpdate(LIN2);
        LINTimeUpdate(LIN5);
    }
}

void LINInit(UART_MODULE id, BYTE version)
{
    uart_init(id, lin_event_handler, UART_BAUDRATE_19200, UART_STD_PARAMS);
    timer_init_2345_us(TIMER4, lin_timing_event_handler, TMR_ON | TMR_SOURCE_INT | TMR_IDLE_CON | TMR_GATE_OFF, 1000);
    
    IRQInit(IRQ_U1E + id, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
    IRQInit(IRQ_U1RX + id, IRQ_ENABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
    IRQInit(IRQ_U1TX + id, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
    
    // The LIN driver use the TIMER 4 for timings. Enable the T4 interruption.
    IRQInit(IRQ_T4, IRQ_ENABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
    
    // Status
    linRegs[id].statusBits.busy = 0;
    linRegs[id].statusBits.sleep = 0;
    linRegs[id].statusBits.readyForNextTransmission = 1;
    // State
    linRegs[id].stateBits.requestType = LIN_TRANSMISSION_REQUEST;
    linRegs[id].stateBits.requestReadBack = 0;
    // Error
    linRegs[id].errorBits.readBackBit = 0;
    linRegs[id].errorBits.cheksumBit = 0;
    // Frame & State machine
    linRegs[id].version = version;
    linRegs[id].stateMachine = LIN_MESSAGE_HEADER_BREAK;
    linRegs[id].readBack = 0x00;
    linRegs[id].frame.id = 0x00;
    linRegs[id].frame.dlc = 0x00;
    linRegs[id].frame.data[0] = 0x00;
    linRegs[id].frame.data[1] = 0x00;
    linRegs[id].frame.data[2] = 0x00;
    linRegs[id].frame.data[3] = 0x00;
    linRegs[id].frame.data[4] = 0x00;
    linRegs[id].frame.data[5] = 0x00;
    linRegs[id].frame.data[6] = 0x00;
    linRegs[id].frame.data[7] = 0x00;
    // Cheksum & LIN Time
    linRegs[id].cheksum = 0x0000;
    linRegs[id].frameTime = 0x0000;
    linRegs[id].busTime = 0x0000;
}

BYTE LINSetIdWithParity(BYTE id)
{
    return ((id&0x3F) | (((((id>>0)&0x01)^((id>>1)&0x01)^((id>>2)&0x01)^((id>>4)&0x01))&0x01)<<6) | ((!(((id>>1)&0x01)^((id>>3)&0x01)^((id>>4)&0x01)^((id>>5)&0x01))&0x01)<<7));
}

LIN_STATUS_BITS *LINGetStatusBitsAdress(UART_MODULE mUartModule)
{
    return &linRegs[mUartModule].statusBits;
}

LIN_STATE_BITS *LINGetStateBitsAdress(UART_MODULE mUartModule)
{
    return &linRegs[mUartModule].stateBits;
}

LIN_ERROR_BITS *LINGetErrorBitsAdress(UART_MODULE mUartModule)
{
    return &linRegs[mUartModule].errorBits;
}

LIN_FRAME *LINGetFrameAdress(UART_MODULE mUartModule)
{
    return &linRegs[mUartModule].frame;
}

BYTE LINGetVersion(UART_MODULE mUartModule)
{
    return linRegs[mUartModule].version;
}

WORD LINGetCheksum(UART_MODULE mUartModule)
{
    return linRegs[mUartModule].cheksum;
}

void LINCleanup(UART_MODULE mUartModule)
{
    // Clear bits state.
    linRegs[mUartModule].stateBits.requestReadBack = 0;
    linRegs[mUartModule].stateBits.requestType = 0;
    // Clear bits error.
    linRegs[mUartModule].errorBits.readBackBit = 0;
    linRegs[mUartModule].errorBits.cheksumBit = 0;
    // Clear busy status bit.
    linRegs[mUartModule].statusBits.busy = 0;
    // Reset state machine.
    linRegs[mUartModule].stateMachine = LIN_MESSAGE_HEADER_BREAK;
}

void LINFlush(UART_MODULE mUartModule, BOOL requestType)
{
    linRegs[mUartModule].stateBits.requestType = requestType;
    linRegs[mUartModule].frameTime = (WORD) (((1.4*(10*(linRegs[mUartModule].frame.dlc + 1)) + 34) / 20) + 1);
    if(linRegs[mUartModule].statusBits.sleep)
    {
        linRegs[mUartModule].stateMachine = LIN_MESSAGE_WAKE_UP;
    }
    linRegs[mUartModule].statusBits.busy = 1;
    linRegs[mUartModule].statusBits.readyForNextTransmission = 0;
    LINDeamonMaster(mUartModule, 0x00);
}

void LINTimeUpdate(UART_MODULE mUartModule)
{
    if(!linRegs[mUartModule].statusBits.busy)
    {
        if(linRegs[mUartModule].busTime-- == 0)
        {
            linRegs[mUartModule].busTime = 0;
            linRegs[mUartModule].statusBits.sleep = 1;
        }
    }
    else
    {
        if(!linRegs[mUartModule].statusBits.sleep)
        {
            if(linRegs[mUartModule].frameTime-- == 0)
            {
                linRegs[mUartModule].frameTime = 0;
                LINCleanup(mUartModule);
            }
        }
    }
    
    if((mTickCompare(tickLin[mUartModule]) >= TIME_BEFORE_NEXT_TRANSMISSION) && !linRegs[mUartModule].statusBits.busy)
    {
        linRegs[mUartModule].statusBits.readyForNextTransmission = 1;
    }
}

void LINDeamonMaster(UART_MODULE mUartModule, BYTE UartDataReceive)
{
    static BYTE indData = 0;

    switch(linRegs[mUartModule].stateMachine)
    {
        case LIN_MESSAGE_WAKE_UP:
            uart_set_baudrate(mUartModule, UART_BAUDRATE_600);
            while (!uart_send_data(mUartModule, 0xfc))
            {
                linRegs[mUartModule].readBack = 0xfc;
                linRegs[mUartModule].stateBits.requestReadBack = true;
                linRegs[mUartModule].stateMachine = LIN_MESSAGE_HEADER_BREAK;
            }
            break;
        case LIN_MESSAGE_HEADER_BREAK:
            linRegs[mUartModule].statusBits.sleep = 0;
            uart_set_baudrate(mUartModule, ((UART_BAUDRATE_19200 << 1)/3));
            while (!uart_send_data(mUartModule, 0x00))
            {
                linRegs[mUartModule].readBack = 0x00;
                linRegs[mUartModule].stateBits.requestReadBack = true;
                linRegs[mUartModule].stateMachine = LIN_MESSAGE_HEADER_SYNC;
            }
            break;
        case LIN_MESSAGE_HEADER_SYNC:
            uart_set_baudrate(mUartModule, UART_BAUDRATE_19200);
            while (!uart_send_data(mUartModule, 0x55))
            {
                linRegs[mUartModule].readBack = 0x55;
                linRegs[mUartModule].stateBits.requestReadBack = true;
                linRegs[mUartModule].stateMachine = LIN_MESSAGE_HEADER_ID;
            }
            break;
        case LIN_MESSAGE_HEADER_ID:
            while (!uart_send_data(mUartModule, linRegs[mUartModule].frame.id))
            {
                linRegs[mUartModule].readBack = linRegs[mUartModule].frame.id;
                if(linRegs[mUartModule].version == LIN_VERSION_1_3)
                {
                    linRegs[mUartModule].cheksum = 0x00;
                }
                else if((linRegs[mUartModule].version == LIN_VERSION_2_0) || (linRegs[mUartModule].version == LIN_VERSION_2_1))
                {
                    linRegs[mUartModule].cheksum = linRegs[mUartModule].frame.id;
                }
                linRegs[mUartModule].stateBits.requestReadBack = true;
                indData = 0;
                if(linRegs[mUartModule].stateBits.requestType == LIN_TRANSMISSION_REQUEST)
                {
                    linRegs[mUartModule].stateMachine = LIN_MESSAGE_TX_DATA;
                }
                else
                {
                    linRegs[mUartModule].stateMachine = LIN_MESSAGE_RX_DATA;
                }
            }
            break;
        case LIN_MESSAGE_TX_DATA:
            while (!uart_send_data(mUartModule, linRegs[mUartModule].frame.data[indData]))
            {
                linRegs[mUartModule].readBack = linRegs[mUartModule].frame.data[indData];
                linRegs[mUartModule].cheksum += linRegs[mUartModule].frame.data[indData];
                if(linRegs[mUartModule].cheksum > 255)
                {
                    linRegs[mUartModule].cheksum -= 255;
                }
                if(++indData == linRegs[mUartModule].frame.dlc)
                {
                    linRegs[mUartModule].stateMachine = LIN_MESSAGE_TX_CHEKSUM;
                    linRegs[mUartModule].cheksum ^= 0xFF;
                }
                linRegs[mUartModule].stateBits.requestReadBack = true;
            }
            break;
        case LIN_MESSAGE_RX_DATA:
            if(++indData == 1)
            {
                // ID receive from UART - Do nothing
            }
            else if((indData >= 2) && (indData <= 9))
            {
                linRegs[mUartModule].frame.data[indData-2] = UartDataReceive;
                linRegs[mUartModule].cheksum += linRegs[mUartModule].frame.data[indData-2];
                if(linRegs[mUartModule].cheksum > 255)
                {
                    linRegs[mUartModule].cheksum -= 255;
                }

                if(indData >= (linRegs[mUartModule].frame.dlc + 1))
                {
                    linRegs[mUartModule].stateMachine = LIN_MESSAGE_RX_CHEKSUM;
                    linRegs[mUartModule].cheksum ^= 0xFF;
                }
            }
            break;
        case LIN_MESSAGE_TX_CHEKSUM:
            while (!uart_send_data(mUartModule, (BYTE)linRegs[mUartModule].cheksum))
            {
                linRegs[mUartModule].readBack = (BYTE)linRegs[mUartModule].cheksum;
                linRegs[mUartModule].stateBits.requestReadBack = true;
                linRegs[mUartModule].stateMachine = LIN_MESSAGE_DEFAULT;
            }
            break;
        case LIN_MESSAGE_RX_CHEKSUM:
            if((BYTE)linRegs[mUartModule].cheksum != UartDataReceive)
            {
                linRegs[mUartModule].errorBits.cheksumBit = 1;
            }
        default:
            tickLin[mUartModule] = mGetTick();
            LINCleanup(mUartModule);
            break;
    }
}
