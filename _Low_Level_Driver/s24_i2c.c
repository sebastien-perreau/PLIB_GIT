/*********************************************************************
*	I2C modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		04/12/2018		- Initial release
*********************************************************************/

#include "../PLIB.h"

extern const I2C_REGISTERS * I2cModules[];
const I2C_REGISTERS * I2cModules[] =
{
	(I2C_REGISTERS*)_I2C1_BASE_ADDRESS,
	(I2C_REGISTERS*)_I2C2_BASE_ADDRESS,
	(I2C_REGISTERS*)_I2C3_BASE_ADDRESS,
	(I2C_REGISTERS*)_I2C4_BASE_ADDRESS,
	(I2C_REGISTERS*)_I2C5_BASE_ADDRESS
};
static uint32_t real_frequency_tab[I2C_NUMBER_OF_MODULES] = {0};
static serial_event_handler_t serial_event_handler[I2C_NUMBER_OF_MODULES] = {NULL};

void i2c_init_as_master(    I2C_MODULE id, 
                            serial_event_handler_t evt_handler,
                            I2C_FREQUENCY frequency,
                            I2C_CONFIGURATION configuration)
{
    i2c_enable(id, OFF);
    if (frequency == I2C_FREQUENCY_400KHZ)
    {
        configuration &= !I2C_SLEW_RATE_NORMAL_SPEED;
    }
    else
    {
        configuration |= I2C_SLEW_RATE_NORMAL_SPEED;
    }
    i2c_configuration(id, configuration);
    i2c_set_frequency(id, frequency);
    serial_event_handler[id] = evt_handler;
    i2c_enable(id, ON);
}

void i2c_enable(I2C_MODULE id, bool enable)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.BUS_ON = enable;
}

void i2c_configuration(I2C_MODULE id, I2C_CONFIGURATION configuration)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONSET = configuration;
}

void i2c_set_frequency(I2C_MODULE id, I2C_FREQUENCY frequency)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    float v_brg = (float) (((((100000 / (2 * frequency)) - 10)*(PERIPHERAL_FREQ / 1000000)) / 100) - 2);
    float v_real_freq = (float) (500000 / ((((v_brg + 2) * 1000) / (PERIPHERAL_FREQ / 1000000) + 104) / 1000));
    p_i2c->I2CBRG = (uint32_t ) v_brg;
    real_frequency_tab[id] = (uint32_t) v_real_freq;
}

void i2c_start(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.SEN = 1;
}

void i2c_restart(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.RSEN = 1;
}

void i2c_stop(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.PEN = 1;
}

/*
 * Description:
 * This routine enables the module to receive data from the I2C bus.
 * Thus, the master generates clock in order to allow the slave 
 * to return its data byte.
 * This bis is automatically clear by module at end of 8 bits receive
 * data byte.
 */
void i2c_receiver_active_sequence(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.RCEN = 1;
}

bool i2c_get_byte(I2C_MODULE id, uint8_t *data)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    if (p_i2c->I2CSTATbits.RBF)
    {
        *data = (uint8_t) p_i2c->I2CRX;
        return 0;
    }
    return 1;
}

void i2c_send_ack(I2C_MODULE id, bool v_ack)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CCONbits.ACKDT = !v_ack;
    p_i2c->I2CCONbits.ACKEN = 1;
}

bool i2c_is_ack_send(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    return (!p_i2c->I2CCONbits.ACKEN);
}

bool i2c_is_ack_received(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    return (!p_i2c->I2CSTATbits.ACKSTAT);
}

bool i2c_send_byte(I2C_MODULE id, uint8_t data)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    if (!p_i2c->I2CSTATbits.TBF)
    {
        p_i2c->I2CTX = data;
        return 0;
    }
    return 1;
}

bool i2c_is_byte_transmitted(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    return (!p_i2c->I2CSTATbits.TRSTAT);
}

bool i2c_is_busy(I2C_MODULE id)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    return (p_i2c->I2CCONbits.SEN || p_i2c->I2CCONbits.PEN || p_i2c->I2CCONbits.RSEN || p_i2c->I2CCONbits.RCEN || p_i2c->I2CCONbits.ACKEN || p_i2c->I2CSTATbits.TRSTAT);
}

void i2c_set_slave_address(I2C_MODULE id, uint32_t address, uint32_t mask, I2C_ADDRESS_CONFIG mode)
{
    I2C_REGISTERS * p_i2c = (I2C_REGISTERS *) I2cModules[id];
    p_i2c->I2CADD = address;
    p_i2c->I2CMSK = mask;
    p_i2c->I2CCONSET = mode;
}

void i2c_interrupt_handler(I2C_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    if (serial_event_handler[id] != NULL)
    {
        (*serial_event_handler[id])(id, evt_type, data);
    }
}

I2C_STATE_MACHIN i2c_master_state_machine(I2C_PARAMS *var, I2C_FUNCTIONS *fct)
{
    I2C_STATE_MACHIN ret;
    
    if (var->bus_management_params.is_running)
    {
        if (!i2c_is_busy(var->module))
        {
            switch (var->state_machine.index)
            {
                case _HOME:

                    var->state_machine.index = _START;
                    break;

                case _START:

                    SET_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_LSB);
                    (var->is_16bits_address_reg) ? SET_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_MSB) : 0;
                    (var->data_access.read_write_type) ? SET_BIT(var->flags, I2C_READ_SEQUENCE) : 0;
                    
                    i2c_start(var->module);
                    var->state_machine.index = (var->data_access.general_call_request) ? _GENERAL_CALL_ADDRESS : _SLAVE_ADDRESS_WRITE;
                    break;

                case _RESTART:

                    i2c_restart(var->module);
                    var->state_machine.index = _SLAVE_ADDRESS_READ;
                    break;
                    
                case _GENERAL_CALL_ADDRESS:
                    
                    if (!i2c_send_byte(var->module, (0x00 & 0xfe) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_LSB);
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_MSB);
                        CLR_BIT(var->flags, I2C_READ_SEQUENCE);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _SLAVE_ADDRESS_WRITE:

                    if (!i2c_send_byte(var->module, (var->slave_address & 0xfe) & 0xff))
                    {
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _SLAVE_ADDRESS_READ:

                    if (!i2c_send_byte(var->module, (var->slave_address | 0x01) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_READ_SEQUENCE);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _ADDRESS_REGISTER_MSB:

                    if (!i2c_send_byte(var->module, (var->data_access.address_register >> 8) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_MSB);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _ADDRESS_REGISTER_LSB:

                    if (!i2c_send_byte(var->module, (var->data_access.address_register >> 0) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_LSB);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _WRITE_BYTE:

                    if (!i2c_send_byte(var->module, var->data_access.p[var->data_access.address_register + var->data_access.index - 1]))
                    {
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _READ_BYTE_SEQ1:

                    i2c_receiver_active_sequence(var->module);
                    var->state_machine.index = _READ_BYTE_SEQ2;
                    break;

                case _READ_BYTE_SEQ2:

                    if (!i2c_get_byte(var->module, &var->data_access.p[var->data_access.address_register + var->data_access.index - 1]))
                    {
                        if (++var->data_access.index > var->data_access.length)
                        {
                            i2c_send_ack(var->module, 0);
                            var->state_machine.index = _STOP;
                        }
                        else
                        {
                            i2c_send_ack(var->module, 1);
                            var->state_machine.index = _READ_BYTE_SEQ1;
                        }
                    }
                    break;

                case _WAIT_AND_VERIFY:

                    if (i2c_is_byte_transmitted(var->module))
                    {
                        if (i2c_is_ack_received(var->module))
                        {
                            if (var->flags > 0)
                            {
                                if (GET_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_MSB))
                                {
                                    var->state_machine.index = _ADDRESS_REGISTER_MSB;
                                }
                                else if (GET_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_LSB))
                                {
                                    var->state_machine.index = _ADDRESS_REGISTER_LSB;
                                }
                                else if (GET_BIT(var->flags, I2C_READ_SEQUENCE))
                                {
                                    var->state_machine.index = _RESTART;
                                }
                            }
                            else
                            {
                                var->state_machine.index = (var->data_access.read_write_type) ? (_READ_BYTE_SEQ1) : (_WRITE_BYTE);
                                if (++var->data_access.index > var->data_access.length)
                                {
                                    var->state_machine.index = _STOP;
                                }
                            }
                        }
                        else
                        {
                            var->state_machine.index = _FAIL;                            
                        }
                    }
                    break;

                case _STOP:

                    i2c_stop(var->module);
                    var->state_machine.index = _HOME;
                    var->state_machine.tick = mGetTick();
                    var->bus_management_params.is_running = false;
                    var->bus_management_params.tick = mGetTick();
                    break;
                    
                case _FAIL:

                    i2c_stop(var->module);
                    var->state_machine.index = _HOME;
                    var->state_machine.tick = mGetTick();
                    var->bus_management_params.is_running = false;
                    var->bus_management_params.tick = mGetTick();
                    var->fail_count++;
                    break;

                default:
                    break;
            }
            ret = var->state_machine.index;
        }
        else
        {
            ret = _BUS_I2C_BUSY;
        }
    } 
    else
    {
        ret = _BUS_MANAGEMENT_BUSY;
    }
    
    if (fct->active_function == 0xff)
    {
        uint8_t j;
        for (j = 0 ; j < fct->maximum_functions ; j++)
        {
            if (GET_BIT(fct->flags, j))                    
            {
                fct->active_function = j;
                break;
            }
        }
    }

    if (fct->active_function < fct->maximum_functions)
    {
        if (ret == _STOP)
        {
            CLR_BIT(fct->flags, fct->active_function);
            fct->active_function = 0xff;
        }
        else if (ret == _START)
        {
            var->data_access.general_call_request   = fct->functions_tab[fct->active_function].general_call_request;
            var->data_access.read_write_type        = fct->functions_tab[fct->active_function].read_write_type;
            var->data_access.address_register       = fct->functions_tab[fct->active_function].address_register;            
            var->data_access.index                  = 0;
            var->data_access.length                 = fct->functions_tab[fct->active_function].length;
        }
    }
    
    return ret;
}
