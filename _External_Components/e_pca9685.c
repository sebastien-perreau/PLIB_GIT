/*********************************************************************
*	External PCA9685
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		26/02/2019		- Initial release
* 
*********************************************************************/

#include "../PLIB.h"

#warning "e_pca9685.c - I2C Frequency can be 100 KHz / 400 KHz / 1 MHz"

/*******************************************************************************
  Function:
    static void e_pca9685_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)

  Description:
    This routine is the event handler called by the interrupt routine (only if enable).
    To use this event handler, it must be passed as parameter in the i2c_init_as_master
    function (initialization sequence). Do not forget to enable the IRQ(s). 
    If you do not want to use it then passed NULL as parameter and disable IRQ(s). 

  Parameters:
    id          - Identifier of the I2C module which generates the event (ex: I2C1)
    evt_type    - Type of event (see. IRQ_EVENT_TYPE for I2C bus)
    data        - The data read by the I2C module (data will be always 8-bits size but for 
                compatibility with other serial bus, the variable is a 32-bits type)
  *****************************************************************************/
static void e_pca9685_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_I2C_MASTER:
            /*
             * Events: 
             * - Start
             * - Repeated Start
             * - Stop
             * - data transmit + ACK receive (9th clock)
             * _ data receive + ACK transmit (9th clock)
             */
            break;
        case IRQ_I2C_SLAVE:
            break;
        case IRQ_I2C_BUS_COLISION:
            break;
    }
}

/*******************************************************************************
  Function:
    uint8_t e_pca9685_deamon(PCA9685_CONFIG *var)

  Description:
    This routine is the main function for managing the PCA9685 driver. It
    should be placed in the 'main while loop' before or after the bus_management
    routine.

  Parameters:
    *var    - The PCA9685_CONFIG pointer variable

  Return:
    The value returned represents the state of the i2c_master_state_machine.
    See I2C_STATE_MACHIN in the i2c driver.
  *****************************************************************************/
uint8_t e_pca9685_deamon(PCA9685_CONFIG *var)
{
    uint8_t ret;
    
    if (!var->is_init_done)
    {
        i2c_init_as_master(var->i2c_params.module, NULL, I2C_FREQUENCY_400KHZ, I2C_CONTINUE_ON_IDLE | I2C_DISABLE_SMBUS);
        IRQInit(IRQ_I2C1B + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        IRQInit(IRQ_I2C1S + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        IRQInit(IRQ_I2C1M + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        var->is_init_done = true;
        ret = _BUS_I2C_INIT;
    }
    else
    {
        if (GET_BIT(var->i2c_functions.flags, PCA9685_FLAG_SLEEP_MODE))
        {
            var->registers.mode1 |= PCA9685_PUT_IN_SLEEP_MODE;
        }
        else if (GET_BIT(var->i2c_functions.flags, PCA9685_FLAG_NORMAL_MODE))
        {
            var->registers.mode1 &= ~PCA9685_PUT_IN_SLEEP_MODE;
        }
        
        ret = i2c_master_state_machine(&var->i2c_params, &var->i2c_functions);
    }
    
    return ret;
}
