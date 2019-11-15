/*********************************************************************
*	LIN Bus driver
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               06/03/2019              - Initial release
*********************************************************************/

#include "../PLIB.h"

/*******************************************************************************
 * This LIN_EVENT array contains 2 informations:
 * is_data_receive:     This variable is a boolean which indicates that a new data 
 *                      has been received by the UART module in the event handler. 
 * data:                This variable contains the data received by the UART module
 *                      in the event handler. It is always 8-bits length. 
 *****************************************************************************/
static lin_event_t lin_event_tab[UART_NUMBER_OF_MODULES] = {0};

/*******************************************************************************
 * Function:
 *   static void lin_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
 *
 * Description:
 *   This routine is the event handler called by the interrupt routine (only if enable).
 *   To use this event handler, it must be passed as parameter in the uart_init
 *   function (initialization sequence). Do not forget to enable the IRQ(s). 
 *   If you do not want to use it then passed NULL as parameter and disable IRQ(s). 
 *
 * Parameters:
 *   id:        Identifier of the UART module which generates the event (ex: UART2)
 *   evt_type:  Type of event (see. IRQ_EVENT_TYPE for UART bus)
 *   data:      The data read by the UART module (data will be always 8-bits size for
 *              the LIN driver but for compatibility with other serial bus, the variable 
 *              is a 32-bits type)
 ******************************************************************************/
static void lin_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_UART_ERROR:
            
            break;
            
        case IRQ_UART_RX:
            
            lin_event_tab[id].data = data;
            lin_event_tab[id].is_data_receive = true;
            break;
            
        case IRQ_UART_TX:
            
            break;
            
    }
}

/*******************************************************************************
 * Function:
 *      static uint8_t lin_get_id_with_parity(uint8_t id)
 *
 * Description:
 *      This local routine is used to complement the ID field with the 2 parity 
 *      bits of the identifier p_frame. 
 *      The LIN ID p_frame is always 6-bits length (useful data) with 2 MSB parity 
 *      bits (to get the 8-bits data length). 
 *      See the ID field in details (MSB to LSB):
 *      P1 - P0 - id.5 - id.4 - id.3 - id.2 - id.1 - id.0
 *
 * Parameters:
 *      id:     This parameter is the identifier (with or without parity) of
 *              the LIN p_frame (ex. 0x3d)
 *
 * Return:
 *      The value returned is the full identifier (useful data + parity bits).
 ******************************************************************************/
static uint8_t lin_get_id_with_parity(uint8_t id)
{
    return ((id & 0x3f) | ((((id >> 0) & 1) ^ ((id >> 1) & 1) ^ ((id >> 2) & 1) ^ ((id >> 4) & 1)) << 6) | (!(((id >> 1) & 1) ^ ((id >> 3) & 1) ^ ((id >> 4) & 1) ^ ((id >> 5) & 1)) << 7));
}

/*******************************************************************************
 * Function: 
 *      LIN_STATE_MACHINE lin_master_deamon(LIN_PARAMS *var)
 * 
 * Description:
 *      This routine is the main state machine for managing a LIN bus (master 
 *      mode only). It can be used by external component drivers as well as
 *      by the user directly. 
 * 
 * Parameters:
 *      *var: A lin_params_t pointer used by the user/driver to manage the state machine.
 * 
 * Return:
 *      LIN_STATE_MACHINE (see the enumeration for more details).
 ******************************************************************************/
LIN_STATE_MACHINE lin_master_deamon(lin_params_t *var)
{
    uint8_t i = 0;
    
    if (!var->is_init_done)
    {
        uart_init(var->uart_module, lin_event_handler, IRQ_UART_RX, UART_BAUDRATE_19200, UART_STD_PARAMS);
        
        if (var->chip_enable._port > 0)
        {
            ports_reset_pin_output(var->chip_enable);
            ports_set_bit(var->chip_enable);
        }
                
        var->state_machine.tick = mGetTick();
        var->is_init_done = true;
    }
            
    if ((var->state_machine.current_index >= _LIN_HEADER_BREAK) && (var->state_machine.current_index <= _LIN_WAIT_AND_READBACK))
    {
        if (mTickCompare(var->state_machine.tick) >= ((var->p_frame[var->current_selected_p_frame]->length == 2) ? (LIN_BUS_TIMING_MAX_2_BYTES) : ((var->p_frame[var->current_selected_p_frame]->length == 4) ? (LIN_BUS_TIMING_MAX_4_BYTES) : LIN_BUS_TIMING_MAX_8_BYTES)))
        {
            var->state_machine.current_index = _LIN_TIMING_FAIL;
        }
    }

    switch (var->state_machine.current_index)
    {
        case _LIN_HOME:

            for ( ; i < var->number_of_p_frame ; i++)
            {
                if (var->p_frame[i]->force_transfer.execute)
                {
                    var->p_frame[i]->force_transfer.execute = false;
                    var->p_frame[i]->is_busy = true;
                    var->current_selected_p_frame = i;
                    break;
                }
                else if (var->p_frame[i]->periodicity > LIN_NOT_PERIODIC)
                {
                    if (mTickCompare(var->p_frame[i]->__tick) >= var->p_frame[i]->periodicity)
                    {
                        mUpdateTick(var->p_frame[i]->__tick);
                        var->p_frame[i]->is_busy = true;
                        var->current_selected_p_frame = i;
                        break;
                    }
                }
            }
            
            if (i == var->number_of_p_frame)
            {
                i = 0;
            }
            
            if (var->current_selected_p_frame != 0xff)
            {
                if (mTickCompare(var->state_machine.tick) >= LIN_BUS_TIMING_SLEEP)
                {
                    var->state_machine.current_index = _LIN_WAKE_UP;
                }
                else
                {
                    var->state_machine.current_index = _LIN_HEADER_BREAK;
                }
                lin_event_tab[var->uart_module].is_data_receive = false;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_WAKE_UP:

            if (!uart_send_break(var->uart_module))
            {
                var->state_machine.current_index = _LIN_WAKE_UP_WAIT_100MS;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_WAKE_UP_WAIT_100MS:

            if (mTickCompare(var->state_machine.tick) >= TICK_100MS)
            {
                lin_event_tab[var->uart_module].is_data_receive = false;
                var->state_machine.current_index = _LIN_HEADER_BREAK;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_HEADER_BREAK:

            if (!uart_send_break(var->uart_module))
            {
                var->state_machine.data_readback = 0x00;
                var->state_machine.next_index = _LIN_HEADER_SYNC;
                var->state_machine.current_index = _LIN_WAIT_AND_READBACK;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_HEADER_SYNC:

            if (!uart_send_data(var->uart_module, 0x55))
            {
                var->p_frame[var->current_selected_p_frame]->id = lin_get_id_with_parity(var->p_frame[var->current_selected_p_frame]->id);
                var->state_machine.data_readback = 0x55;
                var->state_machine.next_index = _LIN_HEADER_ID;
                var->state_machine.current_index = _LIN_WAIT_AND_READBACK;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_HEADER_ID:

            if (!uart_send_data(var->uart_module, var->p_frame[var->current_selected_p_frame]->id))
            {
                var->state_machine.data_readback = var->p_frame[var->current_selected_p_frame]->id;
                var->state_machine.next_index = (var->p_frame[var->current_selected_p_frame]->read_write_type) ? _LIN_TX_DATA : _LIN_RX_DATA;
                if (var->p_frame[var->current_selected_p_frame]->data_length != LIN_AUTO_DATA_LENGTH)
                {
                    var->p_frame[var->current_selected_p_frame]->length = var->p_frame[var->current_selected_p_frame]->data_length;
                }
                else
                {
                    var->p_frame[var->current_selected_p_frame]->length = (((var->p_frame[var->current_selected_p_frame]->id & 0x3f) <= 0x1f) ? (2) : (((var->p_frame[var->current_selected_p_frame]->id & 0x3f) <= 0x2f) ? (4) : (8)));
                }                
                var->p_frame[var->current_selected_p_frame]->__data_index = 0;
                var->p_frame[var->current_selected_p_frame]->checksum = ((var->lin_version == LIN_VERSION_2_X) && ((var->p_frame[var->current_selected_p_frame]->id & 0x3f) < 60)) ? (var->p_frame[var->current_selected_p_frame]->id) : 0;
                var->state_machine.current_index = _LIN_WAIT_AND_READBACK;
                var->state_machine.tick = mGetTick();                    
            }
            break;

        case _LIN_TX_DATA:

            if (!uart_send_data(var->uart_module, var->p_frame[var->current_selected_p_frame]->data[var->p_frame[var->current_selected_p_frame]->__data_index]))
            {
                var->state_machine.data_readback = var->p_frame[var->current_selected_p_frame]->data[var->p_frame[var->current_selected_p_frame]->__data_index];
                var->p_frame[var->current_selected_p_frame]->checksum += var->p_frame[var->current_selected_p_frame]->data[var->p_frame[var->current_selected_p_frame]->__data_index];
                var->p_frame[var->current_selected_p_frame]->checksum -= (var->p_frame[var->current_selected_p_frame]->checksum > 255) ? 255 : 0;
                if (++var->p_frame[var->current_selected_p_frame]->__data_index >= var->p_frame[var->current_selected_p_frame]->length)
                {
                    var->p_frame[var->current_selected_p_frame]->checksum ^= 255;
                    var->state_machine.next_index = _LIN_TX_CHKSM;
                }
                var->state_machine.current_index = _LIN_WAIT_AND_READBACK;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_RX_DATA:

            if (lin_event_tab[var->uart_module].is_data_receive)
            {
                lin_event_tab[var->uart_module].is_data_receive = false;
                var->p_frame[var->current_selected_p_frame]->data[var->p_frame[var->current_selected_p_frame]->__data_index] = lin_event_tab[var->uart_module].data;
                var->p_frame[var->current_selected_p_frame]->checksum += var->p_frame[var->current_selected_p_frame]->data[var->p_frame[var->current_selected_p_frame]->__data_index];
                var->p_frame[var->current_selected_p_frame]->checksum -= (var->p_frame[var->current_selected_p_frame]->checksum > 255) ? 255 : 0;
                if (++var->p_frame[var->current_selected_p_frame]->__data_index >= var->p_frame[var->current_selected_p_frame]->length)
                {
                    var->p_frame[var->current_selected_p_frame]->checksum ^= 255;
                    var->state_machine.current_index = _LIN_RX_CHKSM;
                }
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_TX_CHKSM:

            if (!uart_send_data(var->uart_module, var->p_frame[var->current_selected_p_frame]->checksum))
            {
                var->state_machine.data_readback = var->p_frame[var->current_selected_p_frame]->checksum;
                var->state_machine.next_index = _LIN_END;
                var->state_machine.current_index = _LIN_WAIT_AND_READBACK;
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_RX_CHKSM:

            if (lin_event_tab[var->uart_module].is_data_receive)
            {
                lin_event_tab[var->uart_module].is_data_receive = false;
                if (var->p_frame[var->current_selected_p_frame]->checksum == lin_event_tab[var->uart_module].data)
                {                    
                    var->state_machine.current_index = _LIN_END;
                }
                else
                {
                    var->state_machine.current_index = _LIN_RX_CHKSM_FAIL;
                }
                var->state_machine.tick = mGetTick();
            }
            break;

        case _LIN_WAIT_AND_READBACK:

            if (lin_event_tab[var->uart_module].is_data_receive)
            {
                lin_event_tab[var->uart_module].is_data_receive = false;
                if (lin_event_tab[var->uart_module].data == var->state_machine.data_readback)
                {
                    var->state_machine.current_index = var->state_machine.next_index;
                    var->state_machine.tick = mGetTick();
                }
                else
                {
                    var->state_machine.current_index = _LIN_READBACK_FAIL;
                }
            }
            break;

        case _LIN_RX_CHKSM_FAIL:

            var->errors.rx_chksm++;
            var->p_frame[var->current_selected_p_frame]->is_busy = false;
            var->p_frame[var->current_selected_p_frame]->errors.is_occurs = true;
            var->p_frame[var->current_selected_p_frame]->errors.rx_chksm++;
            var->state_machine.current_index = _LIN_HOME;
            var->state_machine.tick = mGetTick();
            var->current_selected_p_frame = 0xff;
            break;

        case _LIN_READBACK_FAIL:

            var->errors.readback++;
            var->p_frame[var->current_selected_p_frame]->is_busy = false;
            var->p_frame[var->current_selected_p_frame]->errors.is_occurs = true;
            var->p_frame[var->current_selected_p_frame]->errors.readback++;
            var->state_machine.current_index = _LIN_HOME;
            var->state_machine.tick = mGetTick();
            var->current_selected_p_frame = 0xff;
            break;

        case _LIN_TIMING_FAIL:

            var->errors.timing++;
            var->p_frame[var->current_selected_p_frame]->is_busy = false;
            var->p_frame[var->current_selected_p_frame]->errors.is_occurs = true;
            var->p_frame[var->current_selected_p_frame]->errors.timing++;
            var->state_machine.current_index = _LIN_HOME;
            var->state_machine.tick = mGetTick();
            var->current_selected_p_frame = 0xff;
            break;

        case _LIN_END:

            var->p_frame[var->current_selected_p_frame]->is_busy = false;
            var->p_frame[var->current_selected_p_frame]->is_updated = true;
            var->state_machine.current_index = _LIN_HOME;
            var->state_machine.tick = mGetTick();
            var->current_selected_p_frame = 0xff;
            break;
    }

    return var->state_machine.current_index;
}

/*******************************************************************************
 * Function:
 *      uint8_t lin_force_transfer(lin_frame_params_t *frame)
 *
 * Description:
 *      This routine can be used when a "force_transfer" is requested AND also 
 *      when we want to have a feedback on its status ( >0: "on going" / 0: Finished).
 *      This is not mandatory to use it. A "force_transfer" can be executed just by
 *      setting the flag "force_transfer.execute" to '1' of a LIN frame (Tx or Rx). 
 *
 * Parameters:
 *      *frame: This is a pointer of the frame for which you want to execute 
 *              a "force_transfer".
 *
 * Return:
 *      It returns the status of the "force_transfer": ( >0: "on going" / 0: Finished).
 ******************************************************************************/
uint8_t lin_force_transfer(lin_frame_params_t *frame)
{  
    
    switch (frame->force_transfer.sm.index)
    {       
        case 0:
            
            frame->force_transfer.sm.index = 1;
            
        case 1:
            
            if (!frame->is_busy)
            {
                frame->force_transfer.execute = true;
                frame->is_updated = false;
                frame->errors.is_occurs = false;
                frame->force_transfer.sm.index = 2;
            }
            break;
            
        case 2:
            
            if (frame->is_updated || frame->errors.is_occurs)
            {
                frame->force_transfer.sm.index = 0;
            }
            break;
    }
    
    return frame->force_transfer.sm.index;
}
