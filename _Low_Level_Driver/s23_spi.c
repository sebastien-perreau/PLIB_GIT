/*********************************************************************
*	SPI modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		30/01/2014		- Initial release
*       26/06/2015      - Ajout des fonctions
*                           + SPIWriteAndStore8_16_32(...)
*                           + SPIWriteAndStoreByteArray(...)
*       30/05/2016      - Modifications des 3 fonctions d'envoi
*                       - Ajout du DMA sur la fonction Byte Array
*       19/05/2017      - Ajout d'un 'abord DMA' sur la fonction Byte Array
*                       pour correctif bug lorsque rxBuffer == NULL et qu'on
*                       utilise Byte Array et SPIWriteAndStore8_16_32.
*       04/01/2018      - Add "SPIEnable" function.
*********************************************************************/

#include "../PLIB.h"

extern const SPI_REGISTERS * SpiModules[];
const SPI_REGISTERS * SpiModules[] =
{
    (SPI_REGISTERS *)_SPI1_BASE_ADDRESS,
    (SPI_REGISTERS *)_SPI2_BASE_ADDRESS,
    (SPI_REGISTERS *)_SPI3_BASE_ADDRESS,
    (SPI_REGISTERS *)_SPI4_BASE_ADDRESS
};
static spi_event_handler_t spi_event_handler[SPI_NUMBER_OF_MODULES] = {NULL};

const uint8_t spi_tx_irq[] = 
{
    _SPI1_TX_IRQ,
    _SPI2_TX_IRQ,
    _SPI3_TX_IRQ,
    _SPI4_TX_IRQ
};

const uint8_t spi_rx_irq[] = 
{
    _SPI1_RX_IRQ,
    _SPI2_RX_IRQ,
    _SPI3_RX_IRQ,
    _SPI4_RX_IRQ
};

const void *p_spi_tx_reg[] = 
{
    (void*) &SPI1BUF,
    (void*) &SPI2BUF,
    (void*) &SPI3BUF,
    (void*) &SPI4BUF
};

const void *p_spi_rx_reg[] = 
{
    (void*) &SPI1BUF,
    (void*) &SPI2BUF,
    (void*) &SPI3BUF,
    (void*) &SPI4BUF
};

static void spi_io_init(SPI_MODULE id, SPI_CONFIG config)
{
    SPI_IO _spi_io[] =
    {
        SPI_IO_INSTANCE(SPI1_CLK, SPI1_SDO, SPI1_SDI, SPI1_SS),
        SPI_IO_INSTANCE(SPI2_CLK, SPI2_SDO, SPI2_SDI, SPI2_SS),
        SPI_IO_INSTANCE(SPI3_CLK, SPI3_SDO, SPI3_SDI, SPI3_SS),
        SPI_IO_INSTANCE(SPI4_CLK, SPI4_SDO, SPI4_SDI, SPI4_SS),
    };
    (!config&~SPI_CONF_MSTEN) ? ports_reset_pin_output(_spi_io[id].SCK) : ports_reset_pin_input(_spi_io[id].SCK);
    ports_reset_pin_output(_spi_io[id].SDO);
    ports_reset_pin_input(_spi_io[id].SDI);
    (!config&~SPI_CONF_SSEN) ? ports_reset_pin_input(_spi_io[id].SS) : 0;
}

static bool spi_is_rx_available(SPI_MODULE id)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    
    if(spiRegister->SPIxCONbits.ENHBUF)
    {
        return spiRegister->SPIxSTATbits.SPIRBE == 0;
    }
    else
    {
        return spiRegister->SPIxSTATbits.SPIRBF != 0;
    }
}

static bool spi_is_tx_available(SPI_MODULE id)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];

    if(spiRegister->SPIxCONbits.ENHBUF)
    {
        return spiRegister->SPIxSTATbits.SPITBF == 0;
    }
    else
    {
        return spiRegister->SPIxSTATbits.SPITBE != 0;
    }
}

void spi_init(SPI_MODULE id, spi_event_handler_t evt_handler, IRQ_EVENT_TYPE event_type_enable, uint32_t freq_hz, SPI_CONFIG config)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    uint32_t dummy;

    spi_event_handler[id] = evt_handler;
    irq_init(IRQ_SPI1E + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_FAULT) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
    irq_init(IRQ_SPI1RX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_RX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
    irq_init(IRQ_SPI1TX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_TX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
 
    spi_io_init(id, config);
    
    spiRegister->SPIxCON = 0;
    // Clear the receive buffer
    dummy = spiRegister->SPIxBUF;
    // Clear the overflow
    spiRegister->SPIxSTATCLR = _SPI1STAT_SPIROV_MASK;
    spi_set_frequency(id, freq_hz);    
    spiRegister->SPIxCON = config;
    spi_enable(id, ON);
}

void spi_enable(SPI_MODULE id, bool enable)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];    
    spiRegister->SPIxCONbits.SPION = enable;
}

void spi_set_mode(SPI_MODULE id, SPI_CONFIG mode)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    spiRegister->SPIxCONbits.MODE16 = ((mode >> 10) & 0x00000001);
    spiRegister->SPIxCONbits.MODE32 = ((mode >> 11) & 0x00000001);
}

void spi_set_frequency(SPI_MODULE id, uint32_t freq_hz)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    spiRegister->SPIxBRG = ((uint32_t) (PERIPHERAL_FREQ / freq_hz) >> 1) - 1;
}

bool spi_write_and_read(SPI_MODULE id, uint32_t data_w, uint32_t * data_r)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    
    if(spi_is_tx_available(id))
    {
        spiRegister->SPIxBUF = data_w;
        while(!spi_is_rx_available(id));
        *data_r = (uint32_t) spiRegister->SPIxBUF;
        return 0;
    }
    return 1;
}

BOOL SPIWriteAndStore(SPI_MODULE spi_module, _IO chip_select, uint32_t txData, uint32_t* rxData, bool releaseChipSelect)
{
//    if(SPIIsTxAvailable(spi_module))
//    {
//        SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
//        SPIPointerDwordDataReceived[spi_module] = rxData;
//        SPICurrentChipSelect[spi_module] = chip_select;
//        SPIReleaseChipSelect[spi_module] = releaseChipSelect;
//        ports_clr_bit(chip_select);
//        spiRegister->SPIxBUF = txData;
//        return 1;
//    }
    return 0;
}

BYTE SPIWriteAndStore8_16_32(SPI_MODULE spi_module, _IO chip_select, uint32_t txData, uint32_t *rxData, SPI_CONFIG confMode)
{
//    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
//    static BYTE functionState[SPI_NUMBER_OF_MODULES] = {0};
//    
//    switch(functionState[spi_module])
//    {
//        case 0:
//            spi_set_mode(spi_module, confMode);
//            SPIPointerDwordDataReceived[spi_module] = rxData;
//            SPICurrentChipSelect[spi_module] = chip_select;
//            SPIReleaseChipSelect[spi_module] = TRUE;
//            ports_clr_bit(chip_select);
//            functionState[spi_module]++;
//        case 1:
//            if(SPIIsTxAvailable(spi_module))
//            {
//                spiRegister->SPIxBUF = txData;
//                functionState[spi_module]++;
//            }
//            break;
//        case 2:
//            if (ports_get_bit(chip_select))
//            {
//                functionState[spi_module] = 0;
//            }
//            break;
//    }
//
//    return functionState[spi_module];
}

BYTE SPIWriteAndStoreByteArray(SPI_MODULE spi_module, _IO chip_select, void *txBuffer, void *rxBuffer, uint32_t size)
{
//    SPI_REGISTERS *spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
//    static BYTE functionState[SPI_NUMBER_OF_MODULES] = {0};
//    static QWORD tickEOT[SPI_NUMBER_OF_MODULES] = {0};
//    static QWORD periodEOT[SPI_NUMBER_OF_MODULES] = {0};
//    IRQ_SOURCE spiIrqSource[] =
//    {
//        IRQ_SPI1RX,
//        IRQ_SPI2RX,
//        IRQ_SPI3RX,
//        IRQ_SPI4RX
//    };
//    
//    switch(functionState[spi_module])
//    {
//        case 0:
//            spiRegister->SPIxCONbits.MODE16 = ((SPI_CONF_MODE8 >> 10) & 0x00000001);
//            spiRegister->SPIxCONbits.MODE32 = ((SPI_CONF_MODE8 >> 11) & 0x00000001);
//            SPICurrentChipSelect[spi_module] = chip_select;
//            
//            DmaChnSetTxfer(DMA_CHANNEL0+spi_module, txBuffer, (void*)&spiRegister->SPIxBUF, size, 1, 1);
//            periodEOT[spi_module] = (QWORD) ((size*8*20*(spiRegister->SPIxBRG + 1)/(PERIPHERAL_FREQ/1000000))*TICK_1US/10 + 1);
//            if(rxBuffer != NULL)
//            {
//                DmaChnSetTxfer(DMA_CHANNEL4+spi_module, (void*)&spiRegister->SPIxBUF, (void*)rxBuffer, 1, size, 1);
//            }
//            
//            ports_clr_bit(chip_select);
//            DmaChnEnable(DMA_CHANNEL4+spi_module);                         // Enable the DMA channel
//            DmaChnStartTxfer(DMA_CHANNEL0+spi_module, DMA_WAIT_NOT, 0);    // Force the DMA transfer
//            
//            tickEOT[spi_module] = mGetTick();
//            functionState[spi_module] = 1;
//            break;
//        case 1:
//            // Do nothing .. just wait the end of transmission
//            if(mTickCompare(tickEOT[spi_module]) >= periodEOT[spi_module])
//            {
//                ports_set_bit(chip_select);
//                irq_clr_flag(spiIrqSource[spi_module]);
//                DmaChnAbortTxfer(DMA_CHANNEL4+spi_module);
//                DmaChnDisable(DMA_CHANNEL4+spi_module);
//                functionState[spi_module] = 0;
//            }
//            break;
//    }
//
//    return functionState[spi_module];
}

/*******************************************************************************
 * Function: 
 *      const uint8_t spi_get_tx_irq(SPI_MODULE id)
 * 
 * Description:
 *      This routine is used to get the TX IRQ number of a SPI module.
 * 
 * Parameters:
 *      id: The SPI module you want to use.
 * 
 * Return:
 *      The constant TX IRQ number.
 ******************************************************************************/
const uint8_t spi_get_tx_irq(SPI_MODULE id)
{
    return spi_tx_irq[id];
}

/*******************************************************************************
 * Function: 
 *      const uint8_t spi_get_rx_irq(SPI_MODULE id)
 * 
 * Description:
 *      This routine is used to get the RX IRQ number of a SPI module.
 * 
 * Parameters:
 *      id: The SPI module you want to use.
 * 
 * Return:
 *      The constant RX IRQ number.
 ******************************************************************************/
const uint8_t spi_get_rx_irq(SPI_MODULE id)
{
    return spi_rx_irq[id];
}

/*******************************************************************************
 * Function: 
 *      const void *spi_get_tx_reg(SPI_MODULE id)
 * 
 * Description:
 *      This routine is used to get the TX Register of a SPI module.
 * 
 * Parameters:
 *      id: The SPI module you want to use.
 * 
 * Return:
 *      The constant pointer of SPI TX REGISTER.
 ******************************************************************************/
const void *spi_get_tx_reg(SPI_MODULE id)
{
    return (void*) p_spi_tx_reg[id];
}

/*******************************************************************************
 * Function: 
 *      const void *spi_get_rx_reg(SPI_MODULE id)
 * 
 * Description:
 *      This routine is used to get the RX Register of a SPI module.
 * 
 * Parameters:
 *      id: The SPI module you want to use.
 * 
 * Return:
 *      The constant pointer of SPI RX REGISTER.
 ******************************************************************************/
const void *spi_get_rx_reg(SPI_MODULE id)
{
    return (void*) p_spi_rx_reg[id];
}

/*******************************************************************************
 * Function: 
 *      void spi_interrupt_handler(SPI_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      id: The SPI module you want to use.
 *      evt_type: The type of event (RX, TX, FAULT...). See IRQ_EVENT_TYPE.
 *      data: The data (in case of a reception) read in the interruption.
 * 
 * Return:
 *      none
 ******************************************************************************/
void spi_interrupt_handler(SPI_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    if (spi_event_handler[id] != NULL)
    {
        (*spi_event_handler[id])(id, evt_type, data);
    }
}
