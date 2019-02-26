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

SPI_IO_DCPT_PARAMS SpiIoDcpt[] =
{
    SPI_IO_DCPT_INSTANCE(SPI1_CLK, SPI1_SDO, SPI1_SDI, SPI1_SS),
    SPI_IO_DCPT_INSTANCE(SPI2_CLK, SPI2_SDO, SPI2_SDI, SPI2_SS),
    SPI_IO_DCPT_INSTANCE(SPI3_CLK, SPI3_SDO, SPI3_SDI, SPI3_SS),
    SPI_IO_DCPT_INSTANCE(SPI4_CLK, SPI4_SDO, SPI4_SDI, SPI4_SS),
};

IRQ_SOURCE spiIrqSource[] =
{
    IRQ_SPI1RX,
    IRQ_SPI2RX,
    IRQ_SPI3RX,
    IRQ_SPI4RX
};

void SPIInit(SPI_MODULE id, QWORD freqHz, SPI_CONFIG config)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[id];
    SPI_IO_DCPT_PARAMS * p_dcpt = &SpiIoDcpt[id];
    uint32_t dummy;
    
    ports_reset_pin_output(p_dcpt->SDO);
    ports_reset_pin_input(p_dcpt->SDI);

    if(!config&~SPI_CONF_MSTEN)
    {
        ports_reset_pin_output(p_dcpt->SCK);
    }
    else
    {
        ports_reset_pin_input(p_dcpt->SCK);
    }

    if(!config&~SPI_CONF_SSEN)
    {
        ports_reset_pin_input(p_dcpt->SS);
    }

    spiRegister->SPIxCON = 0;
    spiRegister->SPIxBRG = ((uint32_t) (PERIPHERAL_FREQ / freqHz) >> 1) - 1;
    dummy = spiRegister->SPIxBUF;
    spiRegister->SPIxSTATCLR = _SPI1STAT_SPIROV_MASK;
    spiRegister->SPIxCON = config;
    spiRegister->SPIxCONbits.SPION = ON;
}

void SPIEnable(SPI_MODULE mSpiModule, BOOL enable)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    
    spiRegister->SPIxCONbits.SPION = enable;
}

void SPIInitIOAsChipSelect(_IO chip_select)
{
    ports_reset_pin_output(chip_select);
    ports_set_bit(chip_select);
}

DWORD SPIGetMode(SPI_MODULE mSpiModule)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    // return SPI_CONF_MODE8 | SPI_CONF_MODE16 | SPI_CONF_MODE32
    return (spiRegister->SPIxCON & 0x00000C00);
}

void SPISetMode(SPI_MODULE mSpiModule, SPI_CONFIG mode)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    spiRegister->SPIxCONbits.MODE16 = ((mode >> 10) & 0x00000001);
    spiRegister->SPIxCONbits.MODE32 = ((mode >> 11) & 0x00000001);
}

void SPISetFreq(SPI_MODULE mSpiModule, QWORD freqHz)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    spiRegister->SPIxBRG = ((DWORD) (PERIPHERAL_FREQ / freqHz) >> 1) - 1;
}

QWORD SPIGetFreq(SPI_MODULE mSpiModule)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    return (QWORD) (PERIPHERAL_FREQ/(2*(spiRegister->SPIxBRG + 1)));
}

BOOL SPIIsRxAvailable(SPI_MODULE mSpiModule)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];
    
    if(spiRegister->SPIxCONbits.ENHBUF)
    {
        return spiRegister->SPIxSTATbits.SPIRBE == 0;
    }
    else
    {
        return spiRegister->SPIxSTATbits.SPIRBF != 0;
    }
}

BOOL SPIIsTxAvailable(SPI_MODULE mSpiModule)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[mSpiModule];

    if(spiRegister->SPIxCONbits.ENHBUF)
    {
        return spiRegister->SPIxSTATbits.SPITBF == 0;
    }
    else
    {
        return spiRegister->SPIxSTATbits.SPITBE != 0;
    }
}

BOOL SPIWriteAndStore(SPI_MODULE spi_module, _IO chip_select, uint32_t txData, uint32_t* rxData, bool releaseChipSelect)
{
    if(SPIIsTxAvailable(spi_module))
    {
        SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
        SPIPointerDwordDataReceived[spi_module] = rxData;
        SPICurrentChipSelect[spi_module] = chip_select;
        SPIReleaseChipSelect[spi_module] = releaseChipSelect;
        ports_clr_bit(chip_select);
        spiRegister->SPIxBUF = txData;
        return 1;
    }
    return 0;
}

BYTE SPIWriteAndStore8_16_32(SPI_MODULE spi_module, _IO chip_select, uint32_t txData, uint32_t *rxData, SPI_CONFIG confMode)
{
    SPI_REGISTERS * spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
    static BYTE functionState[SPI_NUMBER_OF_MODULES] = {0};
    
    switch(functionState[spi_module])
    {
        case 0:
            SPISetMode(spi_module, confMode);
            SPIPointerDwordDataReceived[spi_module] = rxData;
            SPICurrentChipSelect[spi_module] = chip_select;
            SPIReleaseChipSelect[spi_module] = TRUE;
            ports_clr_bit(chip_select);
            functionState[spi_module]++;
        case 1:
            if((!spiRegister->SPIxCONbits.ENHBUF && spiRegister->SPIxSTATbits.SPITBE) || (spiRegister->SPIxCONbits.ENHBUF && !spiRegister->SPIxSTATbits.SPITBF))
            {
                spiRegister->SPIxBUF = txData;
                functionState[spi_module]++;
            }
            break;
        case 2:
            if (ports_get_bit(chip_select))
            {
                functionState[spi_module] = 0;
            }
            break;
    }

    return functionState[spi_module];
}

BYTE SPIWriteAndStoreByteArray(SPI_MODULE spi_module, _IO chip_select, void *txBuffer, void *rxBuffer, uint32_t size)
{
    SPI_REGISTERS *spiRegister = (SPI_REGISTERS *)SpiModules[spi_module];
    static BYTE functionState[SPI_NUMBER_OF_MODULES] = {0};
    static QWORD tickEOT[SPI_NUMBER_OF_MODULES] = {0};
    static QWORD periodEOT[SPI_NUMBER_OF_MODULES] = {0};
    
    switch(functionState[spi_module])
    {
        case 0:
            spiRegister->SPIxCONbits.MODE16 = ((SPI_CONF_MODE8 >> 10) & 0x00000001);
            spiRegister->SPIxCONbits.MODE32 = ((SPI_CONF_MODE8 >> 11) & 0x00000001);
            SPICurrentChipSelect[spi_module] = chip_select;
            
            DmaChnSetTxfer(DMA_CHANNEL0+spi_module, txBuffer, (void*)&spiRegister->SPIxBUF, size, 1, 1);
            periodEOT[spi_module] = (QWORD) ((size*8*20*(spiRegister->SPIxBRG + 1)/(PERIPHERAL_FREQ/1000000))*TICK_1US/10 + 1);
            if(rxBuffer != NULL)
            {
                DmaChnSetTxfer(DMA_CHANNEL4+spi_module, (void*)&spiRegister->SPIxBUF, (void*)rxBuffer, 1, size, 1);
            }
            
            ports_clr_bit(chip_select);
            DmaChnEnable(DMA_CHANNEL4+spi_module);                         // Enable the DMA channel
            DmaChnStartTxfer(DMA_CHANNEL0+spi_module, DMA_WAIT_NOT, 0);    // Force the DMA transfer
            
            tickEOT[spi_module] = mGetTick();
            functionState[spi_module] = 1;
            break;
        case 1:
            // Do nothing .. just wait the end of transmission
            if(mTickCompare(tickEOT[spi_module]) >= periodEOT[spi_module])
            {
                ports_set_bit(chip_select);
                irq_clr_flag(spiIrqSource[spi_module]);
                DmaChnAbortTxfer(DMA_CHANNEL4+spi_module);
                DmaChnDisable(DMA_CHANNEL4+spi_module);
                functionState[spi_module] = 0;
            }
            break;
    }

    return functionState[spi_module];
}
