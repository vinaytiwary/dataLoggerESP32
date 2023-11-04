#ifndef __I2C_H__
#define __I2C_H__

#define ACK_VAL    0x0
#define NACK_VAL   0x1
#define ESP_SDA					15
#define ESP_SCL					2

void init_i2c_(void);
void i2c_send_byte(uint8_t slave_addr,uint8_t reg_addr, uint8_t data);
void i2c_read_byte(uint8_t slave_addr,uint8_t reg_addr, uint8_t* data);

int i2c_slave_write(uint8_t bus, uint8_t slave_addr, const uint8_t *reg_addr, uint8_t *data, uint32_t len);

int i2c_slave_read(uint8_t bus, uint8_t slave_addr, const uint8_t *reg_addr, uint8_t *data, uint32_t len);

#endif