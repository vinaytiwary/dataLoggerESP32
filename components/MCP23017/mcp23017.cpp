#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "soc/i2c_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcp23017.h"
#include "I2C.h"
#include "lis3dh.h"

MCP23017_REG_T MCP23017_REG;

/* void app_main(void)
{
    init_i2c();
    memset(&MCP23017_REG,0,sizeof(MCP23017_REG_T));

    // MCP23017_REG.GPA_DIR = 0xFF;    //lets set all pins to input
    // MCP23017_REG.GPA_PORT = 0x00;
    // MCP23017_REG.GPB_DIR = 0xFF;
    // MCP23017_REG.GPB_PORT = 0x00;

    MCP23017_set_dir(IODIRB, GPB4, 0);
    MCP23017_pin_write(GPB_ADDR, GPB4, 0);

    MCP23017_set_dir(IODIRB, GPB3, 0);
    MCP23017_pin_write(GPB_ADDR, GPB3, 1);

    MCP23017_set_dir(IODIRB, GPB2, 0);
    MCP23017_pin_write(GPB_ADDR, GPB2, 0);

    
    
    vTaskDelay(100);
    while(1)
    {
        MCP23017_pin_toggle(GPB_ADDR, GPB3);
        //MCP23017_pin_write(GPB_ADDR, GPB3, 0);
        vTaskDelay(100);
    }

} */
#if 0
void init_i2c(void)
{
    i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = ESP_SDA;
	conf.scl_io_num = ESP_SCL;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
    conf.clk_flags = 0;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}
#endif
#if 0
void i2c_send_byte(uint8_t slave_addr,uint8_t reg_addr, uint8_t data)
{
    //create a linked list of commands
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    //append the start cmd to the linked list
    i2c_master_start(cmd);

    //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);

    //append the specific register address you need to WRITE/READ to the linked list
    i2c_master_write_byte(cmd, reg_addr, true);

    //append the byte you need to WRITE on that register
    i2c_master_write_byte(cmd, data, true);

    //append the stop cmd to the linked list
    i2c_master_stop(cmd);

    //execute the command linked list, and mention the maximum number of ticks it can wait
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);

    //free all the resources by deleting the linked list.
    i2c_cmd_link_delete(cmd);
}

void i2c_read_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t * data)
{
    //create a linked list of commands
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    //append the start cmd to the linked list
    i2c_master_start(cmd);

    //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);

    //append the specific register address you need to WRITE/READ to the linked list
    i2c_master_write_byte(cmd, reg_addr, true);
 
    //append the stop cmd to the linked list
    i2c_master_stop(cmd);

    //execute the command linked list, and mention the maximum number of ticks it can wait
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);

    //free all the resources by deleting the linked list.
    i2c_cmd_link_delete(cmd);

    //create a linked list of commands
    /* i2c_cmd_handle_t  */cmd = i2c_cmd_link_create();

    //append the start cmd to the linked list
    i2c_master_start(cmd);

    //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, true);

    //append the cmd to read to the linked list and mention NACK to the slave to indicate we don't want to recieve more bytes after this.
    i2c_master_read_byte(cmd, data, (i2c_ack_type_t)NACK_VAL);

    //append the stop cmd to the linked list
    i2c_master_stop(cmd);

    //execute the command linked list, and mention the maximum number of ticks it can wait
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);

    //free all the resources by deleting the linked list.
    i2c_cmd_link_delete(cmd);
}
#endif
/* 
void toggle_led(void)
{
    static uint8_t state = 0;
    switch(state)
    {
        case 0:
            i2c_send_byte(MCP23017_ADDR_BASE,LED_PORT,~STATUS_LED_PIN);
            //set_RGB_led(RGB_WHITE);
            state = 1;
        break;

        case 1:
            i2c_send_byte(MCP23017_ADDR_BASE,LED_PORT,STATUS_LED_PIN);
            //set_RGB_led(RGB_OFF);
            state = 0;
        break;
    }
}
 */

void MCP23017_set_dir(uint8_t port_no, uint8_t pins, uint8_t dir)
{
#ifdef DBG_MCP 
    printf("\ndir");
#endif
    switch(port_no)
    {
        case IODIRA:
        {
#ifdef DBG_MCP
            printf("A:\t");
#endif

            if(dir)
            {
                MCP23017_REG.GPA_DIR |= pins;
#ifdef DBG_MCP
                printf("in");
#endif

            }
            else
            {
                MCP23017_REG.GPA_DIR &= ~pins;
#ifdef DBG_MCP
                printf("out");
#endif
            }
            i2c_send_byte(MCP23017_ADDR_BASE,IODIRA,MCP23017_REG.GPA_DIR);
        }
        break;

        case IODIRB:
        {
#ifdef DBG_MCP
            printf("B:\t");
#endif
            if(dir)
            {
                MCP23017_REG.GPB_DIR |= pins;
#ifdef DBG_MCP
                printf("in");
#endif
            }
            else
            {
                MCP23017_REG.GPB_DIR &= ~pins;
#ifdef DBG_MCP
                printf("out");
#endif
            }
            i2c_send_byte(MCP23017_ADDR_BASE,IODIRB,MCP23017_REG.GPB_DIR);
        }
        break;

    }

}

void MCP23017_pin_write(uint8_t port_no, uint8_t pins, uint8_t state)
{
#ifdef DBG_MCP
    printf("\npin");
#endif
    switch(port_no)
    {
        case GPA_ADDR:
        {
#ifdef DBG_MCP
            printf("A:\t");
#endif
            if(state)
            {
                MCP23017_REG.GPA_PORT |= pins;
#ifdef DBG_MCP
                printf("Hi");
#endif
            }
            else
            {
                MCP23017_REG.GPA_PORT &= ~pins;
#ifdef DBG_MCP
                printf("Lo");
#endif
            }
            i2c_send_byte(MCP23017_ADDR_BASE,GPA_ADDR,MCP23017_REG.GPA_PORT);
        }
        break;

        case GPB_ADDR:
        {
#ifdef DBG_MCP
            printf("B:\t");
#endif
            if(state)
            {
                MCP23017_REG.GPB_PORT |= pins;
#ifdef DBG_MCP
                printf("Hi");
#endif
            }
            else
            {
                MCP23017_REG.GPB_PORT &= ~pins;
#ifdef DBG_MCP
                printf("Lo");
#endif
            }
            i2c_send_byte(MCP23017_ADDR_BASE,GPB_ADDR,MCP23017_REG.GPB_PORT);
        }
        break;
    }

}

void MCP23017_pin_toggle(uint8_t port_no, uint8_t pins)
{
    switch(port_no)
    {
        case GPA_ADDR:
        {
            if(!(MCP23017_REG.GPA_PORT & pins))
            {
                MCP23017_REG.GPA_PORT |= pins;

            }
            else
            {
                MCP23017_REG.GPA_PORT &= ~pins;
            }
            i2c_send_byte(MCP23017_ADDR_BASE,GPA_ADDR,MCP23017_REG.GPA_PORT);
        }
        break;

        case GPB_ADDR:
        {
            if(!(MCP23017_REG.GPB_PORT & pins))
            {
                MCP23017_REG.GPB_PORT |= pins;
            }
            else
            {
                MCP23017_REG.GPB_PORT &= ~pins;
            }
            //i2c_send_byte(MCP23017_ADDR_BASE,GPB_ADDR,MCP23017_REG.GPB_DIR);
            i2c_send_byte(MCP23017_ADDR_BASE,GPB_ADDR,MCP23017_REG.GPB_PORT);
        }
        break;

    }
}
void init_mcp23017(void)
{
    //init_i2c();
    memset(&MCP23017_REG,0,sizeof(MCP23017_REG_T));
/* 
    MCP23017_REG.GPA_DIR = 0xFF;    //lets set all pins to input
    MCP23017_REG.GPA_PORT = 0x00;
    MCP23017_REG.GPB_DIR = 0xFF;
    MCP23017_REG.GPB_PORT = 0x00;
 */
    // MCP23017_set_dir(IODIRB, GPB4, 0);
    // MCP23017_pin_write(GPB_ADDR, GPB4, 0);

    // MCP23017_set_dir(IODIRB, GPB3, 0);
    // MCP23017_pin_write(GPB_ADDR, GPB3, 1);

    // MCP23017_set_dir(IODIRB, GPB2, 0);
    // MCP23017_pin_write(GPB_ADDR, GPB2, 0);    
}
#if 0
static esp_err_t read_accel_data(i2c_port_t i2c_num, uint8_t reg, uint8_t* data, size_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DHTR_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DHTR_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void read_data()
{
    uint8_t reg_data[6]; // The LIS3DHTR provides 6 bytes of data (X, Y, Z axes)
    esp_err_t ret = read_accel_data(I2C_NUM_0, 0x29, reg_data, sizeof(reg_data)); // Read from the OUT_X_L register
    if (ret == ESP_OK) 
    {
        int16_t x = (int16_t)((reg_data[1] << 8) | reg_data[0]);
        int16_t y = (int16_t)((reg_data[3] << 8) | reg_data[2]);
        int16_t z = (int16_t)((reg_data[5] << 8) | reg_data[4]);
        
        printf("\nX-Axis: %d", x);
        printf("\nY-Axis: %d", y);
        printf("\nZ-Axis: %d", z);
    } 
    else 
    {
        printf("\nCould not read data from the accelerometer");
    }
}

#endif