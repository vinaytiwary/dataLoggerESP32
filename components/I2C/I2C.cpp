#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "soc/i2c_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "I2C.h"

void init_i2c_(void)
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

int i2c_slave_write (uint8_t bus, uint8_t slave_addr, const uint8_t *reg_addr, 
                     uint8_t *data, uint32_t len)
{
    //create a linked list of commands
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    //append the start cmd to the linked list
    i2c_master_start(cmd);

    //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);

    //append the specific register address you need to WRITE/READ to the linked list
    if (reg_addr)
        i2c_master_write_byte(cmd, *reg_addr, true);

    //append the data you need to WRITE on that register
    if (data)
        i2c_master_write(cmd, data, len, true);

    //append the stop cmd to the linked list
    i2c_master_stop(cmd);

    //execute the command linked list, and mention the maximum number of ticks it can wait
    esp_err_t err = i2c_master_cmd_begin((i2c_port_t)bus, cmd, 1000 / portTICK_PERIOD_MS);
    
    //free all the resources by deleting the linked list.
    i2c_cmd_link_delete(cmd);

    return err;
}

int i2c_slave_read (uint8_t bus, uint8_t slave_addr, const uint8_t *reg_addr, 
                    uint8_t *data, uint32_t len)
{
    if (len == 0) return true;

    //create a linked list of commands
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (reg_addr)
    {
        //append the start cmd to the linked list
        i2c_master_start(cmd);

        //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
        i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
        
        //append the specific register address you need to WRITE/READ to the linked list
        i2c_master_write_byte(cmd, *reg_addr, true);

        //append the stop cmd to the linked list
        if (!data)
            i2c_master_stop(cmd);
    }
    if (data)
    {
        //append the start cmd to the linked list
        i2c_master_start(cmd);

        //append the 7 bit slave address, shifted by 1 and ORed with  WRITE/READ cmd to the linked list, also enable ack
        i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, true);

        //append the cmd to read to the linked list and mention NACK to the slave to indicate we don't want to recieve more bytes after this.
        if (len > 1) i2c_master_read(cmd, data, len-1, (i2c_ack_type_t)ACK_VAL);
        i2c_master_read_byte(cmd, data + len-1, (i2c_ack_type_t)NACK_VAL);

        //append the stop cmd to the linked list
        i2c_master_stop(cmd);
    }

    //execute the command linked list, and mention the maximum number of ticks it can wait
    esp_err_t err = i2c_master_cmd_begin((i2c_port_t)bus, cmd, 1000 / portTICK_PERIOD_MS);
    
    //free all the resources by deleting the linked list.
    i2c_cmd_link_delete(cmd);

    return err;
}