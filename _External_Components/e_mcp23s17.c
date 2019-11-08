/*********************************************************************
 *	External peripheral MCP23S17 (16 bits I/O expander with serial interface)
 *	Author : Sébastien PERREAU
 *   Example : PIC32 examples - MCP23S17
 *
 *	Revision history	:
 *		27/04/2015		- Initial release
 *                       - Compatible with all SPI bus in same time
 *       15/04/2016      - Add BUS management with "Deamon Parent"
 *       31/05/2016      - Driver has been largely re-written with SPI using DMA
 * 
 * Note:
 * By default, BANK = 0 and the address pointer increment automatically.
*********************************************************************/

#include "../PLIB.h"

#warning "e_mcp23s17.c - SPI Frequency should be maximum 10 MHz"

/*******************************************************************************
  Function:
    void eMCP23S17Deamon(MCP23S17_CONFIG *var);

  Description:
    This routine is the deamon for the MCP23S17 controller.

  Parameters:
    *var        - The variable assign to the MCP23S17 device.
  *****************************************************************************/
BYTE eMCP23S17Deamon(MCP23S17_CONFIG *var) 
{
    BYTE ret = 1;
    
    if (!var->spi_params.is_chip_select_initialize)
    {
        ports_reset_pin_output(var->spi_params.chip_select);
        ports_set_bit(var->spi_params.chip_select);
        var->spi_params.is_chip_select_initialize = true;
    }
    
    if(var->spi_params.bus_management_params.is_running)
    {
        switch(var->spi_params.state_machine.index)
        {
            case SM_MCP23S17_HOME: 
                var->spi_params.state_machine.index = SM_MCP23S17_WRITE;
                break;
            case SM_MCP23S17_WRITE:
//                if(!SPIWriteAndStoreByteArray(var->spi_params.spi_module, var->spi_params.chip_select, (void*)&var->write_registers, NULL, sizeof(var->write_registers)))
//                {
//                    var->spi_params.state_machine.index = SM_MCP23S17_READ;
//                }
                break;
            case SM_MCP23S17_READ:
                var->read_registers.header = 0x0041;
//                if(!SPIWriteAndStoreByteArray(var->spi_params.spi_module, var->spi_params.chip_select, (void*)&var->read_registers, (void*)&var->read_registers, sizeof(var->read_registers)))
//                {
//                    var->spi_params.state_machine.index = SM_MCP23S17_END;
//                }
                break;
            case SM_MCP23S17_END:
                var->spi_params.state_machine.index = SM_MCP23S17_HOME;
                var->spi_params.bus_management_params.is_running = FALSE;
                var->spi_params.bus_management_params.tick = mGetTick();
                ret = 0;
                break;
        }
    }
    return ret;
}
