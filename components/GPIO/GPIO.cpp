#include <stdio.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pins.h"
#include "mcp23017.h"
#include "GPIO.h"
#include "_debug.h"
#include "app_comm.h"
#include "UART.h"
#include "Handle_sdcard.h"
#include "error.h"
#include "Common.h"
#include "I2C.h"
extern ram_data_t ram_data;

charger_sts_t charger_sts;

void init_gpio(void)
{
    GPIO.enable |= (1UL<<ESP_SDA);
    GPIO.out &= ~(1UL<<ESP_SDA);

    GPIO.enable |= (1UL<<ESP_SCL);
    GPIO.out &= ~(1UL<<ESP_SCL);

    GPIO.enable |= (1UL<<ESP_PWREN);
    GPIO.out |= (1UL<<ESP_PWREN);

    GPIO.enable &= ~(1UL<<ESP_MISO);

    GPIO.enable |= (1UL<<ESP_MOSI);

    GPIO.enable |= (1UL<<ESP_SCK);

    GPIO.enable |= (1UL<<ESP_SD_CS);
    GPIO.out |= (1UL<<ESP_SD_CS);

    GPIO.enable |= (1UL<<ESP_CAN_TX);
    GPIO.out &= ~(1UL<<ESP_CAN_TX);

    /* GPIO.enable1.val &= ~(1 << 0);
    GPIO.out1.val |= (1 << 0);
     */
    gpio_set_direction(GPIO_NUM_32,GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_0,GPIO_MODE_INPUT);
    gpio_set_level(GPIO_NUM_32,1);
    gpio_set_level(GPIO_NUM_0,1);
    /* GPIO.enable |= (1UL<<ESP_NRF_CS);    //PP commented on 29-09-23: maybe GPIO.enable1 & GPIO.out1 are needed here. got the warning "left shift count >= width of type [-Wshift-count-overflow]" on these lines.
    GPIO.out |= (1UL<<ESP_NRF_CS);

    GPIO.enable |= (1UL<<ESP_NRF_CE);
    GPIO.out |= (1UL<<ESP_NRF_CE);

    GPIO.enable |= (1UL<<ESP_SWITCH);
    GPIO.out &= ~(1UL<<ESP_SWITCH);

    GPIO.enable |= (1UL<<ESP_ACCL_INT);
    GPIO.out &= ~(1UL<<ESP_ACCL_INT); */

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
    init_i2c_();
    init_mcp23017();

    MCP23017_set_dir(MCP_LED_RED_DDR, MCP_LED_RED, 0);
    MCP23017_pin_write(MCP_LED_RED_PORT, MCP_LED_RED, 1);

    MCP23017_set_dir(MCP_LED_GREEN_DDR, MCP_LED_GREEN, 0);
    MCP23017_pin_write(MCP_LED_GREEN_PORT, MCP_LED_GREEN, 1);

    MCP23017_set_dir(MCP_LED_BLUE_DDR, MCP_LED_BLUE, 0);
    MCP23017_pin_write(MCP_LED_BLUE_PORT, MCP_LED_BLUE, 1);

    MCP23017_set_dir(MCP_SIM_PWREN_DDR, MCP_SIM_PWREN, 0);  //SIM_PWR_EN as o/p
    MCP23017_set_dir(MCP_SIM_DTR_DDR, MCP_SIM_DTR, 0);      //MDM_DTR as o/p
    MCP23017_set_dir(MCP_SIM_1V8_DDR, MCP_SIM_1V8, 1);      //SIMCOM_1V8 i/p
    MCP23017_set_dir(MCP_SIM_ONOFF_DDR, MCP_SIM_ONOFF, 0);  //MCP_SIM_ON_OFF as o/p
    MCP23017_set_dir(MCP_SIM_RST_DDR, MCP_SIM_RST, 0);      //SIM_RST as o/p

    MCP23017_set_dir(MCP_PWR_LTCH_DDR, MCP_PWR_LTCH, 0);  
    MCP23017_set_dir(MCP_RS485_DDR, MCP_RS485_DIR, 0);  
    MCP23017_set_dir(MCP_IO_IN_DDR, MCP_IO_IN, 1);  
    MCP23017_set_dir(MCP_IO_OUT_DDR, MCP_IO_OUT, 1);  
    MCP23017_set_dir(MCP_CAN_SO_DDR, MCP_CAN_SO, 0); 


    MCP23017_set_dir(MCP_TP_CHARGING_DDR, MCP_TP_CHARGING, 1);  

    MCP23017_set_dir(MCP_TP_CH_EN_DDR, MCP_TP_CH_EN, 0);  
    MCP23017_pin_write(MCP_TP_CH_EN_PORT, MCP_TP_CH_EN, 1);   
}

charger_sts_t get_charger_sts(void)
{
    return charger_sts;
}

void set_charger_sts(charger_sts_t val)
{
    charger_sts = val;
}

void charger_status_atStartup(void)
{
    static uint8_t MCP_portA_read = 0;

    i2c_read_byte(MCP23017_ADDR_BASE, MCP_TP_CHARGING_PORT, &MCP_portA_read);

    if(!(MCP_portA_read & (MCP_TP_CHARGING)))
    {
#ifdef DEBUG_ADC
       UWriteString((char*)"\nc1:",UART_PC);
       UWriteInt(MCP_portA_read, UART_PC);
#endif
       vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 milliseconds 

       i2c_read_byte(MCP23017_ADDR_BASE, MCP_TP_CHARGING_PORT, &MCP_portA_read);

       if(!(MCP_portA_read & (MCP_TP_CHARGING)))
       {
#ifdef DEBUG_ADC
            UWriteString((char*)"\nc2:",UART_PC);
            UWriteInt(MCP_portA_read, UART_PC);
#endif
            set_charger_sts(CHARGING);
       }
       else
       {
#ifdef DEBUG_ADC
            UWriteString((char*)"\nnc1:",UART_PC);
            UWriteInt(MCP_portA_read, UART_PC);
#endif
            set_charger_sts(NOT_CHARGING);
       }
    }
    else
    {
#ifdef DEBUG_ADC
        UWriteString((char*)"\nnc2:",UART_PC);
        UWriteInt(MCP_portA_read, UART_PC);
#endif
        set_charger_sts(NOT_CHARGING);
    }
}

void check_charger_status(void)
{
    static unsigned char cnt = 0;
    static uint8_t MCP_portA_read = 0;

    switch (get_charger_sts())
    {
        case NOT_CHARGING:
        {
            i2c_read_byte(MCP23017_ADDR_BASE, MCP_TP_CHARGING_PORT, &MCP_portA_read);

            if(!(MCP_portA_read & (MCP_TP_CHARGING)))
            {
                if(++cnt > 2)
                {
#ifdef DEBUG_ADC
                    // UWriteString((char*)"\nc3:",UART_PC);
                    // UWriteInt(MCP_portA_read, UART_PC);
#endif
                    cnt = 0;
                    set_charger_sts(CHARGING);
                }
            }
            else
            {
#ifdef DEBUG_ADC
                // UWriteString((char*)"\nnc3:",UART_PC);
                // UWriteInt(MCP_portA_read, UART_PC);
#endif
                cnt = 0;
            }
        }
        break;

        case CHARGING:
        {
            i2c_read_byte(MCP23017_ADDR_BASE, MCP_TP_CHARGING_PORT, &MCP_portA_read);

            if((MCP_portA_read & (MCP_TP_CHARGING)))
            {
                if(++cnt > 2)
                {
#ifdef DEBUG_ADC
                    // UWriteString((char*)"\nnc4:",UART_PC);
                    // UWriteInt(MCP_portA_read, UART_PC);
#endif
                    cnt = 0;
                    set_charger_sts(NOT_CHARGING);
                }
            }
        }
        break;

        default:
        {
#ifdef DEBUG_ADC
            //UWriteString((char*)"\nnc5",UART_PC);
#endif
            set_charger_sts(NOT_CHARGING);
        }
        break;
    }
}

void control_RGB(void)
{
	static int counter = 0;
	static uint8_t led_state = 0;
    static uint16_t toggle_cnt=0;
	uint8_t blink_count = 0;
    unsigned int led_off_time = 20,led_on_time = 1;
    
    if(ram_data.Status & (1<<SYS_SD_EJECT))
    {
        if(toggle_cnt++ >= 100)
        {
            toggle_cnt=0;
            printf("\nEject_low");
            set_system_state(SYS_SD_EJECT,LOW);
        }
        else
        {
            printf("\ntoggle_pin");
            MCP23017_pin_toggle(MCP_LED_RED_PORT,MCP_LED_RED);
        }
    }
    else 
    {
        switch (get_charger_sts())
        {
            case CHARGING:
            {
                set_system_state(SYS_BATT_LOW, LOW);
                set_system_state(SYS_BATT_FULL, LOW);
    #ifdef DEBUG_ADC
                //printf("CH\n");
    #endif
                blink_count = 1;
                if(counter++ >= blink_count)
                {
                    if (led_state == 0)
                    {
                        //set_RGB_led(RGB_WHITE);
                        if((ram_data.Status & (1<<SYS_SD_CARD)) || (ram_data.Status & (1<<SYS_SD_CARD_INIT)) || (ram_data.Status & (1<<SYS_GSM_SIM)))
                        {
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED), 1);
                        }
                        else 
                        {
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_BLUE), 1);
                        }
                        led_state = 1;
                    }
                    else
                    {
                        //set_RGB_led(RGB_OFF);
                        MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
                        led_state = 0;
                    }
                    counter = 0;
                }
            }
            break;

            case NOT_CHARGING:
            {
                if(ram_data.v_batt > 4000U)
                {
                    set_system_state(SYS_BATT_FULL, HIGH);
                    set_system_state(SYS_BATT_LOW, LOW);
    #ifdef DEBUG_ADC
                    //printf("FULL\n");
    #endif
                    blink_count = 5;
                    if(counter++ >= blink_count)
                    {
                        if (led_state == 0)
                        {
                            //set_RGB_led(RGB_WHITE);
                            if((ram_data.Status & (1<<SYS_SD_CARD)) || (ram_data.Status & (1<<SYS_SD_CARD_INIT)) || (ram_data.Status & (1<<SYS_GSM_SIM)))
                            {
                                MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED), 1);
                            }
                            else 
                            {
                                MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN), 1);
                            }
                            led_state = 1;
                        }
                        else
                        {
                            //set_RGB_led(RGB_OFF);
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
                            led_state = 0;
                        }
                        counter = 0;
                    }
                }
                else if(ram_data.v_batt <= 3000U)
                {
                    set_system_state(SYS_BATT_LOW, HIGH);
                    set_system_state(SYS_BATT_FULL, LOW);
    #ifdef DEBUG_ADC
                    //printf("LOW\n");
    #endif
    #if 0
                    blink_count = 10;
                    if(counter++ >= blink_count)
                    {
                        if (led_state == 0)
                        {
                            //set_RGB_led(RGB_WHITE);
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_RED), 1);
                            led_state = 1;
                        }
                        else
                        {
                            //set_RGB_led(RGB_OFF);
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
                            led_state = 0;
                        }
                        counter = 0;
                    }
    #endif
    #if 1
                    switch(led_state)
                    {
                        case 0:
                        {
                            if(counter++ > led_off_time)
                            {
                                counter = 0;
                                led_state = 1;

                                if((ram_data.Status & (1<<SYS_SD_CARD)) || (ram_data.Status & (1<<SYS_SD_CARD_INIT)) || (ram_data.Status & (1<<SYS_GSM_SIM)))
                                {
                                    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED), 1);
                                }
                                else 
                                {
                                    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_RED), 1);
                                }
                                
                            }
                        }
                        break;
                        
                        case 1:
                        {
                            if(counter++ > led_on_time)
                            {
                                counter = 0;
                                led_state = 0;
                                MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
                            }
                        }
                        break;

                        default:
                        break;
                    }
    #endif
                }
                else
                {
                    set_system_state(SYS_BATT_LOW, LOW);
                    set_system_state(SYS_BATT_FULL, LOW);
    #ifdef DEBUG_ADC
                    //printf("NC\n");
    #endif
                    blink_count = 10;
                    if(counter++ >= blink_count)
                    {
                        if (led_state == 0)
                        {
                            //set_RGB_led(RGB_WHITE);
                            
                            if((ram_data.Status & (1<<SYS_SD_CARD)) || (ram_data.Status & (1<<SYS_SD_CARD_INIT)) || (ram_data.Status & (1<<SYS_GSM_SIM)))
                            {
                                MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED), 1);
                            }
                            else 
                            {
                                MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 1);
                            }
                            led_state = 1;
                        }
                        else
                        {
                            //set_RGB_led(RGB_OFF);
                            MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
                            led_state = 0;
                        }
                        counter = 0;
                    }
                }
            }
            break;

            default:
            break;
        }
    }
}