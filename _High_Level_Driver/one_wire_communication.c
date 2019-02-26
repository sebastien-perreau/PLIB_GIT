/*********************************************************************
*	Utilities communication
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		05/12/2013		- Initial release
*               10/12/2013              - Modification of fCommunicationDecoding function.
*                                         Add a define "DECODING_BITRATE" in order to freeze the bitrate.
*                                         Modification of bit calculation in order to have the best detection without recovery.
*
*   Frame details:
*   -------------
*
*   |SYNC|LENGTH|DATA1..DATA15|CKSM|END|
*
*   SYNC:       bit at '1' for synchronization with actualBitrate = 3xbitrate.
*   LENGTH:     5 bits with 4 LSB bits representing the number of data byte and
*               the 5th MSB bit at '1'.
*   DATA1..15:  8 bits representing the data.
*   CKSM:       8 bits representing the cheksum. The cheksum is calculated a
*               follows: ~(0x90 + length + data1 + .. + data15).
*   END:        bit at '1' with actualBitrate = bitrate.
*
*********************************************************************/

#include "../PLIB.h"

/*******************************************************************************
  Function:
    void fCommunicationInitSendVariable(ENCODING_CONFIG *var, BYTE config, QWORD bitrate);

  Description:
    This routine initialize the ENCODING_CONFIG variable used for managing the coding function.

  Parameters:
    *var        - Pointer containing all parameters of the CODING function.

    config      - This variable is using to set some parameters of the ENCODING_CONFIG variable.

    bitrate     - This variable set the bitrate of the encoding frame.

  Returns:


  Example:
    <code>

    ENCODING_CONFIG var_out;
    ...
    fCommunicationInitSendVariable(&var_out, IDLE_ACTIVE_LOW|LENGTH_1, TICK_1US*200);

    </code>
  *****************************************************************************/
void fCommunicationInitSendVariable(ENCODING_CONFIG *var, BYTE config, QWORD bitrate)
{
    var->actualBitrate = bitrate;
    var->bitrate = bitrate;
    var->idleState = config&0x0F;
    var->period = TICK_70MS;
    var->length = (config>>4)&0x0F;
    for(var->bytePointer = 0 ; var->bytePointer < 15 ; var->bytePointer++)
    {
        var->buffer[var->bytePointer] = 0;
    }
    var->bufferTemp = 0;
    var->cheksum = 0;
    var->output = 0;
    var->clock = 0;
    var->state = 0;
    var->dataBit = 0;
    var->stepSendFrame = STEP_SYNCH;
    var->bitPointer = 0;
    var->bytePointer = 0;
    var->tickClock = TICK_INIT;
    var->tickFrame = TICK_INIT;
}

/*******************************************************************************
  Function:
    BOOL fCommunicationEncoding(ENCODING_CONFIG *var);

  Description:
    This routine is used for managing an encoding frame.

  Parameters:
    *var        - Pointer containing all parameters of the CODING function.

  Returns:
    boolean     - This boolean represent the state of the actual bit at the
                  time t.

  Example:
    <code>

    ENCODING_CONFIG var_out;
    ...
    fCommunicationInitSendVariable(&var_out, IDLE_ACTIVE_LOW|LENGTH_1, TICK_1US*200);
    mPORTCSetPinsDigitalOut(BIT_2);

    while(1)
    {
    ...
    LATCbits.LATC2 = fCommunicationEncoding(&var_out);
    ...
    }

    </code>
  *****************************************************************************/
BOOL fCommunicationEncoding(ENCODING_CONFIG *var)
{
    BOOL updateNextDataBit = FALSE;

    if((mGetTick() - var->tickFrame) >= (QWORD) var->period)
    {
        if(((var->state>>STATE_FRAME_PENDING_POSITION)&0x01) == 0)
        {
            var->tickFrame = mGetTick();
            var->state |= STATE_FRAME_PENDING_MASK;
            var->stepSendFrame = STEP_START;
        }
    }

    if((mGetTick() - var->tickClock) >= (QWORD) var->actualBitrate)
    {
        var->tickClock = mGetTick();
        var->clock = !var->clock;
        var->state |= (1<<STATE_HALF_PERIOD_POSITION) | (var->clock<<STATE_FULL_PERIOD_POSITION);
        updateNextDataBit = var->clock;
    }

    if((updateNextDataBit == TRUE) && ((var->state>>STATE_FRAME_PENDING_POSITION)&0x01))
    {
        switch(var->stepSendFrame)
        {
            case STEP_START:
                var->stepSendFrame = STEP_SYNCH;
            case STEP_SYNCH:
                var->actualBitrate = 3*var->bitrate;
                if(var->bitPointer-- >= 0)
                {
                    var->dataBit = 1;
                    break;
                }
                else
                {
                    var->stepSendFrame = STEP_LENGTH; 
                    var->bitPointer = 4;
                    var->bufferTemp = (var->length|0x10);
                }
            case STEP_LENGTH:
                var->actualBitrate = var->bitrate;
                if(var->bitPointer >= 0)
                {
                    var->dataBit = (var->bufferTemp>>(var->bitPointer--))&0x01;
                    break;
                }
                else
                {
                    if(var->length > 0)
                    {
                        var->stepSendFrame = STEP_DATA;
                        var->cheksum = (START_OF_FRAME|var->length);
                    }
                    else
                    {
                        var->stepSendFrame = STEP_CHKSM;
                        var->cheksum = ~(START_OF_FRAME|var->length);
                    }

                    var->bitPointer = 7;
                    var->bytePointer = 0;
                    var->bufferTemp = var->buffer[0];
                }
            case STEP_DATA:
                if(var->bitPointer >= 0)
                {
                    var->dataBit = (var->bufferTemp>>(var->bitPointer--))&0x01;
                    break;
                }
                else
                {
                    var->bitPointer = 7;
                    var->cheksum += var->bufferTemp;
                    if(++var->bytePointer >= var->length)
                    {
                        var->stepSendFrame = STEP_CHKSM;
                    }
                    else
                    {
                        var->bufferTemp = var->buffer[var->bytePointer];
                        var->dataBit = (var->bufferTemp>>(var->bitPointer--))&0x01;
                        break;
                    }
                    var->cheksum = ~(var->cheksum);
                }
            case STEP_CHKSM:
                if(var->bitPointer >= 0)
                {
                    var->dataBit = (var->cheksum>>(var->bitPointer--))&0x01;
                    break;
                }
                else
                {
                    var->stepSendFrame = STEP_END_FRAME;
                    var->bitPointer = 0;
                }
            case STEP_END_FRAME:
                if(var->bitPointer-- >= 0)
                {
                    var->dataBit = 0;
                    break;
                }
                else
                {
                    var->stepSendFrame = STEP_DEFAULT;
                    var->bitPointer = 0;
                    var->state &= ~STATE_FRAME_PENDING_MASK;
                }
            default:
                break;
        }
    }

    if((var->stepSendFrame < STEP_DEFAULT) && (var->stepSendFrame > STEP_START))
    {
        if((var->dataBit) && ((var->state>>STATE_HALF_PERIOD_POSITION)&0x01))
        {
            var->output = !var->output;
            var->state &= ~STATE_HALF_PERIOD_MASK;
        }
        else if(!(var->dataBit) && ((var->state>>STATE_FULL_PERIOD_POSITION)&0x01))
        {
            var->output = !var->output;
            var->state &= ~STATE_FULL_PERIOD_MASK;
        }
    }
    else
    {
        if(var->idleState&IDLE_LOW)
        {
            var->output = 0;
        }
        else if(var->idleState&IDLE_HIGH)
        {
            var->output = 1;
        }
        else if((var->idleState&IDLE_ACTIVE_LOW) && ((var->state>>STATE_FULL_PERIOD_POSITION)&0x01))
        {
            var->output = !var->output;
            var->state &= ~STATE_FULL_PERIOD_MASK;
        }
        else if((var->idleState&IDLE_ACTIVE_HIGH) && ((var->state>>STATE_HALF_PERIOD_POSITION)&0x01))
        {
            var->output = !var->output;
            var->state &= ~STATE_HALF_PERIOD_MASK;
        }
    }

    return (var->output);
}

/*******************************************************************************
  Function:
    void fCommunicationInitReceiveVariable(DECODING_CONFIG *var);

  Description:
    This routine initialize the DECODING_CONFIG variable used for managing the decoding function.

  Parameters:
    *var        - Pointer containing all parameters of the DECODING function.

  Returns:


  Example:
    <code>

    DECODING_CONFIG var_in;
    ...
    fCommunicationInitReceiveVariable(&var_in, TICK_1US*200);

    </code>
  *****************************************************************************/
void fCommunicationInitReceiveVariable(DECODING_CONFIG *var)
{
    var->bitrate = 0;
    var->previousInputState = 0;
    var->numberShortPeriod = 0;
    var->byte = 0;
    var->length = 0;
    for(var->bitPointer = 0 ; var->bitPointer < 15 ; var->bitPointer++)
    {
        var->data[var->bitPointer] = 0;
    }
    var->bitPointer = 0;
    var->stepReceiveFrame = CASE_HEADER;
    var->cheksum = 0;
    var->status = DECODING_FINISHED;
#if (DECODING_BITRATE == 0)
    var->timeDoublePeriod = 0;
    var->timeTriplePeriod = 0;
    var->timeIdle = 0;
#endif
    var->tick = TICK_INIT;
}

/*******************************************************************************
  Function:
    BOOL fCommunicationDecoding(DECODING_CONFIG *var, BOOL input);

  Description:
    This routine is used for managing a decoding frame.

  Parameters:
    *var        - Pointer containing all parameters of the DECODING function.

  Returns:
    boolean     - This boolean represent the state of the actual decoding bit at the
                  time t.

  Example:
    <code>

    DECODING_CONFIG var_in;
    ...
    fCommunicationInitReceiveVariable(&var_in, TICK_1US*200);
    mPORTCSetPinsDigitalIn(BIT_2);
    mPORTCSetPinsDigitalOut(BIT_3);

    while(1)
    {
    ...
    LATCbits.LATC3 = fCommunicationDecoding(&var_in, PORTCbits.RC2);

    if(var_in.status == DECODING_FINISHED)
    {
    ...
    }
    ...
    }

    </code>
  *****************************************************************************/
BOOL fCommunicationDecoding(DECODING_CONFIG *var, BOOL input)
{
   
    if(var->previousInputState != input)
    {
        
        // ----- BITRATE AUTO DETECTION -----
#if (DECODING_BITRATE > 0)
        var->bitrate = DECODING_BITRATE;
#else
        if((mGetTick() - var->tick) < var->timeDoublePeriod)
        {
            var->bitrate = (mGetTick() - var->tick);
            var->timeDoublePeriod = 0;
            var->timeTriplePeriod = 0;
            var->timeIdle = 0;
        }
        else if((mGetTick() - var->tick) < var->timeTriplePeriod)
        {
            var->timeDoublePeriod = (mGetTick() - var->tick);
        }
        else if((mGetTick() - var->tick) <= var->timeIdle)
        {
            var->timeTriplePeriod = (mGetTick() - var->tick);
            var->timeIdle = var->timeTriplePeriod;
        }
        else
        {
            var->timeIdle = (mGetTick() - var->tick);
        }
#endif

        // ----- BIT CALCULATION -----
        if(((mGetTick() - var->tick) >= (var->bitrate/2)) && ((mGetTick() - var->tick) < (3*var->bitrate/2)))
        {
            if(++(var->numberShortPeriod) == 2)
            {
                var->numberShortPeriod = 0;
                var->byte = (var->byte<<1)|0x01;
                var->bitPointer++;
            }
        }
        else if(((mGetTick() - var->tick) >= (3*var->bitrate/2)) && ((mGetTick() - var->tick) < (5*var->bitrate/2)))
        {
            var->numberShortPeriod = 0;
            var->byte = (var->byte<<1)&0xFE;
            var->bitPointer++;
        }
        else if(((mGetTick() - var->tick) >= (5*var->bitrate/2)) && ((mGetTick() - var->tick) < (7*var->bitrate/2)))
        {
            if(++(var->numberShortPeriod) == 2)
            {
                var->numberShortPeriod = 0;
                // HEADER DETECTED
                var->byte = (var->byte<<1)|0x01;
                var->stepReceiveFrame = CASE_LENGTH;
                var->bitPointer = 0;
                var->status = DECODING_PENDING;
            }
        }

        var->previousInputState = input;
        var->tick = mGetTick();
    }
    else if((mGetTick() - var->tick) >= 7*var->bitrate/2)
    {
        var->byte = 0;
        var->bitPointer = 0;
        var->numberShortPeriod = 0;
        var->stepReceiveFrame = CASE_HEADER;
        var->status = DECODING_FINISHED;
    }

    // ----- FRAME RECEPTION -----
    switch(var->stepReceiveFrame)
    {
        case CASE_LENGTH:
            if(var->bitPointer >= 5)
            {
                var->length = var->byte&0x0F;
                var->stepReceiveFrame = CASE_DATA1;
                var->bitPointer = 0;
                var->cheksum = (START_OF_FRAME|var->length);
            }
            break;
        case CASE_DATA1:
        case CASE_DATA2:
        case CASE_DATA3:
        case CASE_DATA4:
        case CASE_DATA5:
        case CASE_DATA6:
        case CASE_DATA7:
        case CASE_DATA8:
        case CASE_DATA9:
        case CASE_DATA10:
        case CASE_DATA11:
        case CASE_DATA12:
        case CASE_DATA13:
        case CASE_DATA14:
        case CASE_DATA15:
            if(var->bitPointer >= 8)
            {
                var->data[var->stepReceiveFrame - 2] = var->byte;
                var->cheksum += var->data[var->stepReceiveFrame - 2];
                if((++var->stepReceiveFrame - 2) >= var->length)
                {
                    var->stepReceiveFrame = CASE_CHKSM;
                }
                var->bitPointer = 0;
            }
            break;
        case CASE_CHKSM:
            if(var->bitPointer >= 8)
            {
                var->cheksum = ~(var->cheksum);
                if(var->byte == var->cheksum)
                {
                    var->status = DECODING_FINISHED;
                }
                else
                {
                    var->status = DECODING_ERROR;
                }
                var->byte = 0;
                var->stepReceiveFrame = CASE_HEADER;
            }
            break;
        default:
            break;
    }

    return (var->byte&0x01);
}
