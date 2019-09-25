/*********************************************************************
 *	DMA modules (0 to 7)
 *	Author : Sébastien PERREAU
 *
 *	Revision history	:
 *               14/03/2019      - Initial release
 ********************************************************************/

#include "../PLIB.h"

extern const DMA_REGISTERS * DmaModule;
const DMA_REGISTERS * DmaModule = (DMA_REGISTERS *)_DMAC_BASE_ADDRESS; 
extern const DMA_CHANNEL_REGISTERS * DmaChannels[];
const DMA_CHANNEL_REGISTERS * DmaChannels[] =
{
    (DMA_CHANNEL_REGISTERS *)_DMAC0_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC1_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC2_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC3_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC4_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC5_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC6_BASE_ADDRESS,
    (DMA_CHANNEL_REGISTERS *)_DMAC7_BASE_ADDRESS
};
static dma_event_handler_t dma_event_handler[DMA_NUMBER_OF_MODULES] = {NULL};
static bool dma_channel_is_using[DMA_NUMBER_OF_MODULES] = {0};

const uint8_t dma_irq[] = 
{
    _DMA0_IRQ,
    _DMA1_IRQ,
    _DMA2_IRQ,
    _DMA3_IRQ,
    _DMA4_IRQ,
    _DMA5_IRQ,
    _DMA6_IRQ,
    _DMA7_IRQ    
};

/*******************************************************************************
  Function:
    void dma_init(  DMA_MODULE id, 
                    dma_event_handler_t evt_handler, 
                    DMA_CHANNEL_CONTROL dma_channel_control,
                    DMA_CHANNEL_INTERRUPT dma_channel_interrupt,
                    DMA_CHANNEL_EVENT dma_channel_event,
                    uint8_t irq_num_tx_start,
                    uint8_t irq_num_tx_abord)

  Description:
    This routine is used to initialize a DMA module.
    
    When using Pattern Match mode and a pattern is detected then a DMA_INT_BLOCK_TRANSFER_DONE
    is set. Interrupt can be handle (if enable) and the DMA channel behavior is the same
    as Block Transfer Complete (DMA_INT_BLOCK_TRANSFER_DONE flag is set and DMA channel is
    disable).
     
    Configure the DMA channel in AUTO_ENABLE mode allow the channel to be always ENABLE even 
    after a BLOCK_TRANSFER_DONE. Thus it is not necessary to re-configure the channel
    with the dma_set_transfer routine. 

  Parameters:
    id                      - The DMA module you want to use.
    evt_handler             - The handler (function) to call when an interruption occurs.
    dma_channel_control     - The DMA Channel Control Register (see DMA_CHANNEL_CONTROL).
    dma_channel_interrupt   - The DMA Channel Interrupt Register (see DMA_CHANNEL_INTERRUPT).
                            It manages the type of interruption and is also used, in this routine,
                            to clear all channel flags.
    dma_channel_event       - The DMA Channel Event Register (see DMA_CHANNEL_EVENT). It is used
                            to setup the type of event for start transfer, abord transfer and/or 
                            pattern match abord.
    irq_num_tx_start        - The IRQ number for start event transfer (e.i: _UART1_TX_IRQ).
    irq_num_tx_abord        - The IRQ number for abord event transfer (e.i: _TIMER_1_IRQ).
  *****************************************************************************/
void dma_init(  DMA_MODULE id, 
                dma_event_handler_t evt_handler, 
                DMA_CHANNEL_CONTROL dma_channel_control,
                DMA_CHANNEL_INTERRUPT dma_channel_interrupt,
                DMA_CHANNEL_EVENT dma_channel_event,
                uint8_t irq_num_tx_start,
                uint8_t irq_num_tx_abord)
{
    DMA_REGISTERS * p_dma = (DMA_REGISTERS *) DmaModule;
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    
    dma_channel_is_using[id] = true;
    
    dma_event_handler[id] = evt_handler;
    irq_init(IRQ_DMA0 + id, (evt_handler != NULL) ? IRQ_ENABLED : IRQ_DISABLED, irq_dma_priority(id));
          
    // Enable the DMA controller
    p_dma->DMACONSET = _DMACON_ON_MASK;
    
    // Abord all current operations on the DMA module.
    dma_abord_transfer(id);
    
    // Set DMA Channel Control Register
    p_dma_channel->DCHCON = (dma_channel_control & ~DMA_CONT_CHANNEL_ENABLE);
    // Set DMA Channel Event Control Register
    p_dma_channel->DCHECON = dma_channel_event | (irq_num_tx_start << _DCH0ECON_CHSIRQ_POSITION) | (irq_num_tx_abord << _DCH0ECON_CHAIRQ_POSITION);
    // Set DMA Channel Interrupt Control Register
    p_dma_channel->DCHINTCLR = DMA_INT_ALL;
    p_dma_channel->DCHINTSET = (dma_channel_interrupt & 0x00ff0000);
}

/*******************************************************************************
  Function:
    DMA_MODULE dma_get_free_channel()

  Description:
    This routine is used to get a free DMA channel. The first channel to be used
    is DMA0 and so on up to channel DMA7.    

  *****************************************************************************/
DMA_MODULE dma_get_free_channel()
{
    DMA_MODULE i;
    for (i = DMA0 ; i < DMA_NUMBER_OF_MODULES ; i++)
    {
        if (!dma_channel_is_using[i])
        {
            dma_channel_is_using[i] = true;
            break;
        }
    }
    return i;
}

/*******************************************************************************
  Function:
    void dma_channel_enable(DMA_MODULE id, bool enable)

  Description:
    This routine is used to enable or disable a DMA channel.

  Parameters:
    id          - The DMA module you want to use.
    enable      - A boolean value (1: enable, 0: disable).
  *****************************************************************************/
void dma_channel_enable(DMA_MODULE id, bool enable)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    (enable) ? (p_dma_channel->DCHCONSET = DMA_CONT_CHANNEL_ENABLE) : (p_dma_channel->DCHCONCLR = DMA_CONT_CHANNEL_ENABLE);
}

/*******************************************************************************
  Function:
    bool dma_channel_is_enable(DMA_MODULE id)

  Description:
    This routine is used to know if the DMA channel is enabled.

  Parameters:
    id          - The DMA module you want to use.
  *****************************************************************************/
bool dma_channel_is_enable(DMA_MODULE id)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    return ((p_dma_channel->DCHCON & DMA_CONT_CHANNEL_ENABLE) > 0) ? 1 : 0;    
}

/*******************************************************************************
  Function:
    void dma_set_channel_event_control(DMA_MODULE id, DMA_CHANNEL_EVENT dma_channel_event)

  Description:
    This routine is used to set the channel event control of a DMA module. This setup is
    first called in the dma_init routine at the initialization but in your program you may
    want to modify this setup at a specific time. You can thus modify this setup whenever 
    you want thanks to this routine.

  Parameters:
    id                      - The DMA module you want to use.
    dma_channel_event       - The DMA Channel Event Register (see DMA_CHANNEL_EVENT). It is used
                            to setup the type of event for start transfer, abord transfer and/or 
                            pattern match abord.
  *****************************************************************************/
void dma_set_channel_event_control(DMA_MODULE id, DMA_CHANNEL_EVENT dma_channel_event)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    
    dma_abord_transfer(id);
    
    p_dma_channel->DCHECONCLR = DMA_EVT_START_TRANSFER_ON_IRQ | DMA_EVT_ABORD_TRANSFER | DMA_EVT_ABORD_TRANSFER_ON_PATTERN_MATCH;
    p_dma_channel->DCHECON |= dma_channel_event;
}

/*******************************************************************************
  Function:
    void dma_set_transfer(DMA_MODULE id, DMA_CHANNEL_TRANSFER * channel_transfer, bool enable_channel, bool force_transfer)

  Description:
    This routine is used to configure a transfer. It setup all pointers, sizes and
    pattern require by the DMA channel for a transfer. It also enable the channel
    (it needs to be disable in order to modify the channel registers) and can
    force the first cell block transfer.
 
    The source and destination pointers can be either RAM content or SFR registers. 
    The sizes source, destination and cell are maximum DMA_MAX_TRANSFER_SIZE.
    The pattern (for pattern match mode) can be either 1 or 2 bytes length (it 
    depends of the configuration in dma_init(... dma_channel_control ...)).

  Parameters:
    id                  - The DMA module you want to use.
    channel_transfer*   - The pointer of DMA_CHANNEL_TRANSFER containing all data 
                        require by the DMA channel to initialize a transfer. 
    enable_channel      - Enable or keep disable the DMA channel after initializing its
                        parameters for the transmission. 
                        For example (DMA RAM to UART Tx) if you enable the DMA channel after
                        the initialization of "set_dma_transfer", the DMA transmission will
                        occurs even if the "force_transfer" is disable.
    force_transfer      - If true then force a start transfer just after initializing 
                        the DMA channel else wait for an event or a manual force 
                        transfer (dma_force_transfer(id)).
  *****************************************************************************/
void dma_set_transfer(DMA_MODULE id, DMA_CHANNEL_TRANSFER * channel_transfer, bool enable_channel, bool force_transfer)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    dma_channel_enable(id, OFF);
    while (dma_channel_is_enable(id));
    p_dma_channel->DCHSSA = _VirtToPhys2(channel_transfer->src_start_addr);
    p_dma_channel->DCHDSA = _VirtToPhys2(channel_transfer->dst_start_addr);
    p_dma_channel->DCHSSIZ = channel_transfer->src_size;
    p_dma_channel->DCHDSIZ = channel_transfer->dst_size;
    p_dma_channel->DCHCSIZ = channel_transfer->cell_size;
    p_dma_channel->DCHDAT = channel_transfer->pattern_data;    
    p_dma_channel->DCHINTCLR = DMA_FLAG_ALL;
    if (enable_channel)
    {
        dma_channel_enable(id, ON);
    }    
    if (force_transfer)
    {        
        dma_force_transfer(id);
    }
}

/*******************************************************************************
  Function:
    uint16_t dma_get_index_cell_pointer(DMA_MODULE id)

  Description:
    This routine is used to get the current index of the cell pointer. When a
    DMA transmission occurs, the data is moved from A to B (A & B are memory area).
    Sometimes you know how many bytes are transfered and sometimes not (for example
    you setup your DMA module to ABORD a transmission on a PATTERN_MATCH). Whit this 
    routine you can get the last index of the DMA module when the ABORD occurs. 
    (Example: You setup your DMA module to receive up to 1000 bytes of a UART module,
    and add an abord condition on a pattern_match - 8 or 16 bits - then if the pattern
    is detected on the UART RX then the abord condition occurs - DMA_INT_BLOCK_TRANSFER_DONE
    flag is set - and the DMA module is aborded. How many bytes have been received (including
    the pattern ? Use this routine to have the answer).

  Parameters:
    id          - The DMA module you want to use.
  *****************************************************************************/
uint16_t dma_get_index_cell_pointer(DMA_MODULE id)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    return (uint16_t) p_dma_channel->DCHCPTR;
}

/*******************************************************************************
  Function:
    void dma_force_transfer(DMA_MODULE id)

  Description:
    This routine is used to force the transfer on a DMA channel. The transfer is
    from source to destination and is cell block length. 

  Parameters:
    id          - The DMA module you want to use.
  *****************************************************************************/
void dma_force_transfer(DMA_MODULE id)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    dma_channel_enable(id, ON);
    p_dma_channel->DCHECONSET = DMA_EVT_FORCE_TRANSFER;
}

/*******************************************************************************
  Function:
    void dma_abord_transfer(DMA_MODULE id)

  Description:
    This routine is used to abord the transfer on a DMA channel. It turns off
    the channel, clear the source and destination pointers, and reset the event
    detector. When an abord transfer is requested, the current transaction in
    progress (if any) will complete before the channel is reset. 
    The channel registers can be modify only while the channel is disabled.

  Parameters:
    id          - The DMA module you want to use.
  *****************************************************************************/
void dma_abord_transfer(DMA_MODULE id)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    p_dma_channel->DCHECONSET = DMA_EVT_ABORD_TRANSFER;
    while ((p_dma_channel->DCHECON & DMA_EVT_ABORD_TRANSFER) > 0);
}

/*******************************************************************************
  Function:
    DMA_CHANNEL_FLAGS dma_get_flags(DMA_MODULE id)

  Description:
    This routine is used to get all the flags (see DMA_CHANNEL_FLAGS) of a DMA
    channel. There are up to 8 types of interruption by DMA channel (so up to 
    8 different flags).
    You can use flags without to enable interruption. Just check and handle
    flags when you want.
 
  Parameters:
    id          - The DMA module you want to use.
  *****************************************************************************/
DMA_CHANNEL_FLAGS dma_get_flags(DMA_MODULE id)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    DMA_CHANNEL_FLAGS flags = (p_dma_channel->DCHINT) & 0xff;
    return flags;
}

/*******************************************************************************
  Function:
    void dma_clear_flags(DMA_MODULE id, DMA_CHANNEL_FLAGS flags)

  Description:
    This routine is used to clear flag(s) of a DMA channel (see DMA_CHANNEL_FLAGS).
 
  Parameters:
    id          - The DMA module you want to use.
    flags       - The flag(s) you want to clear (see DMA_CHANNEL_FLAGS).
  *****************************************************************************/
void dma_clear_flags(DMA_MODULE id, DMA_CHANNEL_FLAGS flags)
{
    DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
    p_dma_channel->DCHINTCLR = flags;
}

/*******************************************************************************
  Function: 
    const uint8_t dma_get_irq(DMA_MODULE id)
  
  Description:
       This routine is used to get the IRQ number of a DMA module.
  
  Parameters:
       id: The DMA module you want to use.
  
  Return:
       The constant IRQ number.
 ******************************************************************************/
const uint8_t dma_get_irq(DMA_MODULE id)
{
    return dma_irq[id];
}

/*******************************************************************************
  Function:
    void dma_interrupt_handler(DMA_MODULE id)

  Description:
    This routine is called when an interruption occurs. This interrupt 
    handler calls the user _event_handler (if existing) otherwise do nothing.

  Parameters:
    id  - The DMA module you want to use.
  *****************************************************************************/
void dma_interrupt_handler(DMA_MODULE id)
{
    if (dma_event_handler[id] != NULL)
    {
        DMA_CHANNEL_REGISTERS * p_dma_channel = (DMA_CHANNEL_REGISTERS *) DmaChannels[id];
        DMA_CHANNEL_FLAGS flags = (p_dma_channel->DCHINT) & 0xff;
        (*dma_event_handler[id])(id, flags);
    }
}
