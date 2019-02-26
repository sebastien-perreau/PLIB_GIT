/*********************************************************************
*	External PCA9685
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		09/12/2018		- Initial release
* 
*********************************************************************/

#include "../PLIB.h"

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

/*
 * TO DO :
 * Init sequence:
 * 1. Restart
 * 2. Set frequency
 * 3. Set params
 * 4. pwm in a loop
 */

void e_pca9685_deamon(PCA9685_CONFIG *var)
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
        if (i2c_master_state_machine(&var->i2c_params) == _HOME)
        {         
            var->i2c_params.data_access.address_register = 0x00;
            var->i2c_params.data_access.read_write_type = I2C_WRITE;
            var->i2c_params.data_access.index = 0;
            var->i2c_params.data_access.length = sizeof(PCA9685_REGS);
        }
    }
}
