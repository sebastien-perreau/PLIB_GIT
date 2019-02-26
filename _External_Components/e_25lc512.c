/*********************************************************************
*	External EEPROM 25xx512 (Microchip Memory) ONLY.
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		13/10/2015		- Initial release
                        - Compatible with all SPI bus in same time but only
                        one device by SPI bus.
*       18/04/2016      - Add BUS management with "Deamon Parent".
* 
*   Description:
*   ------------ 
*   The 25xx512 is a 512 Kbit (64 KO -> 0000h to FFFFh) serial EEPROM memory.
*   128 bytes per page (512 pages total).
*   16 KO per sector (4 sectors total).
*   This driver manage automatically the EEPROM (using the deamon) without 
*   blocking the CPU. 
*   
*   Timings:
*   --------
*   5mS max write op.
*   5mS typical for page erase.
*   10mS/Sector typical for sector erase.
*   10mS typical for bulk erase.
*********************************************************************/

#include "../PLIB.h"

#warning "e_eeprom.c - SPI Frequency should be maximum 20 MHz"

static uint32_t readingDwordValue[SPI_NUMBER_OF_MODULES];

/*******************************************************************************
  Function:
    void e_25lc512_deamon(_25LC512_CONFIG *var)

  Description:
    This routine is the state machine for the 25xx512 controller.
    You can add as much as deamon than you have device because the SPI bus is release at the end of
    each command transmission.

  Parameters:
    *var     - The variable assign to the 25xx512 device.
  *****************************************************************************/
void e_25lc512_deamon(_25LC512_CONFIG *var)
{
    static uint8_t i[SPI_NUMBER_OF_MODULES] = {0};
    
    if (!var->spi_params.is_chip_select_initialize)
    {
        SPIInitIOAsChipSelect(var->spi_params.chip_select);
        var->spi_params.is_chip_select_initialize = true;
    }

    if(var->spi_params.bus_management_params.is_running)
    {
        switch(var->spi_params.state_machine.index)
        {
            case SM_25LC512_HOME:
                for(i[var->spi_params.spi_module] = 1 ; i[var->spi_params.spi_module] < SM_25LC512_MAX_FLAGS ; i[var->spi_params.spi_module]++) 
                {
                    if((var->spi_params.flags >> i[var->spi_params.spi_module])&0x01)
                    {
                        var->spi_params.state_machine.index = i[var->spi_params.spi_module];
                        break;
                    }
                }
                break;
            case SM_25LC512_WRITE_STATUS_REGISTER:
                if(!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, (_25LC512_INST_WRSR << 8) | _25LC512_DISABLE_ALL_SECTOR_PROTECTION, NULL, SPI_CONF_MODE16))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_PAGE_ERASE:
                if(!e_25lc512_erase_sequences(var, 0))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_SECTOR_ERASE:
                if(!e_25lc512_erase_sequences(var, 1))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_CHIP_ERASE:
                if(!e_25lc512_erase_sequences(var, 2))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_BYTES_ERASE:
                if(!e_25lc512_write_sequences(var, 1))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break; 
            case SM_25LC512_READ:
                if(!e_25lc512_read_sequences(var))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_WRITE:
                if(!e_25lc512_write_sequences(var, 0))
                {
                    CLR_BIT(var->spi_params.flags, var->spi_params.state_machine.index);
                    if(!var->spi_params.flags){var->spi_params.state_machine.index = SM_25LC512_END;}else{var->spi_params.state_machine.index = SM_25LC512_HOME;}
                }
                break;
            case SM_25LC512_END:         
                var->spi_params.state_machine.index = SM_25LC512_HOME;
                var->spi_params.bus_management_params.is_running = false;
                var->spi_params.bus_management_params.tick = mGetTick();
                break;
        }
    }
}

/*******************************************************************************
  Function:
    static char e_25lc512_read_sequences(_25LC512_CONFIG *var)

  Description:
    This routine allow the driver to read a sequence of bytes.

  Parameters:
    var      - The variable assign to the 25xx512 device.
  *****************************************************************************/
static char e_25lc512_read_sequences(_25LC512_CONFIG *var)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_GET_STATUS,
        SM_DATA_READ,
    } functionState[SPI_NUMBER_OF_MODULES] = {0};
    
    switch (functionState[var->spi_params.spi_module])
    {
        case SM_FREE:
            var->registers.dR.p[0] = _25LC512_INST_READ;
            var->registers.dR.p[1] = (var->registers.aR >> 8);
            var->registers.dR.p[2] = (var->registers.aR >> 0);
            
            functionState[var->spi_params.spi_module] = SM_GET_STATUS;
        case SM_GET_STATUS:
            if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, (_25LC512_INST_RDSR << 8), &readingDwordValue[var->spi_params.spi_module], SPI_CONF_MODE16))
            {
                var->registers.status_bit.w = (BYTE) (readingDwordValue[var->spi_params.spi_module] & 0xFF);
                if (!var->registers.status_bit.WIP)
                {
                    functionState[var->spi_params.spi_module] = SM_DATA_READ;
                }
            }
            break;
        case SM_DATA_READ:
            if (!SPIWriteAndStoreByteArray(var->spi_params.spi_module, var->spi_params.chip_select, (void*)var->registers.dR.p, (void*)var->registers.dR.p, (var->registers.dR.size + 3)))
            {
                WORD i = 0;
                for (i = 0 ; i < var->registers.dR.size ; i++)
                {
                    var->registers.dR.p[i] = var->registers.dR.p[i + 3];
                }
                functionState[var->spi_params.spi_module] = SM_FREE;
            }
            break;
    }
    
    return functionState[var->spi_params.spi_module];
}

/*******************************************************************************
  Function:
    static char e_25lc512_erase_sequences(_25LC512_CONFIG *var, uint8_t typeOfErase)

  Description:
    This routine allow the driver to erase a sequence of bytes (page/sector/chip).

  Parameters:
    var      - The variable assign to the 25xx512 device.
 
    typeOfErase - Define the exepted type of erase (page/sector or chip).
  *****************************************************************************/
static char e_25lc512_erase_sequences(_25LC512_CONFIG *var, uint8_t typeOfErase)
{
    static enum _functionState
    {
        SM_FREE = 0,
        SM_GET_STATUS,
        SM_WREN,
        SM_ERASE,
    } functionState[SPI_NUMBER_OF_MODULES] = {0};
    static BYTE tHeader[SPI_NUMBER_OF_MODULES][3] = {0};
    
    switch (functionState[var->spi_params.spi_module])
    {
        case SM_FREE:
            if (typeOfErase == 0)
            {
                tHeader[var->spi_params.spi_module][0] = _25LC512_INST_PAGE_ERASE;
            }
            else if (typeOfErase == 1)
            {
                tHeader[var->spi_params.spi_module][0] = _25LC512_INST_SECTOR_ERASE;
            }
            tHeader[var->spi_params.spi_module][1] = (var->registers.aW >> 8);
            tHeader[var->spi_params.spi_module][2] = (var->registers.aW >> 0);
            functionState[var->spi_params.spi_module] = SM_GET_STATUS;
        case SM_GET_STATUS:
            if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, (_25LC512_INST_RDSR << 8), &readingDwordValue[var->spi_params.spi_module], SPI_CONF_MODE16))
            {
                var->registers.status_bit.w = (BYTE) (readingDwordValue[var->spi_params.spi_module] & 0xFF);
                if (!var->registers.status_bit.WIP)
                {
                    if (!var->registers.status_bit.WEL)
                    {
                        functionState[var->spi_params.spi_module] = SM_WREN;
                    }
                    else
                    {
                        functionState[var->spi_params.spi_module] = SM_ERASE;
                    }
                }
            }
            break;
        case SM_WREN:
            if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, _25LC512_INST_WREN, NULL, SPI_CONF_MODE8))
            {
                functionState[var->spi_params.spi_module] = SM_ERASE;
            }
            break;
        case SM_ERASE:
            if (typeOfErase < 2)
            {
                if (!SPIWriteAndStoreByteArray(var->spi_params.spi_module, var->spi_params.chip_select, (void*)&tHeader[var->spi_params.spi_module][0], NULL, 3))
                {
                    functionState[var->spi_params.spi_module] = SM_FREE;
                }
            }
            else
            {
                if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, _25LC512_INST_CHIP_ERASE, NULL, SPI_CONF_MODE8))
                {
                    functionState[var->spi_params.spi_module] = SM_FREE;
                }
            }
            break;
    }
    
    return functionState[var->spi_params.spi_module];
}

/*******************************************************************************
  Function:
    static char e_25lc512_write_sequences(_25LC512_CONFIG *var, bool writeOrErase)

  Description:
    This routine allow the driver to write a sequence of bytes.
    If a write request occurs with a data length upper than 128 bytes then
    it automatically manage that by writing in the next page and so on.

  Parameters:
    var          - The variable assign to the 25xx512 device.
 
    writeOrErase    - If an erase request occurs then all bytes will have 0xFF value.
  *****************************************************************************/
static char e_25lc512_write_sequences(_25LC512_CONFIG *var, bool writeOrErase)
{    
    static WORD i[SPI_NUMBER_OF_MODULES] = {0};
    static WORD maximumNumberOfByteToWriteInCurrentPage[SPI_NUMBER_OF_MODULES] = {0};
    static enum _functionState
    {
        SM_FREE = 0,
        SM_MEMORY_ORGANIZATION,
        SM_GET_STATUS,
        SM_WREN,
        SM_WRITE,
    } functionState[SPI_NUMBER_OF_MODULES] = {0};

    switch(functionState[var->spi_params.spi_module])
    {
        case SM_FREE:    
            maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module] = 0;
            for (i[var->spi_params.spi_module] = 0 ; i[var->spi_params.spi_module] < var->registers.dW.size ; i[var->spi_params.spi_module]++)
            {
                var->registers.dW.p[3 + var->registers.dW.size - 1 - i[var->spi_params.spi_module]] = (writeOrErase)?0xff:(var->registers.dW.p[var->registers.dW.size - 1 - i[var->spi_params.spi_module]]);
            }
            var->registers.dW.p[0] = _25LC512_INST_WRITE;
            functionState[var->spi_params.spi_module] = SM_MEMORY_ORGANIZATION;
        case SM_MEMORY_ORGANIZATION:   
            // Set adress into header
            var->registers.dW.p[1] = (var->registers.aW >> 8);
            var->registers.dW.p[2] = (var->registers.aW >> 0);
            // Shift left buffer
            for (i[var->spi_params.spi_module] = 0 ; i[var->spi_params.spi_module] < var->registers.dW.size ; i[var->spi_params.spi_module]++)
            {
                var->registers.dW.p[3 + i[var->spi_params.spi_module]] = (writeOrErase)?0xff:(var->registers.dW.p[3 + maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module] + i[var->spi_params.spi_module]]);
            }
            // Get number of byte to write in the current Page (max 128 bytes per page)
            maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module] = 128 - (var->registers.aW % 128);
            if (var->registers.dW.size < maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module])
            {
                maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module] = var->registers.dW.size;
            }
            functionState[var->spi_params.spi_module] = SM_GET_STATUS;
        case SM_GET_STATUS:
            if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, (_25LC512_INST_RDSR << 8), &readingDwordValue[var->spi_params.spi_module], SPI_CONF_MODE16))
            {
                var->registers.status_bit.w = (BYTE) (readingDwordValue[var->spi_params.spi_module] & 0xFF);
                if (!var->registers.status_bit.WIP)
                {
                    if (!var->registers.status_bit.WEL)
                    {
                        functionState[var->spi_params.spi_module] = SM_WREN;
                    }
                    else
                    {
                        functionState[var->spi_params.spi_module] = SM_WRITE;
                    }
                }
            }
            break;
        case SM_WREN:
            if (!SPIWriteAndStore8_16_32(var->spi_params.spi_module, var->spi_params.chip_select, _25LC512_INST_WREN, NULL, SPI_CONF_MODE8))
            {
                functionState[var->spi_params.spi_module] = SM_WRITE;
            }
            break;
        case SM_WRITE:
            if (!SPIWriteAndStoreByteArray(var->spi_params.spi_module, var->spi_params.chip_select, (void*)var->registers.dW.p, NULL, (maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module] + 3)))
            {
                if (var->registers.dW.size > maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module])
                {
                    var->registers.dW.size -= maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module];
                    var->registers.aW += maximumNumberOfByteToWriteInCurrentPage[var->spi_params.spi_module];
                    functionState[var->spi_params.spi_module] = SM_MEMORY_ORGANIZATION;
                }
                else
                {
                    var->registers.dW.size = 0;
                    functionState[var->spi_params.spi_module] = SM_FREE;
                }
            }
            break;
    }
    
    return functionState[var->spi_params.spi_module];
}

void e_25lc512_check_for_erasing_memory(_25LC512_CONFIG *var, BUS_MANAGEMENT_VAR *bm)
{
    static enum
    {
        _PREPARE = 0,
        _WAIT_TO_CONFIRM,
        _ERASE_EEPROM,
        _WAIT_ERASE_COMPLETE,
        _SOFT_RESET        
    } sm_reset = _PREPARE;
    static uint64_t tickReset = 0;

    while (!mGetIO(SWITCH3) || (sm_reset >= _ERASE_EEPROM))
    {
        switch (sm_reset)
        {
            case _PREPARE:
                tickReset = mGetTick();
                sm_reset = _WAIT_TO_CONFIRM;
                break;
            case _WAIT_TO_CONFIRM:
                mLatIO(LED2) = fu_turn_indicator(ON, TICK_100MS, TICK_100MS);
                if (mTickCompare(tickReset) >= TICK_5S)
                {
                    sm_reset = _ERASE_EEPROM;
                }
                break;
            case _ERASE_EEPROM:
                e_25lc512_chip_erase_ptr(var);
                tickReset = mGetTick();
                sm_reset = _WAIT_ERASE_COMPLETE;
                break;
            case _WAIT_ERASE_COMPLETE:
                fu_bus_management_task(bm);
                e_25lc512_deamon(var);
                if (mTickCompare(tickReset) >= TICK_100MS)
                {
                    mSetIO(LED2);
                    sm_reset = _SOFT_RESET;
                }
                break;
            case _SOFT_RESET:
                if (mGetIO(SWITCH3))
                {
                    SoftReset();
                }
                break;
        }
    }
}
