/*********************************************************************
*	External AT42QT2120
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		15/02/2019		- Initial release
* 
*********************************************************************/

#include "../PLIB.h"

/*******************************************************************************
  Function:
    -

  Description:
    The '_function_tab' is a constant array which contains the details (read_write_type, 
    register_address and length) of each functions that the AT42QT2120_FLAGS can execute. 
    The order in this array is IMPORTANT and should be the same as the flags.
    For example if the flags look like that: 
    - Reset = 0
    - Calibrate = 1
    - Set_params = 2
    Then the '_function_tab' should contains the parameters in the same order:
    - Reset details (write, address_reset, size_reset)
    - Calibrate details (write, address_calibrate, size_calibrate)
    - Set_params details (write, address_set_params, size_set_params)

  Parameters:
    -
  *****************************************************************************/
const AT42QT2120_FUNCTION _function_tab[] = 
{
    {I2C_WRITE,     AT42QT2120_ADDR_RESET,                              AT42QT2120_SIZE_RESET},
    {I2C_WRITE,     AT42QT2120_ADDR_CALIBRATE,                          AT42QT2120_SIZE_CALIBRATE},
    {I2C_WRITE,     AT42QT2120_ADDR_SET_PARAMS,                         AT42QT2120_SIZE_SET_PARAMS},
    {I2C_WRITE,     AT42QT2120_ADDR_SET_KEY_DETECT_THRESHOLD_0_TO_11,   AT42QT2120_SIZE_SET_KEY_DETECT_THRESHOLD_0_TO_11},
    {I2C_WRITE,     AT42QT2120_ADDR_SET_KEY_CONTROL_0_TO_11,            AT42QT2120_SIZE_SET_KEY_CONTROL_0_TO_11},
    {I2C_WRITE,     AT42QT2120_ADDR_SET_KEY_PULSE_SCALE_0_TO_11,        AT42QT2120_SIZE_SET_KEY_PULSE_SCALE_0_TO_11},
    {I2C_READ,      AT42QT2120_ADDR_GET_KEY_SIGNAL,                     AT42QT2120_SIZE_GET_KEY_SIGNAL},
    {I2C_READ,      AT42QT2120_ADDR_GET_REF_DATA,                       AT42QT2120_SIZE_GET_REF_DATA},
    {I2C_READ,      AT42QT2120_ADDR_GET_STATUS_KEYS_AND_SLIDER,         AT42QT2120_SIZE_GET_STATUS_KEYS_AND_SLIDER}
};

/*******************************************************************************
  Function:
    static void e_at42qt2120_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)

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
static void e_at42qt2120_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
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
    void e_at42qt2120_deamon(AT42QT2120_CONFIG *var)

  Description:
    This routine is the main function for managing the AT42QT2120 driver. It
    should be placed in the 'main while loop' before or after the bus_management
    routine.

  Parameters:
    *var    - The AT42QT2120_CONFIG pointer variable
  *****************************************************************************/
void e_at42qt2120_deamon(AT42QT2120_CONFIG *var)
{
    
    if (!var->is_init_done)
    {
        i2c_init_as_master(var->i2c_params.module, NULL, I2C_FREQUENCY_400KHZ, I2C_CONTINUE_ON_IDLE | I2C_DISABLE_SMBUS);
        IRQInit(IRQ_I2C1B + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        IRQInit(IRQ_I2C1S + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        IRQInit(IRQ_I2C1M + var->i2c_params.module, IRQ_DISABLED, IRQ_PRIORITY_LEVEL_3, IRQ_SUB_PRIORITY_LEVEL_1);
        var->is_init_done = true;
    }
    else
    {
        uint8_t ret, j;
        
        ret = i2c_master_state_machine(&var->i2c_params);
        
        if (var->active_function == AT42QT2120_FLAG_NO_FUNCTION)
        {
            for (j = 0 ; j < AT42QT2120_FLAG_NUMBERS ; j++)
            {
                if (GET_BIT(var->flags, j))                    
                {
                    var->active_function = j;
                    break;
                }
            }
        }
        
        if (var->active_function < AT42QT2120_FLAG_NUMBERS)
        {
            if (ret == _STOP)
            {
                CLR_BIT(var->flags, var->active_function);
                var->active_function = AT42QT2120_FLAG_NO_FUNCTION;
            }
            else if (ret == _START)
            {
                var->i2c_params.data_access.address_register    = _function_tab[var->active_function].address_register;
                var->i2c_params.data_access.read_write_type     = _function_tab[var->active_function].read_write_type;
                var->i2c_params.data_access.index               = 0;
                var->i2c_params.data_access.length              = _function_tab[var->active_function].length;
            }
        }
    }
}
