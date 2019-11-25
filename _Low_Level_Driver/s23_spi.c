/*********************************************************************
 * 
 *	PIC32 SPI modules - SPI1 / SPI2 / SPI3 / SPI4
 * 
 *	Author : S�bastien PERREAU
 * 
*********************************************************************/

#include "../PLIB.h"

extern const spi_registers_t * SpiModules[];
const spi_registers_t * SpiModules[] =
{
    (spi_registers_t *)_SPI1_BASE_ADDRESS,
    (spi_registers_t *)_SPI2_BASE_ADDRESS,
    (spi_registers_t *)_SPI3_BASE_ADDRESS,
    (spi_registers_t *)_SPI4_BASE_ADDRESS
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
    spi_io_t _spi_io[] =
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
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    
    if(spiRegister->SPICON.ENHBUF)
    {
        return spiRegister->SPISTAT.SPIRBE == 0;
    }
    else
    {
        return spiRegister->SPISTAT.SPIRBF != 0;
    }
}

static bool spi_is_tx_available(SPI_MODULE id)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];

    if(spiRegister->SPICON.ENHBUF)
    {
        return spiRegister->SPISTAT.SPITBF == 0;
    }
    else
    {
        return spiRegister->SPISTAT.SPITBE != 0;
    }
}

void spi_init(SPI_MODULE id, spi_event_handler_t evt_handler, IRQ_EVENT_TYPE event_type_enable, uint32_t freq_hz, SPI_CONFIG config)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    uint32_t dummy;

    spi_event_handler[id] = evt_handler;
    irq_init(IRQ_SPI1E + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_FAULT) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
    irq_init(IRQ_SPI1RX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_RX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
    irq_init(IRQ_SPI1TX + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_SPI_TX) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_spi_priority(id));
 
    spi_io_init(id, config);
    
    spiRegister->SPICON.value = 0;
    // Clear the receive buffer
    dummy = spiRegister->SPIBUF;
    // Clear the overflow
    spiRegister->SPISTATCLR = _SPI1STAT_SPIROV_MASK;
    spi_set_frequency(id, freq_hz);    
    spiRegister->SPICON.value = config;
    spi_enable(id, ON);
}

void spi_enable(SPI_MODULE id, bool enable)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];    
    spiRegister->SPICON.SPION = enable;
}

void spi_set_mode(SPI_MODULE id, SPI_CONFIG mode)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    spiRegister->SPICON.MODE16 = ((mode >> 10) & 0x00000001);
    spiRegister->SPICON.MODE32 = ((mode >> 11) & 0x00000001);
}

void spi_set_frequency(SPI_MODULE id, uint32_t freq_hz)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    spiRegister->SPIBRG = ((uint32_t) (PERIPHERAL_FREQ / freq_hz) >> 1) - 1;
}

bool spi_write_and_read_8(SPI_MODULE id, uint32_t data_w, uint8_t * data_r)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    
    if(spi_is_tx_available(id))
    {
        spiRegister->SPIBUF = data_w;
        while(!spi_is_rx_available(id));
        *data_r = (uint8_t) spiRegister->SPIBUF;
        return 0;
    }
    return 1;
}

bool spi_write_and_read_16(SPI_MODULE id, uint32_t data_w, uint16_t * data_r)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    
    if(spi_is_tx_available(id))
    {
        spiRegister->SPIBUF = data_w;
        while(!spi_is_rx_available(id));
        *data_r = (uint16_t) spiRegister->SPIBUF;
        return 0;
    }
    return 1;
}

bool spi_write_and_read_32(SPI_MODULE id, uint32_t data_w, uint32_t * data_r)
{
    spi_registers_t * spiRegister = (spi_registers_t *)SpiModules[id];
    
    if(spi_is_tx_available(id))
    {
        spiRegister->SPIBUF = data_w;
        while(!spi_is_rx_available(id));
        *data_r = (uint32_t) spiRegister->SPIBUF;
        return 0;
    }
    return 1;
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
