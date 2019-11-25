/*********************************************************************
 * 
 *	Examples for the PLIB and its (Internal / External) Peripherals.
 * 
 *	Author : Sébastien PERREAU
 * 
*********************************************************************/

#include "../PLIB.h"

/*
 * IMPORTANT: The X value of "if (mTickCompare(tick) >= X)" can not exceed (2^32 - 1)
 * So with a frequency of 80 MHz, the maximum of "tick compare" is: (2^32 - 1) / TICK_1MS = 53,687 sec
 * The 'if' function can not compare values greater than 32 bits.
 */
void _EXAMPLE_TICK_FUNCTIONS()
{
    static uint8_t count;
    static uint64_t tick = 0;
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
                    mResetTime();
                }
                else if (count == 50)
                {
                    Delay_ms(20);
                }
                else if (count == 100)
                {                    
                    if (!switch_update_tick)
                    {
                        LOG("Time Update Tick: %d", mGetTime());    
                    }
                    else
                    {
                        LOG("Time Update Tick with Catching Up Time: %d", mGetTime());    
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
    static dma_channel_transfer_t dma_tx = {buff_src, buff_dst, 200, 200, 20, 0x0000};
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
            
            dma_set_transfer(dma_id, &dma_tx, true);    // Force the transfer because no EVENT (DMA_EVT_NONE) has been set on dma_id.
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
    that is to say that each time a UART1 Tx is free to use a new DMA cell block is transmitted (if 
    the DMA channel is ENABLE - no need to force the transfer) and so on up to all data source is 
    transmitted. At the end, a DMA_INT_BLOCK_TRANSFER_DONE is set, the event handler is called and 
    the DMA channel turns off automatically (because there is no AUTO_ENABLE option for DMA6 Channel). 
    The 2 events that can generate an interruption are DMA_INT_TRANSFER_ABORD and DMA_INT_BLOCK_TRANSFER_DONE.
 
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
    This DMA Channel is set with DMA_CONT_AUTO_ENABLE in order to keep the channel ENABLE even after
    a Block Transfer Done or a Pattern Match (same behavior as Block Transfer Done). Thus we can
    continue to receive data and store in the RAM buffer (no need to "manually" re-ENABLE the DMA channel).

    When using Pattern Match mode and a pattern is detected then a DMA_INT_BLOCK_TRANSFER_DONE
    is set. Interrupt can be called (if enable) and the DMA channel behavior is the same
    as Block Transfer Complete (DMA_INT_BLOCK_TRANSFER_DONE flag is set and DMA channel is
    disable).

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
    static dma_channel_transfer_t dma6_tx = {buff_src, NULL, 200, 1, 1, 0x0000};
    static dma_channel_transfer_t dma7_rx = {NULL, buff_src, 1, 200, 1, 0xdead};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            mUpdateLedStatusD2(OFF);
            mUpdateLedStatusD3(OFF);
            
            uart_init(  uart_id, 
                        NULL, 
                        IRQ_NONE, 
                        UART_BAUDRATE_2M, 
                        UART_STD_PARAMS);
            
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
            
            dma_set_transfer(DMA7, &dma7_rx, false);    // Do not force the transfer (it occurs automatically when data is received - UART Rx generates the transfer)
            dma_set_transfer(DMA6, &dma6_tx, true);     // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).            
            
            sm_example.index = _MAIN;
            mUpdateTick(sm_example.tick);
            break;
            
        case _MAIN:
            
            // Do what you want...  
            if (!dma_channel_is_enable(DMA6))
            {
                if (mTickCompare(sm_example.tick) >= TICK_1S)
                {
                    mUpdateTick(sm_example.tick);
                    // Re-execute the DMA transfer RAM -> UART TX
                    dma_force_transfer(DMA6);
                }
            }            
            break;
    } 
}
/* -------------------------------------------------------------------------------------------
 * End of   _EXAMPLE_DMA_UART()
 *********************************************************************************************/

/*********************************************************************************************
 * Start of _EXAMPLE_DMA_SPI()
 * -------------------------------------------------------------------------------------------

 ALWAYS CONFIGURE DMA RX BEFORE DMA TX !!! Otherwise the DMA RX can missed one byte.

 *********************************************************************************************/
static void _example_dma_spi_event_handler(uint8_t id, DMA_CHANNEL_FLAGS flags)
{
    if ((flags & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
    {
        if (id == DMA6)
        {            
            mToggleLedStatusD2();
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
    static dma_channel_transfer_t dma6_tx = {buff_src, NULL, 20, 1, 1, 0x0000};
    static dma_channel_transfer_t dma7_rx = {NULL, buff_src, 1, 20, 1, 0x0000};
    static state_machine_t sm_example = {0};
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            mUpdateLedStatusD2(OFF);
            mUpdateLedStatusD3(OFF);
            // Initialize chip select
            mInitIOAsOutput(__PE0);
            mSetIO(__PE0);
            
            spi_init(   SPI1, 
                        NULL, 
                        IRQ_NONE, 
                        10000000, 
                        SPI_STD_MASTER_CONFIG);

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
            
            for (i = 0 ; i < 200 ; i++)
            {
                buff_src[i] = i;
            }
            
            dma6_tx.src_size = 30;
            dma7_rx.dst_size = dma6_tx.src_size;
            
            mClrIO(__PE0);
            dma_set_transfer(DMA7, &dma7_rx, false);    // Do not force the transfer (it occurs automatically when data is received - SPI Rx generates the transfer)
            dma_set_transfer(DMA6, &dma6_tx, true);     // Do not take care of the boolean value because the DMA channel is configure to execute a transfer on event when Tx is ready (IRQ source is Tx of a peripheral - see notes of dma_set_transfer()).            
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            // Do what you want...
            if ((dma_get_flags(DMA7) & DMA_FLAG_BLOCK_TRANSFER_DONE) > 0)
            {
                dma_clear_flags(DMA7, DMA_FLAG_BLOCK_TRANSFER_DONE);  
                mSetIO(__PE0);
                mUpdateLedStatusD3(ON);             
            }
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
    LED_DEF(led, &spwm.pwm[0], OFF, 140, TICK_1MS, TICK_5MS);
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            software_pwm_init(&spwm);    
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            led.enable = fu_turn_indicator(ON, TICK_1S, TICK_1S);
            
            spwm.pwm[1] = 50;
            spwm.pwm[2] = 100;
            spwm.pwm[3] = 180;
            spwm.pwm[4] = 253;
            spwm.pwm[5] = 254;
            
            spwm.pwm[6] = 0;
            spwm.pwm[7] = 255;
            break;
    } 
    
    fu_led(&led);
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

void _EXAMPLE_25LC512()
{
//    _25LC512_DEF(e_25lc512, SPI2, __PD3, TICK_20MS, 150, 150);
//    BUS_MANAGEMENT_DEF(bm_spi, &e_25lc512.spi_params.bus_management_params);
//    static state_machine_t sm_example = {0};
//    static uint64_t tickAntiFloodSw1 = 0;
//    static uint64_t tickAntiFloodSw2 = 0;
//    static uint64_t tickRead = 0;
//    
//    switch (sm_example.index)
//    {
//        case _SETUP:
//            e_25lc512_check_for_erasing_memory(&e_25lc512, &bm_spi);
//            sm_example.index = _MAIN;
//            break;
//            
//        case _MAIN:
//            if (!mGetIO(SWITCH1) && (mTickCompare(tickAntiFloodSw1) > TICK_200MS))
//            {
//                tickAntiFloodSw1 = mGetTick();
//                if (!e_25lc512_is_write_in_progress(e_25lc512))
//                {
//                    // !! BE SURE TO HAVE ENOUGH SPACE INTO THE BUFFER !!
//                    uint8_t i = 0;
//                    e_25lc512.registers.dW.size = 5;
//                    for(i = 0 ; i < (e_25lc512.registers.dW.size) ; i++)
//                    {
//                        e_25lc512.registers.dW.p[i] = 0x44+i;
//                    }
//                    e_25lc512_write_bytes(e_25lc512, 127);
//                }
//            }
//
//            if (!mGetIO(SWITCH2) && (mTickCompare(tickAntiFloodSw2) > TICK_200MS))
//            {
//                tickAntiFloodSw2 = mGetTick();
//                e_25lc512_chip_erase(e_25lc512);
//            }
//
//            if (mTickCompare(tickRead) >= TICK_100MS)
//            {
//                tickRead = mGetTick();
//                e_25lc512_read_bytes(e_25lc512, 127, 5);
//            }
//
//            if (!e_25lc512_is_read_in_progress(e_25lc512))
//            {
//                if (e_25lc512.registers.dR.p[0] == 0x44)
//                {
//                    mUpdateLedStatusD2(ON);
//                }
//                else
//                {
//                    mUpdateLedStatusD2(OFF);
//                }
//            }
//            
//            fu_bus_management_task(&bm_spi);
//            e_25lc512_deamon(&e_25lc512);
//            break;
//            
//    }
}

void _EXAMPLE_MCP23S17()
{
    static state_machine_t sm_example = {0};
    static uint8_t pwm[16] = {0};
    static uint8_t counter = 0;
    static uint64_t tick_counter = 0;
    MCP23S17_DEF(mcp23s17, SPI4, __PB14, 0x00, 0x01);
    LED_DEF(led1, &pwm[0], OFF, 255, TICK_5MS, TICK_5MS);
    LED_DEF(led2, &pwm[1], ON, 10, TICK_5MS, TICK_5MS);
    LED_DEF(led3, &pwm[2], ON, 30, TICK_5MS, TICK_5MS);
    LED_DEF(led4, &pwm[3], ON, 50, TICK_5MS, TICK_5MS);
    LED_DEF(led5, &pwm[4], ON, 70, TICK_5MS, TICK_5MS);
    LED_DEF(led6, &pwm[5], ON, 90, TICK_5MS, TICK_5MS);
    LED_DEF(led7, &pwm[6], ON, 110, TICK_5MS, TICK_5MS);
    LED_DEF(led8, &pwm[7], ON, 130, TICK_5MS, TICK_5MS);
    LED_DEF(led9, &pwm[8], OFF, 150, TICK_5MS, TICK_5MS);
    LED_DEF(led10, &pwm[9], ON, 170, TICK_5MS, TICK_5MS);
    LED_DEF(led11, &pwm[10], ON, 190, TICK_5MS, TICK_5MS);
    LED_DEF(led12, &pwm[11], ON, 210, TICK_5MS, TICK_5MS);
    LED_DEF(led13, &pwm[12], ON, 230, TICK_5MS, TICK_5MS);
    LED_DEF(led14, &pwm[13], ON, 250, TICK_5MS, TICK_5MS);
    LED_DEF(led15, &pwm[14], ON, 0, TICK_5MS, TICK_5MS);
    LED_DEF(led16, &pwm[15], ON, 255, TICK_5MS, TICK_5MS);    
    
    switch (sm_example.index)
    {
        case _SETUP:
            
            mcp23s17.write[0].IODIRA = 0x00;
            mcp23s17.write[0].IODIRB = 0x00;
            mcp23s17.write[1].IODIRB = 0x00;
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (mTickCompare(tick_counter) >= TICK_100US)
            {
                mUpdateTick(tick_counter);
                
                uint8_t i = 0, ind;
                for (i = 0 ; i < 16 ; i++)
                {
                    uint8_t t_pwm = pwm[i] / 4;
                    ind = i%8;
                    if (t_pwm >= 63)
                    {
                        if (i < 8)
                        {
                            SET_BIT(mcp23s17.write[0].OLATA, ind);
                            SET_BIT(mcp23s17.write[0].GPIOA, ind);
                        }
                        else
                        {
                            SET_BIT(mcp23s17.write[0].OLATB, ind);
                            SET_BIT(mcp23s17.write[0].GPIOB, ind);
                        }
                    }
                    else if (t_pwm > (counter & 0x3f))
                    {
                        if (i < 8)
                        {
                            SET_BIT(mcp23s17.write[0].OLATA, ind);
                            SET_BIT(mcp23s17.write[0].GPIOA, ind);
                        }
                        else
                        {
                            SET_BIT(mcp23s17.write[0].OLATB, ind);
                            SET_BIT(mcp23s17.write[0].GPIOB, ind);
                        }
                    }
                    else
                    {
                        if (i < 8)
                        {
                            CLR_BIT(mcp23s17.write[0].OLATA, ind);
                            CLR_BIT(mcp23s17.write[0].GPIOA, ind);
                        }
                        else
                        {
                            CLR_BIT(mcp23s17.write[0].OLATB, ind);
                            CLR_BIT(mcp23s17.write[0].GPIOB, ind);
                        }                    
                    }
                }
                counter++;
            }
            
            if (fu_turn_indicator(ON, TICK_1S, TICK_1S))
            {
                led1.enable = ON;
                
                mcp23s17.write[1].OLATB = 0x55;
                mcp23s17.write[1].GPIOB = 0x55;
            }
            else
            {
                led1.enable = OFF;
                
                mcp23s17.write[1].OLATB = 0xaa;
                mcp23s17.write[1].GPIOB = 0xaa;
            }

            mUpdateLedStatusD2(GET_BIT(mcp23s17.read[0].GPIOA, 0));
            
            fu_led(&led1);
            fu_led(&led2);
            fu_led(&led3);
            fu_led(&led4);
            fu_led(&led5);
            fu_led(&led6);
            fu_led(&led7);
            fu_led(&led8);
            fu_led(&led9);
            fu_led(&led10);
            fu_led(&led11);
            fu_led(&led12);
            fu_led(&led13);
            fu_led(&led14);
            fu_led(&led15);
            fu_led(&led16);
            e_mcp23s17_deamon(&mcp23s17);
            
            break;
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
    static uint8_t tab[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
    
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
                    
                case 9:
                    if (mTickCompare(sm_log.tick) >= TICK_200MS)
                    {
                        char s[50];
                        transform_uint8_t_tab_to_string(s, sizeof(s), tab, sizeof(tab), BASE_16);
                        LOG("%s", p_string(s));
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

static void _example_uart_event_handler(uint8_t id, IRQ_EVENT_TYPE evt_type, uint32_t data)
{
    switch (evt_type)
    {
        case IRQ_UART_ERROR:
            
            break;
            
        case IRQ_UART_RX:
            
            mToggleLedStatusD2();
            break;
            
        case IRQ_UART_TX:
            
            break;
            
    }
}

void _EXAMPLE_UART()
{
    static state_machine_t sm_example = {0};
    SWITCH_DEF(sw1, SWITCH1, ACTIVE_LOW);
    static char tab_string[] = "Helloooo !";
    static uint8_t ind_string = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
      
            uart_init(UART1, _example_uart_event_handler, IRQ_UART_RX, 115200, UART_STD_PARAMS);
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
    LIN_FRAME_DEF(lin_frame1, LIN_WRITE_REQUEST, 0x3c, LIN_AUTO_DATA_LENGTH, TICK_100MS);
    LIN_FRAME_DEF(lin_frame2, LIN_WRITE_REQUEST, 0x31, LIN_1_DATA_BYTE, TICK_50MS);
    LIN_FRAME_DEF(lin_frame3, LIN_WRITE_REQUEST, 0x32, LIN_2_DATA_BYTE, TICK_50MS);
    LIN_FRAME_DEF(lin_frame4, LIN_WRITE_REQUEST, 0x33, LIN_5_DATA_BYTE, TICK_100MS);
    LIN_FRAME_DEF(lin_frame5, LIN_READ_REQUEST, 0x22, LIN_6_DATA_BYTE, TICK_100MS);
    LIN_FRAME_DEF(lin_frame6, LIN_WRITE_REQUEST, 0x34, LIN_AUTO_DATA_LENGTH, LIN_NOT_PERIODIC);
    
    LIN_DEF(lin_bus, UART2, LIN_ENABLE_PIN, LIN_VERSION_2_X, &lin_frame1, &lin_frame2, &lin_frame3, &lin_frame4, &lin_frame5, &lin_frame6);    
     
    SWITCH_DEF(sw2, SWITCH2, ACTIVE_LOW);
    
    static state_machine_t sm_example = {0};
    static state_machine_t sm = {0};
    static uint64_t tick_force_transfer = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
            
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            mUpdateTick(sm.tick);
            
            if (sw2.is_updated)
            {
                sw2.is_updated = false;
                lin_frame1.force_transfer.execute = true;
                lin_frame5.force_transfer.execute = true;
            }
            
            if (lin_frame5.is_updated)
            {
                lin_frame5.is_updated = false;
                // Read bytes...
            }
            else if (lin_frame5.errors.is_occurs)
            {
                lin_frame5.errors.is_occurs = false;
                // Do something...
            }
            
            if (mTickCompare(tick_force_transfer) >= TICK_40MS)
            {
                if (!lin_force_transfer(&lin_frame6))
                {
                    mUpdateTick(tick_force_transfer);
                    if (lin_frame6.is_updated)
                    {
                        // Frame is transmitted
                    }
                    else if (lin_frame6.errors.is_occurs)
                    {
                        // You can check which error(s) occur(s) ...
                    }
                }
            }
                            
            lin_frame1.data[0] = lin_frame5.errors.timing >> 8;
            lin_frame1.data[1] = lin_frame5.errors.timing >> 0;
            lin_frame1.data[2] = lin_frame5.errors.readback >> 8;
            lin_frame1.data[3] = lin_frame5.errors.readback >> 0;
            lin_frame1.data[4] = sm.tick >> 24;
            lin_frame1.data[5] = sm.tick >> 16;
            lin_frame1.data[6] = sm.tick >> 8;
            lin_frame1.data[7] = sm.tick >> 0;
            
            lin_master_deamon(&lin_bus);
            fu_switch(&sw2);
            
            break;
    } 
}

void _EXAMPLE_PINK_LADY()
{
    PINK_LADY_DEF(smartled, SPI2, SK6812RGBW_MODEL, 50);
    PICK_LADY_SHIFT_DEF(shift1, smartled, PL_SHIFT_FROM_TO_TO, 0, 49, 5, TICK_20MS);
    SLIDER_DEF(slider_w, 50, SLIDER_START_TO_END, SLIDER_CENTER_TO_ENDS, TICK_800MS, TICK_400MS);
    SLIDER_DEF(slider_r, 50, SLIDER_CENTER_TO_ENDS, SLIDER_START_TO_END, TICK_400MS, TICK_800MS);
    static RGBW_COLOR pattern[] = {
        {255, 0, 0, 0}, {255, 10, 0, 0}, {255, 20, 0, 0}, {255, 30, 0, 0}, {255, 40, 0, 0}, {255, 50, 0, 0}, {255, 60, 0, 0}, {255, 70, 0, 0}, {255, 80, 0, 0}, {255, 90, 0, 0}, 
        {255, 100, 0, 0}, {255, 110, 0, 0}, {255, 120, 0, 0}, {255, 130, 0, 0}, {255, 140, 0, 0}, {255, 150, 0, 0}, {255, 160, 0, 0}, {255, 170, 0, 0}, {255, 180, 0, 0}, {255, 190, 0, 0}, 
        {255, 200, 0, 0}, {255, 210, 0, 0}, {255, 220, 0, 0}, {255, 230, 0, 0}, {255, 240, 0, 0}, {255, 250, 0, 0}, {250, 255, 0, 0}, {240, 255, 0, 0}, {230, 255, 0, 0}, {220, 255, 0, 0}, 
        {210, 255, 0, 0}, {200, 255, 0, 0}, {190, 255, 0, 0}, {180, 255, 0, 0}, {170, 255, 0, 0}, {160, 255, 0, 0}, {150, 255, 0, 0}, {140, 255, 0, 0}, {130, 255, 0, 0}, {120, 255, 0, 0}, 
        {110, 255, 0, 0}, {100, 255, 0, 0}, {90, 255, 0, 0}, {80, 255, 0, 0}, {70, 255, 0, 0}, {60, 255, 0, 0}, {50, 255, 0, 0}, {40, 255, 0, 0}, {30, 255, 0, 0}, {20, 255, 0, 0}, 
        {10, 255, 0, 0}, {0, 255, 0, 0}, {0, 255, 10, 0}, {0, 255, 20, 0}, {0, 255, 30, 0}, {0, 255, 40, 0}, {0, 255, 50, 0}, {0, 255, 60, 0}, {0, 255, 70, 0}, {0, 255, 80, 0}, 
        {0, 255, 90, 0}, {0, 255, 100, 0}, {0, 255, 110, 0}, {0, 255, 120, 0}, {0, 255, 130, 0}, {0, 255, 140, 0}, {0, 255, 150, 0}, {0, 255, 160, 0}, {0, 255, 170, 0}, {0, 255, 180, 0}, 
        {0, 255, 190, 0}, {0, 255, 200, 0}, {0, 255, 210, 0}, {0, 255, 220, 0}, {0, 255, 230, 0}, {0, 255, 240, 0}, {0, 255, 250, 0}, {0, 250, 255, 0}, {0, 240, 255, 0}, {0, 230, 255, 0}, 
        {0, 220, 255, 0}, {0, 210, 255, 0}, {0, 200, 255, 0}, {0, 190, 255, 0}, {0, 180, 255, 0}, {0, 170, 255, 0}, {0, 160, 255, 0}, {0, 150, 255, 0}, {0, 140, 255, 0}, {0, 130, 255, 0}, 
        {0, 120, 255, 0}, {0, 110, 255, 0}, {0, 100, 255, 0}, {0, 90, 255, 0}, {0, 80, 255, 0}, {0, 70, 255, 0}, {0, 60, 255, 0}, {0, 50, 255, 0}, {0, 40, 255, 0}, {0, 30, 255, 0}, 
        {0, 20, 255, 0}, {0, 10, 255, 0}, {0, 0, 255, 0}, {10, 0, 255, 0}, {20, 0, 255, 0}, {30, 0, 255, 0}, {40, 0, 255, 0}, {50, 0, 255, 0}, {60, 0, 255, 0}, {70, 0, 255, 0}, 
        {80, 0, 255, 0}, {90, 0, 255, 0}, {100, 0, 255, 0}, {110, 0, 255, 0}, {120, 0, 255, 0}, {130, 0, 255, 0}, {140, 0, 255, 0}, {150, 0, 255, 0}, {160, 0, 255, 0}, {170, 0, 255, 0}, 
        {180, 0, 255, 0}, {190, 0, 255, 0}, {200, 0, 255, 0}, {210, 0, 255, 0}, {220, 0, 255, 0}, {230, 0, 255, 0}, {240, 0, 255, 0}, {250, 0, 255, 0}, {255, 0, 250, 0}, {255, 0, 240, 0}, 
        {255, 0, 230, 0}, {255, 0, 220, 0}, {255, 0, 210, 0}, {255, 0, 200, 0}, {255, 0, 190, 0}, {255, 0, 180, 0}, {255, 0, 170, 0}, {255, 0, 160, 0}, {255, 0, 150, 0}, {255, 0, 140, 0}, 
        {255, 0, 130, 0}, {255, 0, 120, 0}, {255, 0, 110, 0}, {255, 0, 100, 0}};
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
                
                pink_lady_release_segment(smartled, PL_ID_0);
                pink_lady_release_segment(smartled, PL_ID_1);
                
                pink_lady_shift_pattern_reset_and_start(shift1);
                
                slider_w.enable = OFF;
                slider_r.enable = OFF;
            }

            switch (sm_colors.index)
            {
                case 0:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 49, RGBW_COLOR_WHITE, RGBW_COLOR_WHITE_MIX, LED_RESO_JUMP | LED_RESO_1_5, TICK_1S);
                    break;
                    
                case 1:
                    pink_lady_set_all_led_off(smartled);
                    break;
                    
                case 2:
                    pink_lady_set_all_led_on(smartled);
                    break;
                
                case 3:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 24, RGBW_COLOR_RED, RGBW_COLOR_RED, LED_RESO_ALL, 0);
                    pink_lady_set_segment_params(&smartled, PL_ID_1, 25, 49, RGBW_COLOR_RED, RGBW_COLOR_BLUE, LED_RESO_ALL, TICK_1S);                    
                    break;
                
                case 4:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 25, 49, RGBW_COLOR_GREEN, RGBW_COLOR_WHITE, LED_RESO_ALL, TICK_1S);
                    break;
                     
                case 5:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 24, RGBW_COLOR_BLUE, RGBW_COLOR_BLUE, LED_RESO_JUMP | LED_RESO_1_2, TICK_1S);
                    pink_lady_set_segment_params(&smartled, PL_ID_1, 25, 49, RGBW_COLOR_BLUE, RGBW_COLOR_BLUE, LED_RESO_CLEAR | LED_RESO_1_2, 0);
                    break;
                    
                case 6:
                    pink_lady_shift_pattern(&shift1);
                    break;
                    
                case 7:
                    pink_lady_shift_pattern_toggle_direction(shift1);
                    pink_lady_shift_pattern_start(shift1);
                    sm_colors.index++;
                    break;
                    
                case 8:
                    pink_lady_shift_pattern(&shift1);
                    break;
                    
                case 9:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 24, RGBW_COLOR_BLUE, RGBW_COLOR_BLUE, LED_RESO_ALL, 0);
                    pink_lady_set_segment_params(&smartled, PL_ID_1, 25, 49, RGBW_COLOR_BLUE, RGBW_COLOR_BLUE, LED_RESO_ALL, TICK_1S);
                    break;
                    
                case 10:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 49, RGBW_COLOR_OFF, RGBW_COLOR_OFF, LED_RESO_ALL, TICK_500MS);
                    if (pink_lady_get_segment_status(smartled, PL_ID_0) == PL_SEGMENT_FINISHED)
                    {
                        sm_colors.index++;
                        mUpdateTick(sm_colors.tick);
                    }
                    break;
                    
                case 11:
                    pink_lady_put_pattern(smartled, pattern, 5, 45);
                    pink_lady_shift_pattern_toggle_direction(shift1);
                    break;
                    
                case 12:
                    pink_lady_shift_pattern(&shift1);
                    break;
                    
                case 13:
                    pink_lady_set_segment_params(&smartled, PL_ID_0, 0, 49, RGBW_COLOR_OFF, RGBW_COLOR_OFF, LED_RESO_ALL, TICK_500MS);
                    if (pink_lady_get_segment_status(smartled, PL_ID_0) == PL_SEGMENT_FINISHED)
                    {
                        sm_colors.index++;
                        mUpdateTick(sm_colors.tick);
                    }
                    break;
                    
                case 14:
                    pink_lady_set_led_rgbw(smartled, 0, 255, 0, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 9, 0, 255, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 19, 0, 0, 255, 0);
                    pink_lady_set_led_rgbw(smartled, 29, 0, 0, 0, 255);
                    pink_lady_set_led_rgbw(smartled, 39, 255, 255, 0, 0);
                    pink_lady_set_led_rgbw(smartled, 49, 0, 255, 255, 0);
                    break;
                    
                case 15:
                    slider_w.enable = fu_turn_indicator(ON, TICK_1S, TICK_1S);
                    slider_r.enable = !fu_turn_indicator(ON, TICK_1S, TICK_1S);

                    {
                        uint8_t i;
                        for (i = 0 ; i < slider_w.p_output.size ; i++)
                        {
                            pink_lady_set_led_rgbw(smartled, i, slider_r.p_output.p[i] ? 255 : 0, 0, 0, slider_w.p_output.p[i] ? 255 : 0);
                        }
                    }
                    break;
                    
                default:
                    sm_colors.index = 0;
                    break;
            }

            fu_slider(&slider_w);
            fu_slider(&slider_r);
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

void _EXAMPLE_SD_CARD()
{
    SWITCH_DEF(sw1, SWITCH2, ACTIVE_LOW);
    SWITCH_DEF(sw2, SWITCH3, ACTIVE_LOW);
    SD_CARD_DEF(sd, SPI1, __PB14, ENABLE);
    FILE_DEF(sd, mFile1, "pict1.mextension");
    FILE_DEF(sd, mFile2, "short_movie_side_2.nff");
    FILE_DEF(sd, mFile3, "Nouveau dossier\\Fold\\ParICI\\seb.txt");
    static state_machine_t sm_example = {0};
    static uint8_t read_buffer[200];
    static uint8_t progression, prev_progression;
    
    static uint32_t address = 0;
    
    switch (sm_example.index)
    {
        case _SETUP:          
                      
            sd_card_open(&mFile1);
            sd_card_open(&mFile2);
            sd_card_open(&mFile3);
            sm_example.index = _MAIN;
            break;
            
        case _MAIN:
            
            if (sw1.is_updated)
            {
                sw1.is_updated = false;
                sd_card_read_file_restart_playback(mFile1);
                sd_card_read_file_restart_playback(mFile2);
                sd_card_read_file_restart_playback(mFile3);
            }
            
            if (sw2.is_updated)
            {
                sw2.is_updated = false;
                sd_card_read_file_restart_playback(mFile1);
            }
            
            switch (sw1.indice)
            {
                case 0:
                    
                    if (!sd_card_read_block_file(&mFile1, (uint8_t *) read_buffer, address, sizeof(read_buffer)))
                    {
                        // Reception completed. Manipulate read_buffer...
                        address += sizeof(read_buffer);
                        if ((address + sizeof(read_buffer)) > mFile1.file_size)
                        {
                            address = 0;
                        }
                    }
                    break;
                    
                case 1:
                    
                    if (!sd_card_read_play_file(&mFile2, (uint8_t *) read_buffer, sizeof(read_buffer), TICK_30MS, &progression))
                    {
                        sd_card_read_file_restart_playback(mFile2);
                    }
                    break;
                    
                case 2:
                    
                    if (!sd_card_read_block_file(&mFile3, (uint8_t *) read_buffer, 0, sizeof(read_buffer)))
                    {
                        // Reception completed. Manipulate read_buffer...
                    }
                    break;
                    
                default:
                    
                    sw1.indice = 0;
                    break;
            }
            
            if (prev_progression != progression)
            {
                prev_progression = progression;
                LOG_BLANCK("progression: %d", progression);
            }
            
            fu_switch(&sw1);
            
            fu_switch(&sw2);
            
            sd_card_deamon(&sd);
            
            break;
    } 
}
