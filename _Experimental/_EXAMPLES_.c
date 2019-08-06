 /*********************************************************************
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		02/06/2017		- Initial release
*                       - Add E_EEPROM example
* 
*   Description:
*   ------------ 
*********************************************************************/

#include "../PLIB.h"

void _EXAMPLE_TICK_FUNCTIONS()
{
    static uint8_t count;
    static uint64_t tick = 0;
    static uint64_t get_time;
    static bool switch_update_tick = false;
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            count = 0;
            mUpdateTick(tick);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            if (mTickCompare(tick) >= TICK_1MS)
            {
                if (!switch_update_tick)
                {
                    mUpdateTick(tick);
                }
                else
                {
                    mUpdateTick_withCathingUpTime(tick, TICK_1MS);
                }
                
                if (count == 0)
                {
                    get_time = mGetTick();
                }
                else if (count == 50)
                {
                    Delay_ms(20);
                }
                else if (count == 100)
                {
                    get_time = (mGetTick() - get_time) / TICK_1US;
                    if (!switch_update_tick)
                    {
                        LOG("Time Update Tick: %d", get_time);    
                    }
                    else
                    {
                        LOG("Time Update Tick with Catching Up Time: %d", get_time);    
                    }
                }
                
                if (++count > 100)
                {
                    count = 0;
                    switch_update_tick = !switch_update_tick;
                }
            }
            break;
    } 
}

static void _example_timer_event_handler(uint8_t id)
{
    if (mGetIO(LED2))
    {
        mUpdateLedStatusD2(OFF);
    }
    else
    {
        mUpdateLedStatusD2(ON);
    }
}

void _EXAMPLE_TIMER()
{
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            timer_init_2345_us(TIMER4, _example_timer_event_handler, TMR_ON | TMR_SOURCE_INT | TMR_IDLE_CON | TMR_GATE_OFF, 100000);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            break;
    } 
}

/*********************************************************************************************
 * Start of _EXAMPLE_DMA_RAM_TO_RAM()
 * -------------------------------------------------------------------------------------------

    The DMA RAM to RAM example execute a data transfer from a source buffer to a destination
    buffer. The total data to transfer is 200 bytes length but we choose to transfer 20
    bytes per 20 bytes (cell block size) in this example. (We can choose a cell size of 200 
    bytes of course to execute the transfer in one time but this isn't the goal of this example).
    The sizes source, destination and cell are maximum DMA_MAX_TRANSFER_SIZE.
    The interruption are set for TRANSFER_ABORD and BLOCK_TRANSFER only. The BLOCK_TRANSFER
    interrupt will be set when all data (200 bytes) will be transfered.
    Each time than 20 bytes are transfered (cell size) then a flag (CELL_TRANSFER) is set
    to tell the user that a new transfer can be operate (interruption are not be enabled for 
    this flag). Thus we will have 10 cells transfers for transferring the complete BLOCK in 
    this example. Because NO DMA events are set in this example, we have to FORCE the transfer 
    each time the DMA channel is free-to-use (cell block is transfered). 
 
 *********************************************************************************************/

static uint8_t buff_src[200] = {0};
static uint8_t buff_dst[200] = {0};
static void _example_dma_ram_to_ram_event_handler(uint8_t id, DMA_CHANNEL_FLAGS flags)
{
    if ((flags & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
    {
        if (buff_dst[78] == 78)
        {
            mUpdateLedStatusD2(ON);
        }
        dma_clear_flags(id, DMA_FLAG_BLOCK_TRANSFER_DONE);
    }
}

void _EXAMPLE_DMA_RAM_TO_RAM()
{
    static state_machine_t sm_example = {0};
    static DMA_CHANNEL_TRANSFER dma_tx = {buff_src, buff_dst, 200, 200, 20, 0x0000};
    static uint8_t loop_counter = 1;
    static uint8_t i;
    static DMA_MODULE dma_id;
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            mUpdateLedStatusD2(OFF);
            mUpdateLedStatusD3(OFF);
            
            dma_id = dma_get_free_channel();
            
            dma_init(   dma_id, 
                        _example_dma_ram_to_ram_event_handler, 
                        DMA_CONT_PRIO_1, 
                        DMA_INT_TRANSFER_ABORD | DMA_INT_BLOCK_TRANSFER_DONE, 
                        DMA_EVT_NONE, 
                        0xff, 
                        0xff);
            for (i = 0 ; i < 200 ; i++)
            {
                buff_src[i] = i;
            }
            dma_set_transfer(dma_id, &dma_tx, true, true);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
           
            if ((dma_get_flags(dma_id) & DMA_FLAG_CELL_TRANSFER_DONE) > 0)
            {
                loop_counter++;
                dma_force_transfer(dma_id);
                dma_clear_flags(dma_id, DMA_FLAG_CELL_TRANSFER_DONE);
            }

            if (loop_counter >= 10)
            {
                mUpdateLedStatusD3(ON);
            }
            else
            {
                mUpdateLedStatusD3(BLINK);
            }
            break;
    } 
}
/* -------------------------------------------------------------------------------------------
 * End of   _EXAMPLE_DMA_RAM_TO_RAM()
 *********************************************************************************************/

/*********************************************************************************************
 * Start of _EXAMPLE_DMA_UART()
 * -------------------------------------------------------------------------------------------

    The DMA UART example execute 2 different transfers. One from a RAM source to a SFR
    REG destination and a second from a SFR REG source to a RAM destination. In this code, 
    we implement a complete DMA example for UART1 (Tx is managing by DMA6 and Rx is managing by
    DMA7).
 
    The transmission will be manage by DMA6. We 'attach' a start event transfer to the DMA channel,
    that is to say that each time a UART1 transmission is done a new DMA cell block is transmitted
    and so on up to all data source is transmitted. At the end, a DMA_INT_BLOCK_TRANSFER_DONE is
    set, the event handler is called and the DMA channel turns off automatically (because there is
    no AUTO_ENABLE option for DMA0 Channel). The 2 events that can generate an interruption are
    DMA_INT_TRANSFER_ABORD and DMA_INT_BLOCK_TRANSFER_DONE.
    We have to FORCE the first transmission in order to generate the first _UART1_TX_IRQ event. Thus
    a new cell block will be automatically transmitted and so on...
 
    The reception will be manage by DMA7. The start event transfer is _UART1_RX_IRQ, that is to say
    that each time a data is received by the UART module, a DMA transfer will operate to store the
    data UART in a RAM buffer "buff_src".
    2 ways to generate a Block Transfer Done:
        - 200 bytes are received by the UART module (buff_src is fully updated).
        - A pattern match occurs.
    The Pattern Match mode is enabled (by setting the event DMA_EVT_ABORD_TRANSFER_ON_PATTERN_MATCH)
    with a pattern size of 2 bytes (pattern = 0xdead). If the reception contains the 2 bytes 0xde 
    and 0xad then the DMA Channel behavior is the same as a Block Transfer Done (interruption is set, 
    event handler is called...).
    The only events which can generate an interruption are DMA_INT_TRANSFER_ABORD and 
    DMA_INT_BLOCK_TRANSFER_DONE.
    This DMA Channel is set with DMA_CONT_AUTO_ENABLE in order to keep the channel active even after
    a Block Transfer Done or a Pattern Match (same behavior as Block Transfer Done). Thus we can
    continue to receive data and store in the RAM buffer.

    When using Pattern Match mode and a pattern is detected then a DMA_INT_BLOCK_TRANSFER_DONE
    is set. Interrupt can be called (if enable) and the DMA channel behavior is the same
    as Block Transfer Complete (DMA_INT_BLOCK_TRANSFER_DONE flag is set and DMA channel is
    disable).

    Configure the DMA channel in AUTO_ENABLE mode allow the channel to be always ENABLE even 
    after a BLOCK_TRANSFER_DONE. Thus it is not necessary to re-configure the channel
    with the dma_set_transfer routine. 

 *********************************************************************************************/
static void _example_dma_uart_event_handler(uint8_t id, DMA_CHANNEL_FLAGS flags)
{
    if ((flags & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
    {
        if (id == DMA6)
        {
            mToggleLedStatusD2();
        }
        else if (id == DMA7)
        {
            mToggleLedStatusD3();   
        }
        dma_clear_flags(id, DMA_FLAG_BLOCK_TRANSFER_DONE);
    }
    else if ((flags & DMA_FLAG_TRANSFER_ABORD) > 0)
    {
        // ...
        dma_clear_flags(id, DMA_FLAG_TRANSFER_ABORD);
    }
}

void _EXAMPLE_DMA_UART()
{
    static state_machine_t sm_example = {0};
    static UART_MODULE uart_id = UART1;
    static uint8_t i;
    static uint8_t buff_src[200] = {0};
    static DMA_CHANNEL_TRANSFER dma6_tx = {buff_src, NULL, 200, 1, 1, 0x0000};
    static DMA_CHANNEL_TRANSFER dma7_rx = {NULL, buff_src, 1, 200, 1, 0xdead};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            mUpdateLedStatusD2(OFF);
            mUpdateLedStatusD3(OFF);
            uart_init(  uart_id, NULL, IRQ_NONE, UART_BAUDRATE_2M, UART_STD_PARAMS);
            dma_init(   DMA6, 
                        _example_dma_uart_event_handler, 
                        DMA_CONT_PRIO_2, 
                        DMA_INT_TRANSFER_ABORD | DMA_INT_BLOCK_TRANSFER_DONE, 
                        DMA_EVT_START_TRANSFER_ON_IRQ, 
                        uart_get_tx_irq(uart_id), 
                        0xff);
            dma_init(   DMA7, 
                        _example_dma_uart_event_handler, 
                        DMA_CONT_PRIO_0 | DMA_CONT_PATTERN_2_BYTES | DMA_CONT_AUTO_ENABLE, 
                        DMA_INT_TRANSFER_ABORD | DMA_INT_BLOCK_TRANSFER_DONE, 
                        DMA_EVT_START_TRANSFER_ON_IRQ | DMA_EVT_ABORD_TRANSFER_ON_PATTERN_MATCH, 
                        uart_get_rx_irq(uart_id), 
                        0xff);
            dma6_tx.dst_start_addr = (void *) uart_get_tx_reg(uart_id);
            dma7_rx.src_start_addr = (void *) uart_get_rx_reg(uart_id);
            for (i = 0 ; i < 200 ; i++)
            {
                buff_src[i] = i;
            }
            dma_set_transfer(DMA6, &dma6_tx, true, true);
            dma_set_transfer(DMA7, &dma7_rx, true, false);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            // Do what you want...
            break;
    } 
}
/* -------------------------------------------------------------------------------------------
 * End of   _EXAMPLE_DMA_UART()
 *********************************************************************************************/

/*********************************************************************************************
 * Start of _EXAMPLE_DMA_SPI()
 * -------------------------------------------------------------------------------------------

 

 *********************************************************************************************/
static void _example_dma_spi_event_handler(uint8_t id, DMA_CHANNEL_FLAGS flags)
{
    if ((flags & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
    {
        if (id == DMA6)
        {
            mSetIO(__PE0);
            mToggleLedStatusD2();
        }
        else if (id == DMA7)
        {
            mToggleLedStatusD3();   
        }
        dma_clear_flags(id, DMA_FLAG_BLOCK_TRANSFER_DONE);
    }
    else if ((flags & DMA_FLAG_TRANSFER_ABORD) > 0)
    {
        // ...
        dma_clear_flags(id, DMA_FLAG_TRANSFER_ABORD);
    }
}

void _EXAMPLE_DMA_SPI()
{
    static uint8_t i;
    static uint8_t buff_src[20] = {0};
    static DMA_CHANNEL_TRANSFER dma6_tx = {buff_src, NULL, 20, 1, 1, 0x0000};
    static DMA_CHANNEL_TRANSFER dma7_rx = {NULL, buff_src, 1, 200, 1, 0x0000};
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            // Initialize chip select
            mInitIOAsOutput(__PE0);
            mSetIO(__PE0);
            
            spi_init(   SPI1, 
                        NULL, 
                        IRQ_NONE, 
                        10000000, 
                        SPI_CONF_MSTEN | SPI_CONF_FRMPOL_LOW | SPI_CONF_SMP_MIDDLE | SPI_CONF_MODE8 | SPI_CONF_CKP_HIGH | SPI_CONF_CKE_OFF);

            dma_init(   DMA6, 
                        _example_dma_spi_event_handler, 
                        DMA_CONT_PRIO_3, 
                        DMA_INT_TRANSFER_ABORD | DMA_INT_BLOCK_TRANSFER_DONE, 
                        DMA_EVT_START_TRANSFER_ON_IRQ, 
                        spi_get_tx_irq(SPI1), 
                        0xff);
            
            dma_init(   DMA7, 
                        NULL, 
                        DMA_CONT_PRIO_3, 
                        DMA_INT_NONE, 
                        DMA_EVT_START_TRANSFER_ON_IRQ, 
                        spi_get_rx_irq(SPI1), 
                        0xff);
            
            dma6_tx.dst_start_addr = (void *) spi_get_tx_reg(SPI1);
            dma7_rx.src_start_addr = (void *) spi_get_rx_reg(SPI1);
            dma_set_transfer(DMA6, &dma6_tx, true, false);
            dma_set_transfer(DMA7, &dma7_rx, true, false);
            
            for (i = 0 ; i < 200 ; i++)
            {
                buff_src[i] = i;
            }
            
            mClrIO(__PE0);
            dma_force_transfer(DMA6);
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            // Do what you want...
            break;
    } 
}
/* -------------------------------------------------------------------------------------------
 * End of   _EXAMPLE_DMA_SPI()
 *********************************************************************************************/

void _EXAMPLE_PWM()
{
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);            
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            // NOTE: PWM2 (OC2) and PWM3 (OC3) are assign to LED2 and LED3
            pwm_init(PWM1_T2_ON | PWM4_T2_ON | PWM5_T3_ON, 40000, 250000);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            switch (sw1.indice)
            {
                case 0:
                    
                    pwm_set_duty_cycle(PWM1, 0);
                    pwm_set_duty_cycle(PWM4, 1);
                    pwm_set_duty_cycle(PWM5, 2);
                    break;
                case 1:
                    
                    pwm_set_duty_cycle(PWM1, 50);
                    pwm_set_duty_cycle(PWM4, 100);
                    pwm_set_duty_cycle(PWM5, 150);
                    break;
                case 2:
                    
                    pwm_set_duty_cycle(PWM1, 255);
                    pwm_set_duty_cycle(PWM4, 254);
                    pwm_set_duty_cycle(PWM5, 253);
                    break;
                case 3:
                    
                    sw1.indice = 0;
                    break;
            }
            fu_switch(&sw1);
            break;
    } 
}

void _EXAMPLE_SOFTWARE_PWM()
{
    static state_machine_t sm_example = {0};
    SOFTWARE_PWM_DEF(spwm, TIMER5, SOFTWARE_PWM_FREQ_200HZ, SOFTWARE_PWM_RESO_1, __PE0, __PE1, __PE2, __PE3, __PE4, __PE5, __PE6, __PG6, __PG7, __PG12, __PG13, __PG14, __PA6, __PA7, __PB11, __PB12, __PB13, __PB14, __PB4, __PB2, __PA0, __PA1, __PA4, __PA5, __PB0, __PB1, __PB3, __PB5, __PB8, __PB9);
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            software_pwm_init(&spwm);    
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            spwm.pwm[0] = 20;
            spwm.pwm[1] = 50;
            spwm.pwm[2] = 100;
            spwm.pwm[3] = 180;
            spwm.pwm[4] = 210;
            spwm.pwm[5] = 250;
            
            spwm.pwm[6] = 0;
            spwm.pwm[7] = 255;  // Not reachable: 99,61% of duty cycle
            break;
    } 
}

void _EXAMPLE_SWITCH()
{
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            switch (sw1.indice)
            {
                case 0:
                    mUpdateLedStatusD2(OFF);
                    break;
                case 1:
                    mUpdateLedStatusD2(fu_turn_indicator(ON, TICK_100MS, TICK_100MS));
                    break;
                case 2:
                    mUpdateLedStatusD2(fu_turn_indicator(ON, TICK_200MS, TICK_200MS));
                    break;
                case 3:
                    mUpdateLedStatusD2(fu_turn_indicator(ON, TICK_500MS, TICK_50MS));
                    break;
                case 4:
                    sw1.indice = 0;
                    break;
            }
            
            if (sw1.type_of_push == LONG_PUSH)
            {
                mUpdateLedStatusD2(BLINK);
                mUpdateLedStatusD3(BLINK_INV);
            }
            
            if (sw1.is_updated)
            {
                sw1.is_updated = false;
                // ...
            }
            
            fu_switch(&sw1);
            break;
    } 
}

void _EXAMPLE_ENCODER()
{
    ENCODER_DEF(encoder, __PE0, __PE1, ACTIVE_HIGH);
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (encoder.indice > 0)
            {
                mUpdateLedStatusD2(fu_turn_indicator(ON, TICK_1MS*encoder.indice, TICK_1MS*encoder.indice));
                mUpdateLedStatusD3(OFF);
            }
            else if (encoder.indice < 0)
            {
                mUpdateLedStatusD2(OFF);
                mUpdateLedStatusD3(fu_turn_indicator(ON, -(TICK_1MS*encoder.indice), -(TICK_1MS*encoder.indice)));
            }
            else
            {
                mUpdateLedStatusD2(BLINK);
                mUpdateLedStatusD3(BLINK_INV);
            }
            
            fu_encoder(&encoder);
            break;
    }   
}

void _EXAMPLE_AVERAGE_AND_NTC()
{
    NTC_DEF(ntc_1, AN1, 25, 10000, 3380, 10000);
    NTC_DEF(ntc_2, AN2, 25, 10000, 3380, 10000);
    AVERAGE_DEF(avg_1, AN3, 30, TICK_1MS);
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            adc10_init(AN1|AN2|AN3|AN15, ADC10_VREFP_VREFN, NULL);
            {
                uint16_t i;
                NTC_PARAMS ntc_params = {25, 33000, 3380};
                for (i = 0 ; i <= 255 ; i++)
                {
                    float temperature;
                    NTC_STATUS ret = fu_calc_ntc(ntc_params, 4700, i, 8, & temperature);
                    if (ret == NTC_SUCCESS)
                    {
                        if (temperature > 0)
                        {
                            LOG("Temperature [%d] = %1f °C", i, p_float(temperature));
                        }
                        else
                        {
                            // Log function do not print negative values.
                            LOG("Temperature [%d] = < 0,0 °C", i, p_float(temperature));
                        }
                    }
                    else if (ret == NTC_FAIL_SHORT_CIRCUIT_GND)
                    {
                        LOG("Fail short circuit to GND.");
                    }
                    else if (ret == NTC_FAIL_SHORT_CIRCUIT_VREF)
                    {
                        LOG("Fail short circuit to Vref.");
                    }
                }
            }
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            fu_adc_ntc(&ntc_1);
            fu_adc_ntc(&ntc_2);
            fu_adc_average(&avg_1);
            
            if (ntc_1.temperature > 32.0)
            {
                // ...
            }
            break;
    }   
}

void _EXAMPLE_SLIDER()
{
    SLIDER_DEF(slider_w, 198, SLIDER_START_TO_END, SLIDER_CENTER_TO_ENDS, TICK_800MS, TICK_400MS);
    SLIDER_DEF(slider_r, 198, SLIDER_CENTER_TO_ENDS, SLIDER_START_TO_END, TICK_400MS, TICK_800MS);
    PINK_LADY_DEF(smartled, SPI3, SK6812RGBW_MODEL, 198);
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            slider_w.enable = fu_turn_indicator(ON, TICK_1S, TICK_1S);
            slider_r.enable = !fu_turn_indicator(ON, TICK_1S, TICK_1S);

            {
                uint8_t i;
                for (i = 0 ; i < slider_w.p_output.size ; i++)
                {
                    pink_lady_set_led_rgbw(smartled, i, slider_r.p_output.p[i] ? 255 : 0, 0, 0, slider_w.p_output.p[i] ? 255 : 0);
                }
            }

            fu_slider(&slider_w);
            fu_slider(&slider_r);
            pink_lady_deamon(&smartled);
            break;
    }   
}

void _EXAMPLE_25LC512()
{
    _25LC512_DEF(e_25lc512, SPI2, __PD3, TICK_20MS, 150, 150);
    BUS_MANAGEMENT_DEF(bm_spi, &e_25lc512.spi_params.bus_management_params);
    static state_machine_t sm_example = {0};
    static uint64_t tickAntiFloodSw1 = 0;
    static uint64_t tickAntiFloodSw2 = 0;
    static uint64_t tickRead = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:
            e_25lc512_check_for_erasing_memory(&e_25lc512, &bm_spi);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            if (!mGetIO(SWITCH1) && (mTickCompare(tickAntiFloodSw1) > TICK_200MS))
            {
                tickAntiFloodSw1 = mGetTick();
                if (!e_25lc512_is_write_in_progress(e_25lc512))
                {
                    // !! BE SURE TO HAVE ENOUGH SPACE INTO THE BUFFER !!
                    uint8_t i = 0;
                    e_25lc512.registers.dW.size = 5;
                    for(i = 0 ; i < (e_25lc512.registers.dW.size) ; i++)
                    {
                        e_25lc512.registers.dW.p[i] = 0x44+i;
                    }
                    e_25lc512_write_bytes(e_25lc512, 127);
                }
            }

            if (!mGetIO(SWITCH2) && (mTickCompare(tickAntiFloodSw2) > TICK_200MS))
            {
                tickAntiFloodSw2 = mGetTick();
                e_25lc512_chip_erase(e_25lc512);
            }

            if (mTickCompare(tickRead) >= TICK_100MS)
            {
                tickRead = mGetTick();
                e_25lc512_read_bytes(e_25lc512, 127, 5);
            }

            if (!e_25lc512_is_read_in_progress(e_25lc512))
            {
                if (e_25lc512.registers.dR.p[0] == 0x44)
                {
                    mUpdateLedStatusD2(ON);
                }
                else
                {
                    mUpdateLedStatusD2(OFF);
                }
            }
            
            fu_bus_management_task(&bm_spi);
            e_25lc512_deamon(&e_25lc512);
            break;
            
    }
}

void _EXAMPLE_MCP23S17()
{
    MCP23S17_DEF(e_mcp23s17, SPI2, __PD3, TICK_20MS);
    BUS_MANAGEMENT_DEF(bm, &e_mcp23s17.spi_params.bus_management_params);
    static bool isInitDone = false;
    static uint64_t tickAntiFloodSw1 = 0;
    
    if (!isInitDone)
    {
        e_mcp23s17.write_registers.IODIRA = 0x00; 
        isInitDone = true;
    }
    else
    {
        if (!mGetIO(SWITCH1) && (mTickCompare(tickAntiFloodSw1) > TICK_200MS))
        {
            tickAntiFloodSw1 = mGetTick();
            
            e_mcp23s17.write_registers.OLATA = !e_mcp23s17.read_registers.GPIOA;
        }
        else
        {
            e_mcp23s17.write_registers.OLATA = 0x00;
        }
        
        if (GET_BIT(e_mcp23s17.read_registers.GPIOA, 0))
        {
            mUpdateLedStatusD2(ON);
        }
        else
        {
            mUpdateLedStatusD2(OFF);
        }
        
        fu_bus_management_task(&bm);
        eMCP23S17Deamon(&e_mcp23s17);
    }
}

#define VALEO   "1MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"2MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"3MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"4MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"5MMMMMMMMMMMMMMMMMMMMMMMMMMMhhhhhhhhhhhhhhhhhhhhMMMMMMMMMMMMMMMMMMMhhhhhhhhhhhhhhhhhhmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNhhhhhhhhhhhhhhhhhMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"6MMMMMMMMMMMMMMMMMMMMMMMMMMM+///////////////////NMMMMMMMMMMMMMMMMMd/////////////////+NMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMh////////////////+MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"7MMMMMMMMMMMMMMMMMMMMMMMMMMMo///////////////////mMMMMMMMMMMMMMMMMMo/////////////////dMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMs////////////////sMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"8MMMMMMMMMMMMMMMMMMMMMMMMMMMo///////////////////mMMMMMMMMMMMMMMMMh/////////////////yMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM+////////////////hMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"9MMMMMMMMMMMMMMMMMMMMMMMMMMMs///////////////////mMMMMMMMMMMMMMMMN+////////////////oMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN/////////////////mMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"10MMMMMMMMMMMMMMMMMMMMMMMMMMy///////////////////mMMMMMMMMMMMMMMMs////////////////+NMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMh/////////////////MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"11MMMMMMMMMMMMMMMMMMMMMMMMMMd///////////////////dMMMMMMMMMMMMMMm/////////////////dMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMs////////////////oMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"12MMMMMMMMMMMMMMMMMMMMMMMMMMm///////////////////hMMMMMMMMMMMMMN+////////////////yMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM+////////////////yMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"13MMMMMMMMMMMMMMMMMMMMMMMMMMN///////////////////hMMMMMMMMMMMMMy////////////////sMMMMMMMMMMMMNmmdhhhyyyyyyyyyyyyyyyyyhhhhhNMMMN/////////////////mMMMMMMMMMMMMMMNmdhyyssooooooossyhdmMMMMMMMMMMMMMMMMMMMMMMMMMMNmdhyyssooooooossyyhdmNMMMMMMMMMMMMMMMMMMM\n\r"\
				"14MMMMMMMMMMMMMMMMMMMMMMMMMMM+//////////////////yMMMMMMMMMMMMm////////////////oNMMMMMMMmdyo+++//////////////////////////+MMMMh/////////////////MMMMMMMMMMMmhs++///////////////////++shmMMMMMMMMMMMMMMMMMMNdyo++////////////////////++ydNMMMMMMMMMMMMMMM\n\r"\
				"15MMMMMMMMMMMMMMMMMMMMMMMMMMMs//////////////////sMMMMMMMMMMMN+///////////////+mMMMMMdy++////////////////////////////////sMMMMs////////////////oMMMMMMMMNho+///////////////////////////+odMMMMMMMMMMMMMNds+/////////////////////////////+smMMMMMMMMMMMMM\n\r"\
				"16MMMMMMMMMMMMMMMMMMMMMMMMMMMh//////////////////oMMMMMMMMMMMs////////////////dMMMNho////////////////////////////////////hMMMM+////////////////yMMMMMMNy+/////////////////////////////////oNMMMMMMMMMMh+///////////////////////////////////sNMMMMMMMMMMM\n\r"\
				"17MMMMMMMMMMMMMMMMMMMMMMMMMMMm//////////////////+MMMMMMMMMMh////////////////hMMNh+//////////////////////////////////////mMMMN/////////////////dMMMMMmo/////////////////++/////////////////+mMMMMMMMNs//////////////////////////////////////+NMMMMMMMMMM\n\r"\
				"18MMMMMMMMMMMMMMMMMMMMMMMMMMMM+//////////////////MMMMMMMMMm+///////////////yMMmo/////////////////+ossso////////////////+MMMMd/////////////////MMMMMd+///////////////ohmNNNdo///////////////oMMMMMMm+////////////////ohmNmds/////////////////oMMMMMMMMMM\n\r"\
				"19MMMMMMMMMMMMMMMMMMMMMMMMMMMMs//////////////////NMMMMMMMNo///////////////sMMd+////////////////sdNMMMMs////////////////sMMMMy////////////////oMMMMd////////////////yMMMMMMMNo///////////////dMMMMm+////////////////sNMMMMMMs/////////////////hMMMMMMMMM\n\r"\
				"20MMMMMMMMMMMMMMMMMMMMMMMMMMMMd//////////////////mMMMMMMMs///////////////sNMd+////////////////hMMMMMMM+////////////////hMMMM+////////////////yMMMN+///////////////sMMMMMMMMMy///////////////sMMMMo////////////////+NMMMMMMMm/////////////////sMMMMMMMMM\n\r"\
				"21MMMMMMMMMMMMMMMMMMMMMMMMMMMMN//////////////////hMMMMMMy///////////////oNMm+////////////////dMMMMMMMm/////////////////mMMMN/////////////////dMMMs////////////////mMMMMMMMMMs///////////////sMMMy/////////////////dMMMMMMMMM/////////////////+MMMMMMMMM\n\r"\
				"22MMMMMMMMMMMMMMMMMMMMMMMMMMMMMo/////////////////sMMMMMd///////////////oNMMo////////////////sMMMMMMMMh////////////////+MMMMd/////////////////NMMN/////////////////hhhhhhhhhh////////////////sMMN+////////////////+MMMMMMMMMN/////////////////+MMMMMMMMM\n\r"\
				"23MMMMMMMMMMMMMMMMMMMMMMMMMMMMMh/////////////////oMMMMm+//////////////+NMMd/////////////////mMMMMMMMMs////////////////oMMMMy////////////////oMMMh///////////////////////////////////////////yMMh/////////////////sMMMMMMMMMm/////////////////oMMMMMMMMM\n\r"\
				"24MMMMMMMMMMMMMMMMMMMMMMMMMMMMMN//////////////////MMMN+//////////////+mMMMo////////////////+MMMMMMMMM+////////////////hMMMMo////////////////yMMMo///////////////////////////////////////////dMMo/////////////////dMMMMMMMMMh/////////////////sMMMMMMMMM\n\r"\
				"25MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo/////////////////mMNo//////////////+mMMMN/////////////////yMMMMMMMMN/////////////////mMMMN/////////////////dMMM///////////////////////////////////////////+NMM+/////////////////mMMMMMMMMMy/////////////////yMMMMMMMMM\n\r"\
				"26MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMd/////////////////hNo//////////////+mMMMMh/////////////////dMMMMMMMMh////////////////+MMMMd/////////////////NMMm///////////////////////////////////////////yMMN/////////////////+MMMMMMMMMMo/////////////////dMMMMMMMMM\n\r"\
				"27MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM+////////////////os//////////////+mMMMMMs/////////////////NMMMMMMMMs////////////////oMMMMy////////////////oMMMm/////////////////osssssssssssssssssssssssssmMMd/////////////////oMMMMMMMMMN//////////////////NMMMMMMMMM\n\r"\
				"28MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMy///////////////////////////////+mMMMMMMo/////////////////NMMMMMMMM+////////////////yMMMMo////////////////yMMMm/////////////////dMMMMMMMMMMMMMMMMMMMMMMMMMMMMd/////////////////yMMMMMMMMMd/////////////////sMMMMMMMMMM\n\r"\
				"29MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMm//////////////////////////////+dMMMMMMM+/////////////////NMMMMMMMN/////////////////mMMMN/////////////////dMMMm/////////////////oMMMMMMMMMMMMMMMMMMMMMMMMMMMMd/////////////////yMMMMMMMMMo/////////////////mMMMMMMMMMM\n\r"\
				"30MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo////////////////////////////+dMMMMMMMM+/////////////////hMMMMMMMh/////////////////NMMMd/////////////////NMMMN//////////////////smNMMMMMMMMMMMMMMMMMNNMMMMMMd/////////////////sMMMMMMMMm/////////////////sMMMMMMMMMMM\n\r"\
				"31MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMd///////////////////////////+mMMMMMMMMMs//////////////////ymmmmhs+////////////////oMMMMy////////////////+MMMMMs///////////////////oshddmmmmmmddhhysoooMMMMMMN/////////////////+NMMMMMMNo////////////////oNMMMMMMMMMMM\n\r"\
				"32MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM+/////////////////////////+mMMMMMMMMMMh////////////////////++////////////////////yMMMMo////////////////yMMMMMm+/////////////////////////////////////yMMMMMMMs/////////////////omNMMNdo////////////////+mMMMMMMMMMMMM\n\r"\
				"33MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMh////////////////////////+mMMMMMMMMMMMN+/////////////////////////////////////////dMMMM/////////////////dMMMMMMd+////////////////////////////////////mMMMMMMMNo//////////////////ooo+/////////////////oNMMMMMMMMMMMMM\n\r"\
				"34MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN+//////////////////////oNMMMMMMMMMMMMMh/////////////////////////////////////////NMMMm/////////////////NMMMMMMMms+/////////////////////////////////oMMMMMMMMMNs////////////////////////////////////+hMMMMMMMMMMMMMMM\n\r"\
				"35MMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMy/////////////////////oNMMMMMMMMMMMMMMMh+/////////////////////++///////////////oMMMMy////////////////+MMMMMMMMMMmy+///////////////////////////////hMMMMMMMMMMMdo////////////////////////////////ohNMMMMMMMMMMMMMMMM\n\r"\
				"36MMMMMMMmhdmNMMMMMMMMMMMMMMMMMMMMMMN+///////////////////sNMMMMMMMMMMMMMMMMMNyo+/////////////++shmN+///////////////yMMMMo////////////////yMMMMMMMMMMMMNmhso++/////////////////////+++oNMMMMMMMMMMMMMmyo+////////////////////////+oydNMMMMMMMMMMMMMMMMMM\n\r"\
				"37MMMMMMMMNdyyhdmNMMMMMMMMMMMMMMMMMMMdhhhhhhhhhhhhhhhhhhhNMMMMMMMMMMMMMMMMMMMMMmdhsssooossyhdmNMMMMhhhhhhhhhhhhhhhhNMMMMhhhhhhhhhhhhhhhhhmMMMMMMMMMMMMMMMMMNmmdhhyysssssssyyyhhddmmNNMMMMMMMMMMMMMMMMMMmdhyoo++++++/+++++++osyhdNMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"38MMMMMMMMMMNhyyyyhdNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNNmmmmmmmmNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"39MMMMMMMMMMMMmyyyyyyhdNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"40MMMMMMMMMMMMMMdyyyyyyyhdNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"41MMMMMMMMMMMMMMMNhyyyyyyyyhmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNNNNNNNmmmmmmddddddhhhhhhhhhhhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhhhhhhhhhhhhhdddddddmmmmmmmNNNNNNNNNMMMMMMMMMMMMMMMMM\n\r"\
				"42MMMMMMMMMMMMMMMMMdyyyyyyyyyhmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNNNNNmmmddddhhhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhdddddddmMMMMMMMMMMMMMMM\n\r"\
				"43MMMMMMMMMMMMMMMMMMNyyyyyyyyyyhmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNNNNmmdddhhhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhhhhddddddmmmmmmmNNNNNNNNNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"44MMMMMMMMMMMMMMMMMMMNhyyyyyyyyyyhmMMMMMMMMMMMMMMMMMNNNNmmddhhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhhddddmmmmNNNNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"45MMMMMMMMMMMMMMMMMMMMMdyyyyyyyyyyyhNMMMMMNNmmddhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhhddmmmmNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"46MMMMMMMMMMMMMMMMMMMMMMmyyyyyyyyyyyyhhhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhddmmmNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"47MMMMMMMMMMMMMMMMMMMMMMMNyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhhddmmmNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"48MMMMMMMMMMMMMMMMMMMMMMMMNyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhddmmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"49MMMMMMMMMMMMMMMMMMMMMMMMMNyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhddmmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"50MMMMMMMMMMMMMMMMMMMMMMMMMMNyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyhddmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"51MMMMMMMMMMMMMMMMMMMMMMMMMMMNyyyyyyyyyyyyyyyyyyyyyyyyyhdmmNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"52MMMMMMMMMMMMMMMMMMMMMMMMMMMMmyyyyyyyyyyyyyyyyyhdmmNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"53MMMMMMMMMMMMMMMMMMMMMMMMMMMMMdyyyyyyyyyyhdmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"54MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMyyyyhdmNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"55MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNmNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"56MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"57MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"58MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\r"\
				"59MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
void _EXAMPLE_LOG(ACQUISITIONS_PARAMS var)
{
    static state_machine_t sm_example = {0};
    static state_machine_t sm_log = {0};
    static char str1[] = "String 1";
    static char *str2 = "String 2";
    static uint32_t val = 0x546389;
    static float val_f = 346.946;
        
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            // Do not forget to enable LOG driver.
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            switch (sm_log.index)
            {
                case 0:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG_BLANCK("!! Start to LOG !!");
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 1:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG("Binary log - @: %x - 32 digits: %32b - necessary digits: %b", (uint32_t) &val, val, val);
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 2:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG("Octave log - @: %x - 10 digits: %10o - necessary digits: %o", (uint32_t) &val, val, val);
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 3:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG("Decade log - @: %x - 5 digits: %5d - necessary digits: %d", (uint32_t) &val, val, val);
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 4:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG("Hex log - @: %x - 10 digits: %10x - necessary digits: %x", (uint32_t) &val, val, val);
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 5:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG("Float log (using p_float(var) is mandatory / result is approximative >3 digits after coma) - 5 digits: %5f - 1 digit: %1f - default 3 digits: %f", p_float(val_f), p_float(val_f), p_float(val_f));
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 6:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG_SHORT("Char log - %c %c %c", 'a', 'B', 'c');
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 7:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG_SHORT("Strings log short (case %d) - String + %s + %s + %s", sm_log.index, p_string("String 0"), p_string(str1), p_string(str2));
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                case 8:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        LOG_BLANCK("%s", p_string(VALEO));
                        sm_log.index++;
                        sm_log.tick = mGetTick();
                    }
                    break;
                    
                default:
                    if (mTickCompare(sm_log.tick) >= TICK_1S)
                    {
                        LOG("@: %x - Voltage = %fV - Current = %fA - Power Consumption = %fW - Temperature = %1f°C", (uint32_t) &var, p_float(var.voltage.average), p_float(var.current.average), p_float(var.power_consumption), p_float(var.ntc.temperature));
                        sm_log.tick = mGetTick();
                    }
                    break;
            }
            break;
    } 
}
//
//void _EXAMPLE_WS2812B_SINGLE_SEGMENT()
//{
//    WS2812B_DEF(ws2812b_single, SPI1, __PA0, 18, 18);
//    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
//    static WS2812B_ANIMATION animation;
//    static state_machine_t sm_example = {0};
//    static bool _execute = false;
//    static uint64_t tick = 0;
//    
//    if (sw1.type_of_push == LONG_PUSH)
//    {
//        if (mTickCompare(tick) >= TICK_4S)
//        {
//            tick = mGetTick();
//            sw1.indice++;
//            _execute = true;
//            animation.number_of_repetition = 0;
//        }
//    }
//    
//    switch (sm_example.index)
//    {
//        case _SETUP:          
//      
//            sm_example.index = _MAIN;
//            break;
//            
//        case _MAIN:
//            if (sw1.is_updated)
//            {
//                sw1.is_updated = false;
//                _execute = true;
//            }
//            
//            switch (sw1.indice)
//            {
//                case 0:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_ALL, COLOR_OFF);
//                    }
//                    break;
//                    
//                case 1:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_1_3, COLOR_BLUE);
//                    }
//                    break;
//                    
//                case 2:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color_delay(ws2812b_single, 0, LED_ALL, COLOR_GREEN, TICK_1S);
//                    }
//                    break;
//                    
//                case 3:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color_from_to(ws2812b_single, 0, LED_ALL, COLOR_WHITE, 6, 11);
//                        ws2812b_put_color_from_to(ws2812b_single, 0, LED_ALL, COLOR_RED, 12, 17);
//                    }
//                    break;
//                    
//                case 4:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_ALL, COLOR_OFF);
//                        ws2812b_put_gradient(ws2812b_single, 0, LED_1_2, COLOR_RED, COLOR_GREEN);
//                    }
//                    break;
//                    
//                case 5:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_gradient_delay(ws2812b_single, 0, LED_ALL, COLOR_CYAN, COLOR_RED, TICK_1S);
//                    }
//                    break;
//                    
//                case 6:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color_delay(ws2812b_single, 0, LED_ALL, COLOR_WHITE, TICK_1S);
//                    }
//                    break;
//                    
//                case 7:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_gradient_from_to(ws2812b_single, 0, LED_ALL, COLOR_RED, COLOR_GREEN, 5, 12);
//                    }
//                    break;
//                    
//                case 8:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_gradient_from_to_delay(ws2812b_single, 0, LED_ALL, COLOR_GREEN, COLOR_BLUE, 5, 12, TICK_1S);
//                    }
//                    break;
//                    
//                case 9:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color_effect(ws2812b_single, 0, LED_ALL, COLOR_BLUE, WS2812B_EFFECT_TRIANGLE | WS2812B_SUPERPOSE_EFFECT, WS2812B_REPETITION_INFINITE, TICK_1S);
//                    }
//                    break;
//                    
//                case 10:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_ALL, COLOR_PURPLE);
//                        ws2812b_put_color_effect_from_to(ws2812b_single, 0, LED_ALL, COLOR_GREEN, 5, 12, WS2812B_EFFECT_GAUSSIAN | WS2812B_RESTORE_COLOR, 20, TICK_1S);
//                    }
//                    break;
//                    
//                case 11:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_ALL, COLOR_GREEN);
//                        ws2812b_put_color_effect_from_to(ws2812b_single, 0, LED_ALL, COLOR_RED, 5, 12, WS2812B_EFFECT_SAWTOOTH, 20, TICK_1S);
//                        ws2812b_put_color_effect_from_to(ws2812b_single, 0, LED_ALL, COLOR_RED, 13, 17, WS2812B_EFFECT_SAWTOOTH_INV, 20, TICK_1S);
//                    }
//                    break;
//                    
//                case 12:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_gradient_effect(ws2812b_single, 0, LED_ALL, COLOR_BLUE, COLOR_GREEN, WS2812B_EFFECT_TRIANGLE, WS2812B_REPETITION_INFINITE, TICK_200MS);
//                    }
//                    break;
//                    
//                case 13:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color(ws2812b_single, 0, LED_ALL, COLOR_WHITE);
//                        ws2812b_put_gradient_effect_from_to(ws2812b_single, 0, LED_ALL, COLOR_RED, COLOR_CYAN, 0, 8, WS2812B_EFFECT_GAUSSIAN | WS2812B_RESTORE_COLOR, WS2812B_REPETITION_INFINITE, TICK_1S);
//                    }
//                    break;
//                    
//                case 14:
//                    if (_execute)
//                    {
//                        _execute = false;
//                        ws2812b_put_color_delay(ws2812b_single, 0, LED_ALL, COLOR_OFF, TICK_1S);
//                    }
//                    if (ws2812b_is_segment_updated(ws2812b_single, 0))
//                    {
//                        eWS2812BSetParamsChenillard(0, FIRST_LED, LAST_LED, 4, COLOR_RED, COLOR_RED, 2, 0, 0, WS2812B_REPETITION_INFINITE, -1, TICK_2S, &animation, ws2812b_single);
//                    }
//                    break;
//                    
//                default:
//                    sw1.indice = 0;
//                    break;
//            }
//            
//            fu_switch(&sw1);
//            eWS2812BAnimation(&animation, &ws2812b_single);
//            eWS2812BFlush(TICK_100US, &ws2812b_single);
//            break;
//    }   
//}

void _EXAMPLE_UART()
{
    static state_machine_t sm_example = {0};
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
    static char tab_string[] = "Helloooo !";
    static uint8_t ind_string = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
      
            uart_init(UART1, NULL, IRQ_NONE, 115200, UART_STD_PARAMS);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (sw1.is_updated)
            {
                sw1.is_updated = false;
            }
            switch (sw1.indice)
            {
                case 0:
                    // Do nothing
                    break;
                    
                case 1:
                    if (mTickCompare(sm_example.tick) >= TICK_100MS)
                    {
                        if (!uart_send_break(UART1))
                        {
                            sm_example.tick = mGetTick();
                        }
                    }
                    ind_string = 0;
                    break;
                    
                case 2:
                    if (mTickCompare(sm_example.tick) >= TICK_100MS)
                    {
                        if (!uart_send_data(UART1, tab_string[ind_string]))
                        {
                            if (tab_string[++ind_string] == '\0')
                            {
                                ind_string = 0;
                                sm_example.tick = mGetTick();
                            }
                        }
                    }
                    break;
                    
                default:
                    sw1.indice = 0;
                    break;
            }
            fu_switch(&sw1);
            break;
    }   
}

void _EXAMPLE_PCA9685()
{
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
    PCA9685_DEF(pca9685, I2C2, 0x40, TICK_2MS);
    BUS_MANAGEMENT_DEF(bm_i2c2, &pca9685.i2c_params.bus_management_params);
    static state_machine_t sm_example = {0};
    static uint16_t val = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
      
            e_pca9685_reset(pca9685);
            e_pca9685_set_frequency(pca9685, PCA9685_FREQ_500HZ);
            e_pca9685_config_mode1(pca9685, (PCA9685_USE_INTERNAL_CLOCK | PCA9685_ENABLE_AUTO_INCREMENT_REG));
            e_pca9685_config_mode2(pca9685, (PCA9685_OUTPUT_LOGIC_STATE_NORMAL | PCA9685_UPDATE_OUTPUT_ON_STOP_CMD | PCA9685_OUTPUTS_ARE_DRIVE));
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            switch (sw1.indice)
            {
                case 0:
                    break;
                    
                case 1:
                    e_pca9685_set_frequency(pca9685, PCA9685_FREQ_100HZ);
                    sw1.indice++;
                    break;
                    
                case 2:
                    break;
                    
                case 3:
                    e_pca9685_reset(pca9685);
                    sw1.indice++;
                    break;
                    
                default:
                    sw1.indice = 0;
                    break;
            }
            
            if (mTickCompare(sm_example.tick) >= TICK_10MS)
            {
                sm_example.tick = mGetTick();
                if (++val >= 4096)
                {
                    val = 0;
                }
            }
            e_pca9685_set_pwm(pca9685, 0, 1024);
            e_pca9685_set_pwm(pca9685, 1, val);
            e_pca9685_set_pwm(pca9685, 2, 3096);
            e_pca9685_set_pwm_with_phase(pca9685, 3, 768, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 4, 1024, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 5, 1280, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 6, 1536, 2048);
            pca9685.registers.output[7].phase = 1792;
            pca9685.registers.output[7].duty_cycle = 244;
            e_pca9685_set_pwm_with_phase(pca9685, 8, 2048, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 9, 2304, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 10, 2560, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 11, 2816, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 12, 3072, 2048);
            e_pca9685_set_pwm_with_phase(pca9685, 13, 3328, 2048);
            e_pca9685_output_off(pca9685, 14);
            e_pca9685_output_on(pca9685, 15);

            fu_bus_management_task(&bm_i2c2);
            e_pca9685_deamon(&pca9685);
            fu_switch(&sw1);
            break;
    } 
}

void _EXAMPLE_AT42QT2120()
{
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
    AT42QT2120_DEF(at42qt2120, I2C2, TICK_10MS);
    BUS_MANAGEMENT_DEF(bm_i2c2, &at42qt2120.i2c_params.bus_management_params);
    static state_machine_t sm_example = {0};
    
    static uint32_t val = 0;
    static uint32_t old_val = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
      
            e_at42qt2120_reset(at42qt2120);
            e_at42qt2120_calibrate(at42qt2120);
            e_at42qt2120_enable_slider(at42qt2120);
            
            e_at42qt2120_set_key_control(at42qt2120, AT42QT2120_KEY_3, AT42QT2120_AKS_GROUP_1);
            e_at42qt2120_set_key_control(at42qt2120, AT42QT2120_KEY_4, AT42QT2120_AKS_GROUP_1);
            e_at42qt2120_set_key_control(at42qt2120, AT42QT2120_KEY_5, AT42QT2120_AKS_GROUP_1);
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN: 
            
            e_at42qt2120_get_status_keys_and_slider(at42qt2120);
            
            switch (sw1.indice)
            {
                case 0:
                    break;
                    
                case 1:
                    e_at42qt2120_disable_slider(at42qt2120);
                    sw1.indice++;
                    break;
                    
                case 2:
                    break;
                    
                case 3:
                    e_at42qt2120_enable_slider(at42qt2120);
                    sw1.indice++;
                    break;
                    
                default:
                    sw1.indice = 0;
                    break;
            }
            
            val = (at42qt2120.registers.keys0to7_status << 8) | (at42qt2120.registers.slider_position << 0);
            if (val != old_val)
            {
                old_val = val;
                LOG_BLANCK("slider: %d \nKey3: %d \nKey4: %d \nKey5: %d", val&0xff, GET_BIT(val, 11), GET_BIT(val, 12), GET_BIT(val, 13));
            }
            mUpdateLedStatusD2(GET_BIT(at42qt2120.registers.keys0to7_status, 3));
            
            fu_bus_management_task(&bm_i2c2);
            e_at42qt2120_deamon(&at42qt2120);
            fu_switch(&sw1);
            break;
    } 
}

void _EXAMPLE_VEML7700()
{
    VEML7700_DEF(veml7700, I2C2, TICK_10MS);
    BUS_MANAGEMENT_DEF(bm_i2c2, &veml7700.i2c_params.bus_management_params);
    static state_machine_t sm_example = {0};
    static uint64_t tick = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          

            e_veml7700_set_config(veml7700, VEML7700_ALS_GAIN_1 | VEML7700_ALS_INTEGRATION_TIME_25MS | VEML7700_ALS_PERSISTANT_PROTECT_NUMBER_1 | VEML7700_ALS_INTERRUPT_DISABLE | VEML7700_ALS_POWER_ON);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN: 
            
            if (e_veml7700_is_flag_empty(veml7700))
            {
                e_veml7700_read_als(veml7700);
                e_veml7700_read_white(veml7700);
            }
            
            if (mTickCompare(tick) >= TICK_200MS)
            {
                tick = mGetTick();
                LOG_BLANCK("Ambient light sensor: %d \nWhite: %d\n", veml7700.registers.ambient_light_sensor, veml7700.registers.white);
            }
            
            fu_bus_management_task(&bm_i2c2);
            e_veml7700_deamon(&veml7700);
            break;
    } 
}

void _EXAMPLE_BLE(ble_params_t * p_ble)
{
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:          
            // Declare ble variable "ble_params" and enable it with cfg_ble(ENABLE, &ble_params)
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            // CENTRAL write a buffer to the PERIPHERAL
            mUpdateLedStatusD3(p_ble->service.buffer.in_data[0]);
            if (p_ble->service.buffer.in_is_updated)
            {
                p_ble->service.buffer.in_is_updated = false;
                mToggleLedStatusD2();
            }
            // Notify the CENTRAL by using a "buffer object"
            if (mTickCompare(sm_example.tick) >= TICK_1S)
            {
                sm_example.tick = mGetTick();
                p_ble->service.buffer.out_data[0]++;
                p_ble->service.buffer.out_length = 1;
                p_ble->status.flags.send_buffer = true;
            }
            break;
    } 
}

void _EXAMPLE_LIN()
{
    LIN_DEF(lin1, UART2, LIN1_ENABLE, LIN_VERSION_2_X);
    static LIN_FRAME_PARAMS lin_frame1 = {0};
    static LIN_FRAME_PARAMS lin_frame2 = {0};
    static state_machine_t sm_example = {0};
    static state_machine_t sm = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:          
            
            lin_frame1.read_write_type = LIN_WRITE_REQUEST;
            lin_frame1.id = 0x30;
            
            lin_frame2.read_write_type = LIN_READ_REQUEST;
            lin_frame2.id = 0x22;
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (lin1.frame == NULL) 
            {
                if (mTickCompare(sm.tick) >= TICK_10MS)
                {
                    switch (sm.index)
                    {
                        case 0:

                            lin_frame1.data[0] = lin_frame2.errors.timing >> 8;
                            lin_frame1.data[1] = lin_frame2.errors.timing >> 0;
                            lin_frame1.data[2] = lin_frame2.errors.readback >> 8;
                            lin_frame1.data[3] = lin_frame2.errors.readback >> 0;
                            lin_frame1.data[4] = sm.tick >> 24;
                            lin_frame1.data[5] = sm.tick >> 16;
                            lin_frame1.data[6] = sm.tick >> 8;
                            lin_frame1.data[7] = sm.tick >> 0;

                            lin1.frame = &lin_frame1;
                            sm.index++;
                            sm.tick = mGetTick();
                            break;
                        case 1:
                            lin1.frame = &lin_frame2;
                            sm.index = 0;
                            sm.tick = mGetTick();
                            break;
                        default:
                            sm.index = 0;
                            sm.tick = mGetTick();
                            break;
                    }
                }
            }
            else
            {
                sm.tick = mGetTick();
            }            
            
            lin_master_deamon(&lin1);
            
            break;
    } 
}

void _EXAMPLE_PINK_LADY()
{
    PINK_LADY_DEF(smartled, SPI2, SK6812RGBW_MODEL, 50);
    PINK_LADY_SEGMENT_DEF(smartled_seg_1, smartled);
    static state_machine_t sm_colors = {0};
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:          
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (mTickCompare(sm_colors.tick) >= TICK_3S)
            {
                sm_colors.tick = mGetTick();
                sm_colors.index++;
            }

            switch (sm_colors.index)
            {
                case 0:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 19, RGBW_COLOR_BLUE, RGBW_COLOR_WHITE, LED_RESO_ALL, 0))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 1:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 20, 49, RGBW_COLOR_WHITE, RGBW_COLOR_RED, LED_RESO_ALL, 0))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 2:
                    break;
                    
                case 3:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_RED, RGBW_COLOR_GREEN, LED_RESO_ALL, TICK_200MS))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 4:
                    break;
                    
                case 5:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_GREEN, RGBW_COLOR_BLUE, LED_RESO_1_3, TICK_500MS))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 6:
                    break;
                    
                case 7:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_WHITE, RGBW_COLOR_WHITE_MIX, LED_RESO_1_5, TICK_500MS))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 8:
                    break;
                    
                case 9:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_CYAN, RGBW_COLOR_OFF, LED_RESO_ALL, TICK_500MS))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 10:
                    break;
                    
                case 11:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_RED, RGBW_COLOR_RED, LED_RESO_1_3, 0))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 12:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 10, 39, RGBW_COLOR_BLUE, RGBW_COLOR_OFF, LED_RESO_1_4, 0))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 13:
                    break;
                    
                case 14:
                    if (!pink_lady_set_segment_params(&smartled_seg_1, 0, 49, RGBW_COLOR_OFF, RGBW_COLOR_OFF, LED_RESO_ALL, 0))
                    {
                        sm_colors.index++;
                    }
                    break;
                case 15:
                    pink_lady_set_led_rgbw(smartled, 0, 255, 0, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 9, 0, 255, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 19, 0, 0, 255, 0);
                    pink_lady_set_led_rgbw(smartled, 29, 0, 0, 0, 255);
                    pink_lady_set_led_rgbw(smartled, 39, 255, 255, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 49, 0, 255, 255, 0);
                    break;
                    
                default:
                    sm_colors.index = 0;
                    break;
            }

            pink_lady_deamon(&smartled);
            break;
    } 
}

void _EXAMPLE_TPS92662()
{
    TPS92662_DEF(tps_pix32_module1, UART5, __PE0, 500000, 0x00, 0x01, 0x02);
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:          
                        
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            e_tps92662_set_width(tps_pix32_module1, 0, 0, 100);
            e_tps92662_set_width(tps_pix32_module1, 0, 1, 200);
            e_tps92662_set_width(tps_pix32_module1, 0, 2, 300);
            e_tps92662_set_width(tps_pix32_module1, 0, 3, 400);
            e_tps92662_set_width(tps_pix32_module1, 0, 4, 500);
            e_tps92662_set_width(tps_pix32_module1, 0, 5, 600);
            e_tps92662_set_width(tps_pix32_module1, 0, 6, 700);
            e_tps92662_set_width(tps_pix32_module1, 0, 7, 800);
            e_tps92662_set_width(tps_pix32_module1, 0, 8, 900);
            e_tps92662_set_width(tps_pix32_module1, 0, 9, 1000);
            e_tps92662_set_width(tps_pix32_module1, 0, 10, 0);
            e_tps92662_set_width(tps_pix32_module1, 0, 11, 1023);
            
            e_tps92662_set_width(tps_pix32_module1, 1, 0, 10);
            e_tps92662_set_width(tps_pix32_module1, 1, 1, 20);
            e_tps92662_set_width(tps_pix32_module1, 1, 2, 30);
            e_tps92662_set_width(tps_pix32_module1, 1, 3, 40);
            e_tps92662_set_width(tps_pix32_module1, 1, 4, 50);
            e_tps92662_set_width(tps_pix32_module1, 1, 5, 60);
            e_tps92662_set_width(tps_pix32_module1, 1, 6, 70);
            e_tps92662_set_width(tps_pix32_module1, 1, 7, 80);
            e_tps92662_set_width(tps_pix32_module1, 1, 8, 90);
            e_tps92662_set_width(tps_pix32_module1, 1, 9, 100);
            e_tps92662_set_width(tps_pix32_module1, 1, 10, 110);
            e_tps92662_set_width(tps_pix32_module1, 1, 11, 120);
            
            e_tps92662_set_width(tps_pix32_module1, 2, 0, 810);
            e_tps92662_set_width(tps_pix32_module1, 2, 1, 820);
            e_tps92662_set_width(tps_pix32_module1, 2, 2, 830);
            e_tps92662_set_width(tps_pix32_module1, 2, 3, 840);
            e_tps92662_set_width(tps_pix32_module1, 2, 4, 850);
            e_tps92662_set_width(tps_pix32_module1, 2, 5, 860);
            e_tps92662_set_width(tps_pix32_module1, 2, 6, 870);
            e_tps92662_set_width(tps_pix32_module1, 2, 7, 880);
            e_tps92662_set_width(tps_pix32_module1, 2, 8, 890);
            e_tps92662_set_width(tps_pix32_module1, 2, 9, 900);
            e_tps92662_set_width(tps_pix32_module1, 2, 10, 910);
            e_tps92662_set_width(tps_pix32_module1, 2, 11, 920);
            
            if (!e_tps92662_deamon(&tps_pix32_module1))
            {
                e_tps92662_send_widths(tps_pix32_module1, 0);
                e_tps92662_send_widths(tps_pix32_module1, 1);
                e_tps92662_send_widths(tps_pix32_module1, 2);            
            }
            
            if (mTickCompare(sm_example.tick) >= TICK_20MS)
            {
                sm_example.tick = mGetTick();

                e_tps92662_get_errors(tps_pix32_module1, 0);
                e_tps92662_get_errors(tps_pix32_module1, 1);
                e_tps92662_get_errors(tps_pix32_module1, 2);

                e_tps92662_get_adc(tps_pix32_module1, 0);
                e_tps92662_get_adc(tps_pix32_module1, 1);
                e_tps92662_get_adc(tps_pix32_module1, 2);
            } 
                        
            break;
    } 
}
