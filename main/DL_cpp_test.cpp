#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "my_rtc.h"
#include "timer.h"
#include "mcp23017.h"
#include "UART.h"
#include "gprs.h"
#include "gps.h"
#include "web_comm.h"
#include "app_comm.h"
#include "pins.h"
#include "_debug.h"
#include "Shubham.h"
#include "EEPROM.h"
#include "Handle_sdcard.h"
#include "adc.h"
#include "error.h"
#include "GPIO.h"
#include "soc/gpio_struct.h"
#include "lis3dh.h"

extern schedular_flg_t schedular_flg;
extern uart_hal_context_t uart_hal0;
extern uart_hal_context_t uart_hal1;
extern volatile uint8_t test_uart0_intr_flg;
extern volatile uint8_t test_uart1_intr_rx_flg;
extern volatile uint8_t test_uart1_intr_tx_flg;
extern volatile uint32_t status_intr_uart1;
extern volatile uint32_t status_intr_uart0;
extern conn_state_t conn_state;
extern uint8_t e2p_read_date;
extern e2p_date_t e2p_date;
extern e2p_sd_wr_cfg_t e2p_sd_wr_cfg;
extern gps_t gps;
extern ram_data_t ram_data;
extern gps_statuses_t gps_statuses;

extern char Fpath[30];

void init_peripherals(void)
{
  init_timer();

  esp_log_level_set(UART_TAG, ESP_LOG_INFO);
  initUart(UART_PC);
  
  init_modem();
  
  lis3dh_init_sensor(); // since i2c is initialized in init_gpio(), in order to initialize mcp23017 pins, we don't need to do this here for lis3dh init.
  
  init_sdcard();
  
  init_adc();
  
  rtc_setTime(00,00,00,01,01,2000,0);
  
  init_eeprom();
}

void update_startup_statuses()
{
  set_system_state(SYS_GSM_SIM, LOW);
  set_system_state(SYS_GPS_PASS, HIGH);
  set_system_state(SYS_GPS_DATE, HIGH);
  set_system_state(SYS_GPRS_DATE, HIGH);

  if((ram_data.Status)&(1<<SYS_SD_CARD))
  {
    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_BLUE|MCP_LED_GREEN), 1);
  }
  else if((ram_data.Status)&(1<<SYS_SD_CARD_INIT))
  {
    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_BLUE|MCP_LED_GREEN), 1);
  }
  else if(!e2p_read_date) //no need to check if rtc corrupted or not, at startup it is corrupted. so just check if e2p is corrupted
  {
    MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED|MCP_LED_BLUE), 0);
    MCP23017_pin_write(MCP_LED_GREEN_PORT,MCP_LED_RED, 1);
  }
}
void init_config(void)
{
  eepromSdWriteRate();
  e2p_read_date = eepromReadDate();
  eepromSdReadRate();
  update_startup_statuses();
}

extern "C" void app_main(void)
{
    init_gpio();
    init_peripherals();
    init_config();

#ifdef DEBUG_1SEC
    printf("\nSetupA=%d,SetupB=%d",SETUPA_testing, SETUPB_testing);
#endif  

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 10 milliseconds

    while(1)
    {
      checkforUARTFrameTimeout(GPRS_UART);

      if(schedular_flg.flg_10ms==true)
      {
        schedular_flg.flg_10ms=false;
        static int counter=0;
  
        Get_Curr_sdData(Fpath, 10);
        
        check_charger_status();
        
        if(test_uart0_intr_flg || test_uart1_intr_rx_flg || test_uart1_intr_tx_flg /* && UART0.status.txfifo_cnt == 0 */)
        {
            test_uart0_intr_flg = false;
            test_uart1_intr_rx_flg = false;
            test_uart1_intr_tx_flg = false;
        }
      }
      if(schedular_flg.flg_50ms==true)
      {
        schedular_flg.flg_50ms=false;
        manage_gps_gprs();
      }
      if(schedular_flg.flg_100ms==true)
      {
        schedular_flg.flg_100ms=false;
        check_card_detect();
        check_switch_pressed();
        control_RGB();
      }
      if(schedular_flg.flg_1sec==true)
      {
        schedular_flg.flg_1sec=false;
#ifdef DEBUG_1SEC
        UWriteString((char*)"\n1s", UART_PC);
#endif   
        update_ram_data();
        check_system_status();
        read_lis3dh();
      }
          
    }
}
