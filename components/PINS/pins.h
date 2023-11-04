#ifndef _PINS_H
#define _PINS_H_

#include "mcp23017.h"

/***************************************************************ESP_PINS********************************************************************************/
#define ESP_DGB_TX				1
#define ESP_DGB_RX				3

#define ESP_SIMCOM_TX			13	
#define ESP_SIMCOM_RX			4

#define ESP_RS485_TX			21
#define ESP_RS485_RX			22
#define ESP_RS232_TX			12
#define ESP_RS232_RX			27

#define	ESP_MISO				19
#define	ESP_MOSI				23
#define	ESP_SCK					18
#define	ESP_SD_CS				5
#define	ESP_NRF_CS				32
#define	ESP_NRF_CE				33

#define ESP_SWITCH				0
#define ESP_PWREN				14
#define ESP_SDA					15
#define ESP_SCL					2

#define ESP_ADC_BATTIN			36
#define ESP_ADC_ACC				39
#define ESP_ADC_LVL				34

#define ESP_ACCL_INT			35

#define ESP_CAN_TX				25
#define ESP_CAN_RX				26

#define PIN_NUM_MISO            19
#define PIN_NUM_MOSI            23
#define PIN_NUM_CLK             18
#define PIN_NUM_CS              GPIO_NUM_5

/***************************************************************ESP_PINS********************************************************************************/
/***************************************************************MCP_PINS********************************************************************************/

#define MCP_PORTA               GPA_ADDR
#define MCP_PORTB               GPB_ADDR

#define MCP_PWR_LTCH			GPA0
#define MCP_RS485_DIR			GPA1
#define MCP_IO_IN				GPA2
#define MCP_IO_OUT				GPA3
#define MCP_CAN_SO				GPA4
#define MCP_TP_CHARGING		    GPA5
#define MCP_TP_CH_EN			GPA6
#define MCP_FREE				GPA7

#define MCP_SIM_PWREN	        GPB0
#define MCP_SIM_DTR		        GPB1
#define MCP_LED_GREEN	        GPB3
#define MCP_LED_BLUE	        GPB2
#define MCP_LED_RED		        GPB4
#define MCP_SIM_1V8		        GPB5
#define MCP_SIM_ONOFF	        GPB6
#define MCP_SIM_RST		        GPB7

#define MCP_PWR_LTCH_DDR        IODIRA
#define MCP_PWR_LTCH_PORT       MCP_PORTA

#define MCP_RS485_DDR           IODIRA
#define MCP_RS485_PORT          MCP_PORTA

#define MCP_IO_IN_DDR           IODIRA
#define MCP_IO_IN_PORT          MCP_PORTA

#define MCP_IO_OUT_DDR          IODIRA
#define MCP_IO_OUT_PORT         MCP_PORTA

#define MCP_CAN_SO_DDR          IODIRA
#define MCP_CAN_SO_PORT         MCP_PORTA

#define MCP_TP_CHARGING_DDR     IODIRA
#define MCP_TP_CHARGING_PORT    MCP_PORTA

#define MCP_TP_CH_EN_DDR        IODIRA
#define MCP_TP_CH_EN_PORT       MCP_PORTA

#define MCP_SIM_PWREN_DDR       IODIRB
#define MCP_SIM_PWREN_PORT      MCP_PORTB

#define MCP_SIM_DTR_DDR         IODIRB
#define MCP_SIM_DTR_PORT        MCP_PORTB

#define MCP_SIM_1V8_DDR         IODIRB
#define MCP_SIM_1V8_PORT        MCP_PORTB

#define MCP_SIM_ONOFF_DDR       IODIRB
#define MCP_SIM_ONOFF_PORT      MCP_PORTB

#define MCP_SIM_RST_DDR         IODIRB
#define MCP_SIM_RST_PORT        MCP_PORTB

#define MCP_LED_GREEN_DDR       IODIRB
#define MCP_LED_GREEN_PORT      MCP_PORTB

#define MCP_LED_BLUE_DDR        IODIRB
#define MCP_LED_BLUE_PORT       MCP_PORTB

#define MCP_LED_RED_DDR         IODIRB
#define MCP_LED_RED_PORT        MCP_PORTB

/***************************************************************MCP_PINS********************************************************************************/

#endif