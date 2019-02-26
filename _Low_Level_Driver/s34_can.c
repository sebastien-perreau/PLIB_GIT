/*********************************************************************
*	CAN1 and CAN2 modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		19/07/2013		- Initial release
*       30/06/2014      - Implementation of automatic algorithm for transmission and reception
*       29/09/2015      - Implementation of filters routines 
*                       - Improvement of the CANTaskTx() routine 
*                       - Sort by increasing period when a frame is added CANAddFrame() -> Necessary for the new CANTaskTx()
*                       - Global refresh in can.h and can.c
*
* 
* FILTERS:
* -------
* By default, filters are inactive. You must create a CAN_FILTERS variable in order to config and use filters.
* The user has 4 MASK and 32 FILTERS at disposal.
* IMPORTANT: The CANDeamonFilter must be called in the main loop !
* Parameters can be modify directly with the CAN_FILTERS variable.
* | Mask bit | Filter bit | ID bit | ACCEPT |
*  -----------------------------------------
* |    0           x          x        YES  |
* |    1           0          0        YES  |
* |    1           0          1        NO   |
* |    1           1          0        NO   |
* |    1           1          1        YES  |
* For example, if the user want to receive all frames present on the bus, then he should use a mask_value = 0 
* and active only one filter assign with this mask:
* filters.mask[CAN_FILTER_MASK0] = 0x00000000;
* filters.enableFilterAndAttachMask[CAN_FILTER0] = CAN_ENABLE_FILTER | CAN_FILTER_MASK0; 
* 
* 
* Example of application in the main program:

    CAN_FRAMES framesTx = INIT_CAN_FRAMES();                // Define Tx variable
    CAN_FRAMES framesRx = INIT_CAN_FRAMES();                // Define Rx variable
    CAN_FILTERS filters = INIT_CAN_FILTERS();               // Define filters variable
    ...
    filters.mask[CAN_FILTER_MASK0] = CAN_DEFAULT_MASK0;     // Param MASK 0
    filters.mask[CAN_FILTER_MASK1] = CAN_DEFAULT_MASK1;     // Param MASK 1
    filters.mask[CAN_FILTER_MASK2] = CAN_DEFAULT_MASK2;     // Param MASK 2
    filters.mask[CAN_FILTER_MASK3] = CAN_DEFAULT_MASK3;     // Param MASK 3
    filters.id[CAN_FILTER0] = 0x300;                        // Use Filter0 with ID=0x300
    filters.id[CAN_FILTER1] = 0x200;                        // Use Filter1 with ID=0x200
    filters.enableFilterAndAttachMask[CAN_FILTER0] = CAN_ENABLE_FILTER | CAN_FILTER_MASK1;  // Assign Filter0 with MASK1 and enable it
    filters.enableFilterAndAttachMask[CAN_FILTER1] = CAN_ENABLE_FILTER | CAN_FILTER_MASK1;  // Assign Filter1 with MASK1 and enable it
    // Filters allow to receive ID (with DEFAULT_MASK1 = 0x1FFFFFFC)
    // 0x300/0x301/0x302/0x303
    // 0x200/0x201/0x202/0x203
    CANSetLinkForFramesReception(CAN1, &framesRx);          // Set a link between the CAN module and the Rx variable
    CANAddFrame(&framesTx, 0x600, CAN_ID_STANDARD, 8, TICK_10MS);
    CANAddFrame(&framesTx, 0x601, CAN_ID_STANDARD, 8, TICK_10MS);
    CANAddFrame(&framesTx, 0x602, CAN_ID_STANDARD, 8, TICK_20MS);
    CANAddFrame(&framesTx, 0x603, CAN_ID_STANDARD, 8, TICK_10MS);

     while(1)
    {
        CAN_FRAME *frame_0x300 = CANGetFrame(framesRx, 0x300);
        
        CANSetData1Byte(&framesTx, 0x600, 0, framesRx.numberOfFrame);
        if(framesRx.numberOfFrame > 0)
        {
            CANSetData1Byte(&framesTx, 0x600, 1, framesRx.ptrFrames[0].period);
        }

        if(framesRx.numberOfFrame == 2)
        {
            filters.id[CAN_FILTER0] = 0x100;
            filters.enableFilterAndAttachMask[CAN_FILTER1] = CAN_DISABLE_FILTER | CAN_FILTER_MASK1; 
        }
  
        if(frame_0x300 != NULL)
        {
            myVar = ((frame_0x300->data[7] >> 3)&0x01);
        }
        else  
        {
            ...
        }

        CANFreedomReceiveMemory(&framesRx, TICK_2S);
        CANTaskTx(CAN1, &framesTx);
        CANDeamonFilters(CAN1, &filters);

    }
*********************************************************************/

#include "../PLIB.h"

CAN_FILTERS currentFilters[CAN_NUM_OF_MODULES] = {INIT_CAN_FILTERS(), INIT_CAN_FILTERS()};
BYTE CANMessageFifoArea[CAN_NUM_OF_MODULES][CAN_SIZE_MESSAGE_FIFO_AREA];
CAN_FRAMES *mCANAdressFramesRx[CAN_NUM_OF_MODULES];
const CAN_REGISTERS * mCANModules[CAN_NUM_OF_MODULES] = 
{
#ifdef CAN1_BASE_ADDRESS
	(CAN_REGISTERS *)CAN1_BASE_ADDRESS
#endif
#ifdef CAN2_BASE_ADDRESS
	,(CAN_REGISTERS *)CAN2_BASE_ADDRESS 
#endif
};
const CAN_FLTCON_BYTES mCANFilterControlMap[CAN_NUM_OF_FILTERS] = 
{
	{CAN_FILTER_CONTROL0,0,0x80 	 },
	{CAN_FILTER_CONTROL0,1,0x8000 	 },
	{CAN_FILTER_CONTROL0,2,0x800000	 },
	{CAN_FILTER_CONTROL0,3,0x80000000},
	{CAN_FILTER_CONTROL1,0,0x80 	 },
	{CAN_FILTER_CONTROL1,1,0x8000 	 },
	{CAN_FILTER_CONTROL1,2,0x800000	 },
	{CAN_FILTER_CONTROL1,3,0x80000000},
	{CAN_FILTER_CONTROL2,0,0x80 	 },
	{CAN_FILTER_CONTROL2,1,0x8000 	 },
	{CAN_FILTER_CONTROL2,2,0x800000	 },
	{CAN_FILTER_CONTROL2,3,0x80000000},
	{CAN_FILTER_CONTROL3,0,0x80 	 },
	{CAN_FILTER_CONTROL3,1,0x8000 	 },
	{CAN_FILTER_CONTROL3,2,0x800000	 },
	{CAN_FILTER_CONTROL3,3,0x80000000},
	{CAN_FILTER_CONTROL4,0,0x80 	 },
	{CAN_FILTER_CONTROL4,1,0x8000 	 },
	{CAN_FILTER_CONTROL4,2,0x800000	 },
	{CAN_FILTER_CONTROL4,3,0x80000000},
	{CAN_FILTER_CONTROL5,0,0x80 	 },
	{CAN_FILTER_CONTROL5,1,0x8000 	 },
	{CAN_FILTER_CONTROL5,2,0x800000	 },
	{CAN_FILTER_CONTROL5,3,0x80000000},
	{CAN_FILTER_CONTROL6,0,0x80 	 },
	{CAN_FILTER_CONTROL6,1,0x8000 	 },
	{CAN_FILTER_CONTROL6,2,0x800000	 },
	{CAN_FILTER_CONTROL6,3,0x80000000},
	{CAN_FILTER_CONTROL7,0,0x80 	 },
	{CAN_FILTER_CONTROL7,1,0x8000 	 },
	{CAN_FILTER_CONTROL7,2,0x800000	 },
	{CAN_FILTER_CONTROL7,3,0x80000000}
};       

/*******************************************************************************
  Function:
    void CANInit(CAN_MODULE module, DWORD busSpeed);

  Description:
    This routine initialise the CAN module. Be careful filters are not initialise
    with this routine (keep default value after POR).

  Parameters:
    module      - Identifies the desired CAN module.

    busSpeed    - Bus baudrate (in bit per seconde)

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANInit(CAN_MODULE module, DWORD busSpeed)
{
    BYTE i = 0;
    CAN_BIT_CONFIG canBitConfig;
    CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];
    
    // Set Configuration mode
    CANSetOperatingMode(module, CAN_CONFIGURATION);
    while(CANGetOperatingMode(module) != CAN_CONFIGURATION);
    
    // Set the Buffer Base Adress to the CAN module     
    canRegisters->CxFIFOBA = KVA_TO_PA(&CANMessageFifoArea[module][0]);

    // Configure Channels (channel_number, number of message...) for TX and RX
    CANConfigureChannelForTx(module, CAN_CHANNEL0, 32, CAN_TX_RTR_DISABLED, CAN_HIGHEST_PRIORITY);  // Utilisation du canal 0 pour l'envoi.
    for(i = 0 ; i < 31 ; i++)
    {
        CANConfigureChannelForRx(module, CAN_CHANNEL1 + i, 5, CAN_RX_FULL_RECEIVE); // Utilisation des canaux 1 à 31 pour la réception.
    }
    
    // Set baudrate
    canBitConfig.phaseSeg2Tq            = PHASE_SEG2;
    canBitConfig.phaseSeg1Tq            = PHASE_SEG1;
    canBitConfig.propagationSegTq       = PROP_SEG;
    canBitConfig.phaseSeg2TimeSelect    = TRUE;
    canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = SJW;
    CANSetSpeed(module, &canBitConfig, PERIPHERAL_FREQ, busSpeed);
    
    // Set Normal mode
    CANSetOperatingMode(module, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(module) != CAN_NORMAL_OPERATION);
    
    // Set Events (by default, all module and channel EVENT are disable)
    CANEnableModuleEvent(module, CAN_RX_EVENT, TRUE);
    CANEnableChannelEvent(module, CAN_CHANNEL0, (CAN_TX_CHANNEL_EMPTY), TRUE);  // On peut vérifier qu'il y a un évènement TX_EMPTY avec CANGetChannelEvent(...) car on l'autorise mais on n'autorise pas le vecteur d'interruption avec CANEnableModuleEvent(module, CAN_RX_EVENT|CAN_TX_EVENT, TRUE);
    for(i = 0 ; i < 31 ; i++)
    {
        CANEnableChannelEvent(module, CAN_CHANNEL1 + i, (CAN_RX_CHANNEL_NOT_EMPTY|CAN_RX_CHANNEL_FULL), TRUE);
    }
}

/*******************************************************************************
  Function:
    static void CANConfigFilter(CAN_MODULE module, CAN_FILTER filter, UINT32 id);

  Description:
    This routine allow the driver to change an ID for a filter n (n=0..31).

  Parameters:
    module      - Identifies the desired CAN module.

    filter      - the desire Filter n (n=0..31).

    id          - the new ID for the selected filter n.

  Returns:

  Example:
    <code>
    </code>
  *****************************************************************************/
static void CANConfigFilter(CAN_MODULE module, CAN_FILTER filter, UINT32 id)
{
    DWORD saveEnableMask;
	UINT sid, eid;
	CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];

    // Save actual state of enable mask
    saveEnableMask = (canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCON & mCANFilterControlMap[filter].fltEnMask);
    // Disable filter
    canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONCLR = mCANFilterControlMap[filter].fltEnMask;    
	id &= 0x1FFFFFFF;
    sid = (id & 0x7FF);
    eid = (id & 0x1FFFF800) >> 11;
    canRegisters->canFilterRegs[filter].CxRXFbits.SID = sid;
    canRegisters->canFilterRegs[filter].CxRXFbits.EID = eid;
    canRegisters->canFilterRegs[filter].CxRXFbits.EXID = 1;
    // Restore enable mask
    if(saveEnableMask > 0)
    {
        canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONSET = mCANFilterControlMap[filter].fltEnMask;  
    }
    else
    {
        canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONCLR = mCANFilterControlMap[filter].fltEnMask;
    }
}

/*******************************************************************************
  Function:
    static void CANConfigMask(CAN_MODULE module, CAN_FILTER_MASK mask, UINT32 maskbits);

  Description:
    This routine allow the driver to change the value of a mask n (n=0..3).

  Parameters:
    module      - Identifies the desired CAN module.

    mask        - the desire Mask n (n=0..3).

    maskbits    - the new value for the selected mask n.

  Returns:

  Example:
    <code>
    </code>
  *****************************************************************************/
static void CANConfigMask(CAN_MODULE module, CAN_FILTER_MASK mask, UINT32 maskbits)
{
	UINT sid, eid;
	CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];
    
    // Set Configuration mode
    CANSetOperatingMode(module, CAN_CONFIGURATION);
    while(CANGetOperatingMode(module) != CAN_CONFIGURATION);
    
	maskbits &= 0x1FFFFFFF;
    sid = (maskbits & 0x7FF);
    eid = (maskbits & 0x1FFFF800) >> 11;
    canRegisters->canFilterMaskRegs[mask].CxRXMbits.SID = sid;
    canRegisters->canFilterMaskRegs[mask].CxRXMbits.EID = eid;
    canRegisters->canFilterMaskRegs[mask].CxRXMbits.MIDE = 0;
    
    // Set Normal mode
    CANSetOperatingMode(module, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(module) != CAN_NORMAL_OPERATION);
}

/*******************************************************************************
  Function:
    static void CANLinkFilterToChannelAndEnable(CAN_MODULE module, CAN_FILTER filter, CAN_FILTER_MASK mask, CAN_CHANNEL channel, BOOL enable);

  Description:
    This routine allow the driver to link a filter with a mask and a channel and enable it. 
    Channel 0 is always use for the data transmission.  (cf. CANInit())
    Channel 1..31 is always use for the data reception.  (cf. CANInit())

  Parameters:
    module      - Identifies the desired CAN module.

    filter      - the desire Filter n (n=0..31).

    mask        - the desire Mask n (n=0..3).

    channel     - the desire Channel n (n=1..31).

    enable      - Enable or disable the selected filter.

  Returns:

  Example:
    <code>
    </code>
  *****************************************************************************/
static void CANLinkFilterToChannelAndEnable(CAN_MODULE module, CAN_FILTER filter, CAN_FILTER_MASK mask, CAN_CHANNEL channel, BOOL enable)
{
	CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];

    // Disable filter
    canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONCLR = mCANFilterControlMap[filter].fltEnMask;    
	canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONbyte[mCANFilterControlMap[filter].byteIndex] = (mask << 5)|channel;
    if(enable)
    {
        canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONSET = mCANFilterControlMap[filter].fltEnMask;
    }
    else
    {
        canRegisters->canFilterControlRegs[mCANFilterControlMap[filter].fltcon].CxFLTCONCLR = mCANFilterControlMap[filter].fltEnMask;
    }
}

/*******************************************************************************
  Function:
    void CANDeamonFilters(CAN_MODULE module, CAN_FILTERS *filters);

  Description:
    This routine is the deamon for the configuratin of the filters and maks.
    This function must be called in the main loop (go inside as much as possible).

  Parameters:
    module      - The desire CAN module.
 
    filters     - The CAN_FILTERS variable containing all dynamic filters, masks and links.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANDeamonFilters(CAN_MODULE module, CAN_FILTERS *filters)
{
    BYTE i = 0;
    
    for(i = 0 ; i < 4 ; i++)
    {
        if(currentFilters[module].mask[i] != filters->mask[i])
        {
            currentFilters[module].mask[i] = filters->mask[i];
            CANConfigMask(module, i, filters->mask[i]);
        }
    }
    
    for(i = 0 ; i < 31 ; i++)
    {
        if(currentFilters[module].id[i] != filters->id[i])
        {
            currentFilters[module].id[i] = filters->id[i];
            CANConfigFilter(module, i, filters->id[i]);
        }
    }
    
    for(i = 0 ; i < 31 ; i++)
    {
        if(currentFilters[module].enableFilterAndAttachMask[i] != filters->enableFilterAndAttachMask[i])
        {
            currentFilters[module].enableFilterAndAttachMask[i] = filters->enableFilterAndAttachMask[i];
            CANLinkFilterToChannelAndEnable(module, i, (filters->enableFilterAndAttachMask[i] >> 0)&0x03, i+1, (filters->enableFilterAndAttachMask[i] >> 7)&0x01);
        }
    }
}

/*******************************************************************************
  Function:
    BOOL CANAddMessageFifoBuffer(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data);

  Description:
    This routine allow the driver to add a message in the FIFO buffer. The adress pointer is
    automatically incremented. The user cannot call this routine directly.

  Parameters:
    module      - Identifies the desired CAN module.

    channel     - Identifies the CAN channel.

    id          - Identifier of message.

    idExtended  - If identfier is extended, then TRUE.

    length      - Number of data byte in the message.

    data        - Pointer of data.

  Returns:
    TRUE        - If Fifo buffer is not full. Message has been add.

    FALSE       - Fifo buffer is full. Message has not been add.

  Example:
    <code>
    </code>
  *****************************************************************************/
static BOOL CANAddMessageFifoBuffer(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data)
{
    BOOL ret;
    DWORD sid = (id&0x000007FF), eid = 0;
    CANTxMessageBuffer* message = (CANTxMessageBuffer*) CANGetTxMessageBuffer(module, channel);
    CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];

    // if message equal null then the channel is full.
    if(message != NULL)
    {
        message->messageWord[0] = 0;
        message->messageWord[1] = 0;
        message->messageWord[2] = 0;
        message->messageWord[3] = 0;

        if(idExtended)
        {
            sid = 0;
            eid = (id&0x3FFFFFFF);
        }

        message->msgSID.SID = sid;
        message->msgEID.EID = eid;
        message->msgEID.IDE = idExtended;
        message->msgEID.RTR = 0;
        message->msgEID.SRR = 1;
        message->msgEID.RB0 = 0;
        message->msgEID.RB1 = 0;
        message->msgEID.DLC = length;

        message->data[0] = (BYTE) data[0];
        message->data[1] = (BYTE) data[1];
        message->data[2] = (BYTE) data[2];
        message->data[3] = (BYTE) data[3];
        message->data[4] = (BYTE) data[4];
        message->data[5] = (BYTE) data[5];
        message->data[6] = (BYTE) data[6];
        message->data[7] = (BYTE) data[7];
        // Update channel
        canRegisters->canFifoRegisters[channel].CxFIFOCONSET = 0x00002000;
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

/*******************************************************************************
  Function:
    BOOL CANSendMessage(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data)

  Description:
    This routine allow the user to add a message in the FIFO buffer and flush all the buffer.

  Parameters:
    module      - Identifies the desired CAN module.

    channel     - Identifies the CAN channel.

    id          - Identifier of message.

    idExtended  - If identfier is extended, then TRUE.

    length      - Number of data byte in the message.

    data        - Pointer of data.

  Returns:
    TRUE        - If message has been add before transmission.

    FALSE       - If message has not been add before transmission.

  Example:
    <code>
    </code>
  *****************************************************************************/
BOOL CANSendMessage(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data)
{
    BOOL ret = CANAddMessageFifoBuffer(module, channel, id, idExtended, length, data);
    CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];
    // TxFlush
    canRegisters->canFifoRegisters[channel].CxFIFOCONSET = 0x00000008;

    return ret;
}

/*******************************************************************************
  Function:
    void CANTaskTx(CAN_MODULE module, CAN_CHANNEL channel, CAN_FRAMES *frame)

  Description:
    This routine is the deamon for the CAN transmission (full management of frames).
    This function must be called in the main loop (go inside as much as possible).

  Parameters:
    module      - Identifies the desired CAN module.

    frames      - The CAN_FRAMES variable containing all dynamic frames.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANTaskTx(CAN_MODULE module, CAN_FRAMES *frames)
{     
    BYTE i = 0;
    CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];
    
    for(i = 0 ; i < frames->numberOfFrame ; i++)
    {
        if((mTickCompare(frames->ptrFrames[i].tick) >= frames->ptrFrames[i].period) && frames->ptrFrames[i].enable)
        {
            frames->ptrFrames[i].tick = mGetTick();
            CANAddMessageFifoBuffer(module, CAN_CHANNEL0, frames->ptrFrames[i].id, frames->ptrFrames[i].idExtended, frames->ptrFrames[i].length, frames->ptrFrames[i].data);    
        } 
    }
    // TxFlush
    canRegisters->canFifoRegisters[CAN_CHANNEL0].CxFIFOCONSET = 0x00000008;
}

/*******************************************************************************
  Function:
    void CANAddFrame(CAN_FRAMES *frames, DWORD id, BOOL idExtended, BYTE length, QWORD period)

  Description:
    This routine allow the user to create a new CAN frame. Each time a frame is
    created (or removed) the RAM memory is dynamically modify. Thus the RAM
    size is never constant and depends of the number of frame (Tx and Rx).

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desier identifier.

    idExtended  - 0: ID STANDAR / 1: ID EXTENDED.

    length      - Size of data array.

    period      - The period of the CAN frame.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANAddFrame(CAN_FRAMES *frames, DWORD id, BOOL idExtended, BYTE length, QWORD period)
{    
    BYTE i = 0;
    BYTE ind = frames->numberOfFrame;
    
    if(CANGetIndiceID(*frames, id) == -1)
    {
        if(frames->numberOfFrame == 0)
        {
            frames->ptrFrames = malloc(sizeof(CAN_FRAME));
        }
        else
        {
            frames->ptrFrames = realloc(frames->ptrFrames, (frames->numberOfFrame + 1)*sizeof(CAN_FRAME));
        }
        
        for(i = 0 ; i < frames->numberOfFrame ; i++)
        {
            if(frames->ptrFrames[i].period > period)
            {
                ind = i;
                break;
            }
        }
        
        if(ind != frames->numberOfFrame)
        {
            for(i = frames->numberOfFrame ; i > ind ; i--)
            {
                memcpy(&frames->ptrFrames[i], &frames->ptrFrames[i-1], sizeof(CAN_FRAME));
            }
        }
        
        frames->ptrFrames[ind].enable = ON;
        frames->ptrFrames[ind].id = id;
        frames->ptrFrames[ind].idExtended = idExtended;
        frames->ptrFrames[ind].length = length;
        frames->ptrFrames[ind].data[0] = 0;
        frames->ptrFrames[ind].data[1] = 0;
        frames->ptrFrames[ind].data[2] = 0;
        frames->ptrFrames[ind].data[3] = 0;
        frames->ptrFrames[ind].data[4] = 0;
        frames->ptrFrames[ind].data[5] = 0;
        frames->ptrFrames[ind].data[6] = 0;
        frames->ptrFrames[ind].data[7] = 0;
        frames->ptrFrames[ind].period = period;
        frames->ptrFrames[ind].tick = TICK_INIT;
        
        frames->numberOfFrame++;
    }
}

/*******************************************************************************
  Function:
    void CANRemoveFrame(CAN_FRAMES *frames, DWORD id)

  Description:
    This routine allow the user to remove a CAN frame. Each time a frame is
    created (or removed) the RAM memory is dynamically modify. Thus the RAM
    size is never constant and depends of the number of frame (Tx and Rx).

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to remove.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANRemoveFrame(CAN_FRAMES *frames, DWORD id)
{
    BYTE i;
    CHAR8 ind = CANGetIndiceID(*frames, id);
    if(ind >= 0)
    {
        for(i = ind ; i < (frames->numberOfFrame - 1) ; i++)
        {
            memcpy(&frames->ptrFrames[i], &frames->ptrFrames[i+1], sizeof(CAN_FRAME));
        }
        frames->numberOfFrame--;
        frames->ptrFrames = realloc(frames->ptrFrames, frames->numberOfFrame*sizeof(CAN_FRAME));
    }
}

/*******************************************************************************
  Function:
    CHAR8 CANGetIndiceID(CAN_FRAMES frames, DWORD id)

  Description:
    This routine return the indice (of an array) corresponding of the desire ID.
    The CAN_FRAMES variable contains an array of CAN_FRAME, each CAN_FRAME as an unique
    identifier but we don't know where this frame is located in memory. This function
    return this information.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to remove.

  Returns:
    CHAR8       - The indice of CAN_FRAME array.

  Example:
    <code>
    </code>
  *****************************************************************************/
CHAR8 CANGetIndiceID(CAN_FRAMES frames, DWORD id)
{
    BYTE i;
    for(i = 0 ; i < frames.numberOfFrame ; i++)
    {
        if(frames.ptrFrames[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

/*******************************************************************************
  Function:
    void CANEnableFrame(CAN_FRAMES *frames, DWORD id, BOOL enable)

  Description:
    This routine allow the user to enable/disable a frame.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to enable/disable.

    enable      - ON: enable / OFF:disable.

  Returns:
    None.

  Example:
    <code>

    CAN_FRAMES framesTx = INIT_CAN_FRAMES();
    ...
    CANEnableFrame(&framesTx, 0x200, OFF);
    ...

    </code>
  *****************************************************************************/
void CANEnableFrame(CAN_FRAMES *frames, DWORD id, BOOL enable)
{
    CHAR8 ind = CANGetIndiceID(*frames, id);
    if(ind >= 0)
    {
        frames->ptrFrames[ind].enable = enable;
    }
}

/*******************************************************************************
  Function:
    void CANSetData(CAN_FRAMES *frames, DWORD id, BYTE data[8])

  Description:
    This routine allow the user to modify ALL the data array of a CAN_FRAME.
    Be careful, all 8 bytes will be modify.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to enable/disable.

    data        - new table of 8 BYTES.

  Returns:
    None.

  Example:
    <code>

    CAN_FRAMES framesTx = INIT_CAN_FRAMES();
    BYTE newDataArray[8] = {...};
    ...
    CANSetData(&framesTx, 0x200, newDataArray);
    ...

    </code>
  *****************************************************************************/
void CANSetData(CAN_FRAMES *frames, DWORD id, BYTE data[8])
{
    CHAR8 ind = CANGetIndiceID(*frames, id);
    if(ind >= 0)
    {
        memcpy(frames->ptrFrames[ind].data, data, 8);
    }
}

/*******************************************************************************
  Function:
    void CANSetData_mask(CAN_FRAMES *frames, DWORD id, BYTE data[8], BYTE mask)

  Description:
    This routine allow the user to modify desired data array of a CAN_FRAME.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to enable/disable.

    data        - New table of 8 BYTES.

    mask        - The mask which valid the data (bit per bit).

  Returns:
    None.

  Example:
    <code>

    CAN_FRAMES framesTx = INIT_CAN_FRAMES();
    BYTE newDataArray[8] = {...};
    ...
    CANSetData_mask(&framesTx, 0x200, newDataArray, 0b00111100); // Only BYTES 2, 3, 4, 5 will me modify in framesTx.
    ...

    </code>
  *****************************************************************************/
void CANSetData_mask(CAN_FRAMES *frames, DWORD id, BYTE data[8], BYTE mask)
{
    CHAR8 ind = CANGetIndiceID(*frames, id);
    if(ind >= 0)
    {
        if((mask >> 0) & 0x01)      {frames->ptrFrames[ind].data[0] = data[0];}
        if((mask >> 1) & 0x01)      {frames->ptrFrames[ind].data[1] = data[1];}
        if((mask >> 2) & 0x01)      {frames->ptrFrames[ind].data[2] = data[2];}
        if((mask >> 3) & 0x01)      {frames->ptrFrames[ind].data[3] = data[3];}
        if((mask >> 4) & 0x01)      {frames->ptrFrames[ind].data[4] = data[4];}
        if((mask >> 5) & 0x01)      {frames->ptrFrames[ind].data[5] = data[5];}
        if((mask >> 6) & 0x01)      {frames->ptrFrames[ind].data[6] = data[6];}
        if((mask >> 7) & 0x01)      {frames->ptrFrames[ind].data[7] = data[7];}
    }
}

/*******************************************************************************
  Function:
    void CANSetData1Byte(CAN_FRAMES *frames, DWORD id, BYTE indiceData, BYTE data)

  Description:
    This routine allow the user to modify only 1 byte of a CAN_FRAME.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The desire identifier to enable/disable.

    indiceData  - Indice of the BYTE array [0..7]

    data        - desire data.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANSetData1Byte(CAN_FRAMES *frames, DWORD id, BYTE indiceData, BYTE data)
{
    CHAR8 ind = CANGetIndiceID(*frames, id);
    if(ind >= 0)
    {
        frames->ptrFrames[ind].data[indiceData] = data;
    }
}

/*******************************************************************************
  Function:
    void CANSetLinkForFramesReception(CAN_MODULE module, CAN_FRAMES *frames)

  Description:
    This routine create a link between the CAN_FRAMES variable (creating by user in the main)
    and the receive functions (not accessible by user). If this link is not realized then
    each receive frames will not be stored in RAM memory (so frames will only be accessible
    in the interrupt handler).
    The CAN_FRAMES variable for reception is also dynamicaly managed. RAM size depends
    of the number of reading CAN frame. A timeout can be setting in order to release memory
    (for frame that are no longer present). Cf. CANFreedomReceiveMemory function.

  Parameters:
    module      - The desire CAN module.

    frames      - The CAN_FRAMES variable containing all dynamic frames.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANSetLinkForFramesReception(CAN_MODULE module, CAN_FRAMES *frames)
{
    mCANAdressFramesRx[module] = frames;
}

/*******************************************************************************
  Function:
    void CANTaskRx(CAN_MODULE module)

  Description:
    This routine is the deamon for the CAN reception (full management of frames).
    This function must be called in the interrupt handler (called eache time a new
    frame is received).

  Parameters:
    module      - The desire CAN module.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANTaskRx(CAN_MODULE module)
{
    CAN_EVENT_CODE eventCode;
    CANRxMessageBuffer* mCANGetMessageBuffer[CAN_NUM_OF_MODULES];
    CAN_REGISTERS * canRegisters = (CAN_REGISTERS *)mCANModules[module];
    
    if((CANGetModuleEvent(module) & CAN_RX_EVENT) != 0)
    {
        eventCode = CANGetPendingEventCode(module);
        switch(eventCode)
        {
            case CAN_NO_EVENT:
                break;
            case CAN_ERROR_EVENT:
                break;
            case CAN_WAKEUP_EVENT:
                break;
            default:
                // Up to 32 Channels Event (cf. CANInit() for configuration of channels)
                mCANGetMessageBuffer[module] = (CANRxMessageBuffer*)CANGetRxMessage(module, eventCode);
                // Update channel            
                canRegisters->canFifoRegisters[eventCode].CxFIFOCONSET = 0x00002000;
                if(mCANAdressFramesRx[module] != NULL)
                {
                    CANAddToReceivedBuffer(module, *mCANGetMessageBuffer[module]);
                }
                break;
        }
    }
}

/*******************************************************************************
  Function:
    static void CANAddToReceivedBuffer(CAN_MODULE module, CANRxMessageBuffer frame)

  Description:
    This routine must not be called by user. It's an internal function used by
    the CAN driver when the CANTaskRx is called.
    When a new frame is detected then it's automaticaly store is the RAM
    memory (dynamicaly) or data is updated if already stored.
    Be careful, the CANSetLinkForFramesReception function must be called
    in order to use the RAM memory storage.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    frame       - The CAN frame received.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
static void CANAddToReceivedBuffer(CAN_MODULE module, CANRxMessageBuffer frame)
{
    CHAR8 ind;
    if(frame.msgEID.IDE)        // Extended ID
    {
        ind = CANGetIndiceID(*mCANAdressFramesRx[module], (DWORD) frame.msgEID.EID);
    }
    else                        // Standar ID
    {
        ind = CANGetIndiceID(*mCANAdressFramesRx[module], (DWORD) frame.msgSID.SID);
    }

    if(ind == -1)               // New frame (not yet stored in receive buffer)
    {
        ind = mCANAdressFramesRx[module]->numberOfFrame;
        if(ind == 0)
        {
            mCANAdressFramesRx[module]->ptrFrames = malloc(sizeof(CAN_FRAME));
        }
        else
        {
            mCANAdressFramesRx[module]->ptrFrames = realloc(mCANAdressFramesRx[module]->ptrFrames, (ind + 1)*sizeof(CAN_FRAME));
        }
        mCANAdressFramesRx[module]->ptrFrames[ind].enable = ON;
        if(frame.msgEID.IDE)    // Extended ID
        {
            mCANAdressFramesRx[module]->ptrFrames[ind].id = frame.msgEID.EID;
        }
        else                    // Standar ID
        {
            mCANAdressFramesRx[module]->ptrFrames[ind].id = frame.msgSID.SID;
        }
        mCANAdressFramesRx[module]->ptrFrames[ind].idExtended = frame.msgEID.IDE;
        mCANAdressFramesRx[module]->numberOfFrame++;
    }

    mCANAdressFramesRx[module]->ptrFrames[ind].length = frame.msgEID.DLC;
    memcpy(mCANAdressFramesRx[module]->ptrFrames[ind].data, frame.data, 8);
    mCANAdressFramesRx[module]->ptrFrames[ind].period = mGetTick() - mCANAdressFramesRx[module]->ptrFrames[ind].tick;
    mCANAdressFramesRx[module]->ptrFrames[ind].tick = mGetTick();
}

/*******************************************************************************
  Function:
    void CANFreedomReceiveMemory(CAN_FRAMES *frame, QWORD timeout)

  Description:
    This routine allow the program to realease RAM memory when a frame is no longer
    present on the BUS. This timeout is define by the user.
    We are doing that because the CAN_FRAMES variable for reception is dynamicaly managed.
    RAM size depends of the number of reading CAN frame.

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    timeout     - The desire timeout.

  Returns:
    None.

  Example:
    <code>
    </code>
  *****************************************************************************/
void CANFreedomReceiveMemory(CAN_FRAMES *frames, QWORD timeout)
{
    static BYTE i = 0, j = 0;

    if(frames->numberOfFrame > 0)
    {
        if(mTickCompare(frames->ptrFrames[i].tick) > timeout)
        {
            for(j = i ; j < (frames->numberOfFrame - 1) ; j++)
            {
                memcpy(&frames->ptrFrames[j], &frames->ptrFrames[j+1], sizeof(CAN_FRAME));
            }
            frames->numberOfFrame--;
            frames->ptrFrames = realloc(frames->ptrFrames, frames->numberOfFrame*sizeof(CAN_FRAME));
        }
        if(++i >= frames->numberOfFrame)
        {
            i = 0;
        }
    }
}

/*******************************************************************************
  Function:
    CAN_FRAME* CANGetFrame(CAN_FRAMES frame, DWORD id)

  Description:
    This routine allow the user to get the content of a frame (define by the
    ID in parameter).

  Parameters:
    frames      - The CAN_FRAMES variable containing all dynamic frames.

    id          - The identifier of the receive frame.

  Returns:
    A pointer on the content of the receive frame (if existing) otherwhise 
    the function will return NULL.

  Example:
    <code>
    cf. can.c header
    </code>
  *****************************************************************************/
CAN_FRAME* CANGetFrame(CAN_FRAMES frame, DWORD id)
{
    CHAR8 indId = CANGetIndiceID(frame, id);
    
    if(indId != -1)
    {
        return (CAN_FRAME*) (&frame.ptrFrames[indId]);
    }
    else
    {
        return NULL;
    }
}
