/*********************************************************************
*	I2C modules
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		04/12/2018		- Initial release
*       feb. 2019       - Add comments
*                       - Add i2c_master_state_machine function
*                       - Modifications of I2C_PARAMS in order to add
*                       the new features of 'i2c_master_state_machine'. 
*********************************************************************/

#include "../PLIB.h"

extern const i2c_registers_t * I2cModules[];
const i2c_registers_t * I2cModules[] =
{
	(i2c_registers_t*)_I2C1_BASE_ADDRESS,
	(i2c_registers_t*)_I2C2_BASE_ADDRESS,
	(i2c_registers_t*)_I2C3_BASE_ADDRESS,
	(i2c_registers_t*)_I2C4_BASE_ADDRESS,
	(i2c_registers_t*)_I2C5_BASE_ADDRESS
};
static uint32_t real_frequency_tab[I2C_NUMBER_OF_MODULES] = {0};
static i2c_event_handler_t i2c_event_handler[I2C_NUMBER_OF_MODULES] = {NULL};

/*******************************************************************************
 * Function: 
 *      void i2c_init_as_master(    I2C_MODULE id, 
 *                                  i2c_event_handler_t evt_handler,
 *                                  IRQ_EVENT_TYPE event_type_enable,
 *                                  I2C_FREQUENCY frequency,
 *                                  I2C_CONFIGURATION configuration)
 * 
 * Description:
 *      This routine is used to initialize an i2c module.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      evt_handler: The handler (function) to call when an interruption occurs.
 *      event_type_enable: The event(s) you want to enable by interruption (See. 
 *      IRQ_EVENT_TYPE). 
 *      frequency: The desire frequency (see. I2C_FREQUENCY).
 *      configuration: Params follow the I2C_CONFIGURATION enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_init_as_master(    I2C_MODULE id, 
                            i2c_event_handler_t evt_handler,
                            IRQ_EVENT_TYPE event_type_enable,
                            I2C_FREQUENCY frequency,
                            I2C_CONFIGURATION configuration)
{
    i2c_event_handler[id] = evt_handler;
    irq_init(IRQ_I2C1B + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_I2C_BUS_COLISION) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_i2c_priority(id));
    irq_init(IRQ_I2C1S + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_I2C_SLAVE) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_i2c_priority(id));
    irq_init(IRQ_I2C1M + id, ((evt_handler != NULL) && ((event_type_enable & IRQ_I2C_MASTER) > 0)) ? IRQ_ENABLED : IRQ_DISABLED, irq_i2c_priority(id));
    
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
    i2c_enable(id, ON);
}

/*******************************************************************************
 * Function: 
 *      void i2c_enable(I2C_MODULE id, bool enable)
 * 
 * Description:
 *      This routine is used to enable the i2c module.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      enable: 1: enable / 0: disable
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_enable(I2C_MODULE id, bool enable)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.BUS_ON = enable;
}

/*******************************************************************************
 * Function: 
 *      void i2c_configuration(I2C_MODULE id, I2C_CONFIGURATION configuration)
 * 
 * Description:
 *      This routine is used to configure the i2c module.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      configuration: Params follow the I2C_CONFIGURATION enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_configuration(I2C_MODULE id, I2C_CONFIGURATION configuration)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCONSET = configuration;
}

/*******************************************************************************
 * Function: 
 *      void i2c_set_frequency(I2C_MODULE id, I2C_FREQUENCY frequency)
 * 
 * Description:
 *      This routine is used to configure the i2c module frequency.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      frequency: The desire frequency (see. I2C_FREQUENCY).
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_set_frequency(I2C_MODULE id, I2C_FREQUENCY frequency)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    float v_brg = (float) (((((100000 / (2 * frequency)) - 10)*(PERIPHERAL_FREQ / 1000000)) / 100) - 2);
    float v_real_freq = (float) (500000 / ((((v_brg + 2) * 1000) / (PERIPHERAL_FREQ / 1000000) + 104) / 1000));
    p_i2c->I2CBRG = (uint32_t ) v_brg;
    real_frequency_tab[id] = (uint32_t) v_real_freq;
}

/*******************************************************************************
 * Function: 
 *      void i2c_start(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to send a "start" condition. It is used by the 
 *      master to establish a frame transmission. 
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_start(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.SEN = 1;
}

/*******************************************************************************
 * Function: 
 *      void i2c_restart(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to send a "restart" condition. It is used by the 
 *      master to establish a new frame transmission without to do a "STOP" and
 *      a "START". It is often used for reading operation:
 *      First step the master send a write request in order to fix an address
 *      register.
 *      Second, the master send a "RESTART" in order to send a read request.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_restart(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.RSEN = 1;
}

/*******************************************************************************
 * Function: 
 *      void i2c_stop(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to send a "stop" condition. It is used by the 
 *      master to close a frame transmission.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_stop(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.PEN = 1;
}

/*******************************************************************************
 * Function: 
 *      void i2c_receiver_active_sequence(I2C_MODULE id)
 * 
 * Description:
 *      This routine enables the i2c module to receive data from the I2C device 
 *      on the bus. Thus, the master generates clock in order to allow the slave 
 *      to return its data byte.
 *      This bis is automatically clear by module at end of 8 bits receive
 *      data byte.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_receiver_active_sequence(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.RCEN = 1;
}

/*******************************************************************************
 * Function: 
 *      bool i2c_get_byte(I2C_MODULE id, uint8_t *data)
 * 
 * Description:
 *      This routine is used to get a data store in the i2c receive buffer
 *      register. A "receive overflow - I2COV" can occurs if a byte is received 
 *      while the I2CxRCV register is still holding the previous byte.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      *data: A uint8_t pointer to store the receive data. A receive data
 *      cannot be different as a uint8_t type.
 * 
 * Return:
 *      0: Data successfully receive.
 *      1: No data receive.
 ******************************************************************************/
bool i2c_get_byte(I2C_MODULE id, uint8_t *data)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    if (p_i2c->I2CSTAT.RBF)
    {
        *data = (uint8_t) p_i2c->I2CRX;
        return 0;
    }
    return 1;
}

/*******************************************************************************
 * Function: 
 *      void i2c_send_ack(I2C_MODULE id, bool v_ack)
 * 
 * Description:
 *      This routine is used to send an acknowledgment. 
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      v_ack: 0: /NACK , 1: ACK
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_send_ack(I2C_MODULE id, bool v_ack)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CCON.ACKDT = !v_ack;
    p_i2c->I2CCON.ACKEN = 1;
}

/*******************************************************************************
 * Function: 
 *      bool i2c_is_ack_send(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to verify that the ACK has been send.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      0: ACK has not been send.
 *      1: ACK has been send.
 ******************************************************************************/
bool i2c_is_ack_send(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    return (!p_i2c->I2CCON.ACKEN);
}

/*******************************************************************************
 * Function: 
 *      bool i2c_is_ack_received(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to verify if an ACK bit has been received.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      0: /NACK receive.
 *      1: ACK receive.
 ******************************************************************************/
bool i2c_is_ack_received(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    return (!p_i2c->I2CSTAT.ACKSTAT);
}

/*******************************************************************************
 * Function: 
 *      bool i2c_send_byte(I2C_MODULE id, uint8_t data)
 * 
 * Description:
 *      This routine is used to send a data over the I2C bus. A write collision
 *      can occurs - IWCOL - if the bus is not ready for transmission. 
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      data: A uint8_t variable containing the data to send.
 * 
 * Return:
 *      0: Data successfully transmitted.
 *      1: No data transmitted.
 ******************************************************************************/
bool i2c_send_byte(I2C_MODULE id, uint8_t data)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    if (!p_i2c->I2CSTAT.TBF)
    {
        p_i2c->I2CTX = data;
        return 0;
    }
    return 1;
}

/*******************************************************************************
 * Function: 
 *      bool i2c_is_byte_transmitted(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to verify that a byte has been successfully 
 *      transmitted.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      0: Data not successfully transmitted.
 *      1: Data successfully transmitted.
 ******************************************************************************/
bool i2c_is_byte_transmitted(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    return (!p_i2c->I2CSTAT.TRSTAT);
}

/*******************************************************************************
 * Function: 
 *      bool i2c_is_busy(I2C_MODULE id)
 * 
 * Description:
 *      This routine is used to verify that the I2C bus is ready for next 
 *      operations.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 * 
 * Return:
 *      0: The I2C bus is ready to use.
 *      1: The I2C bus is busy.
 ******************************************************************************/
bool i2c_is_busy(I2C_MODULE id)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    return (p_i2c->I2CCON.SEN || p_i2c->I2CCON.PEN || p_i2c->I2CCON.RSEN || p_i2c->I2CCON.RCEN || p_i2c->I2CCON.ACKEN || p_i2c->I2CSTAT.TRSTAT);
}

/*******************************************************************************
 * Function: 
 *      void i2c_set_slave_address(I2C_MODULE id, uint32_t address, uint32_t mask, I2C_ADDRESS_CONFIG mode)
 * 
 * Description:
 *      This routine is used to verify that the I2C bus is ready for next 
 *      operations.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      address: The slave address.
 *      mask: The mask assign to the slave address for answering to master device
 *      addresses. 
 *      mode: Params follow the I2C_ADDRESS_CONFIG enumeration.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_set_slave_address(I2C_MODULE id, uint32_t address, uint32_t mask, I2C_ADDRESS_CONFIG mode)
{
    i2c_registers_t * p_i2c = (i2c_registers_t *) I2cModules[id];
    p_i2c->I2CADD = address;
    p_i2c->I2CMSK = mask;
    p_i2c->I2CCONSET = mode;
}

/*******************************************************************************
 * Function: 
 *      void i2c_interrupt_handler(I2C_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
 * 
 * Description:
 *      This routine is called when an interruption occured. This interrupt 
 *      handler calls the user _event_handler (if existing) otherwise do nothing.
 * 
 * Parameters:
 *      id: The I2C module you want to use.
 *      evt_type: The type of event (MASTER, SLAVE, BUS_COLLISION...). See IRQ_EVENT_TYPE.
 *      data: The data (in case of a reception) read in the interruption.
 * 
 * Return:
 *      none
 ******************************************************************************/
void i2c_interrupt_handler(I2C_MODULE id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    if (i2c_event_handler[id] != NULL)
    {
        (*i2c_event_handler[id])(id, evt_type, data);
    }
}

/*******************************************************************************
 * Function: 
 *      I2C_STATE_MACHIN i2c_master_state_machine(I2C_PARAMS *var, I2C_FUNCTIONS *fct)
 * 
 * Description:
 *      This routine is the main state machine for managing the I2C protocol (for
 *      master mode only). It is used by external component drivers thanks to 
 *      the I2C_PARAMS and I2C_FUNCTIONS pointers. 
 * 
 * Parameters:
 *      *var: A I2C_PARAMS pointer used by the driver to manage the state machine.
 *      *fct: A I2C_FUNCTIONS pointer used by the driver to load parameters 
 *      in I2C_PARAMS at the right time.
 * 
 * Return:
 *      I2C_STATE_MACHIN (see the enumeration for more details).
 ******************************************************************************/
I2C_STATE_MACHINE i2c_master_state_machine(I2C_PARAMS *var, I2C_FUNCTIONS *fct)
{
    I2C_STATE_MACHINE ret;
    
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

                    if (!i2c_send_byte(var->module, (var->data_access.address_register_device >> 8) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_MSB);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _ADDRESS_REGISTER_LSB:

                    if (!i2c_send_byte(var->module, (var->data_access.address_register_device >> 0) & 0xff))
                    {
                        CLR_BIT(var->flags, I2C_SEND_ADDRESS_REGISTERS_LSB);
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _WRITE_BYTE:

                    if (!i2c_send_byte(var->module, var->data_access.p[var->data_access.address_register_pic32 + var->data_access.index - 1]))
                    {
                        var->state_machine.index = _WAIT_AND_VERIFY;
                    }
                    break;

                case _READ_BYTE_SEQ1:

                    i2c_receiver_active_sequence(var->module);
                    var->state_machine.index = _READ_BYTE_SEQ2;
                    break;

                case _READ_BYTE_SEQ2:

                    if (!i2c_get_byte(var->module, &var->data_access.p[var->data_access.address_register_pic32 + var->data_access.index - 1]))
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
            var->data_access.general_call_request       = fct->functions_tab[fct->active_function].general_call_request;
            var->data_access.read_write_type            = fct->functions_tab[fct->active_function].read_write_type;
            var->data_access.address_register_device    = fct->functions_tab[fct->active_function].address_register_device;    
            var->data_access.address_register_pic32     = fct->functions_tab[fct->active_function].address_register_pic32;    
            var->data_access.index                      = 0;
            var->data_access.length                     = fct->functions_tab[fct->active_function].length;
        }
    }
    
    return ret;
}
