#ifndef MCP23017_TEST3_H_
#define MCP23017_TEST3_H_

#define ACK_VAL    0x0
#define NACK_VAL   0x1
#define MCP23017_ADDR_BASE      0x20
#define ESP_SDA					15
#define ESP_SCL					2

#define IODIRA  (0x00)
#define IODIRB  (0x01)
#define GPA_ADDR    (0x12)
#define GPA0    (1<<0)
#define GPA1    (1<<1)
#define GPA2    (1<<2)
#define GPA3    (1<<3)
#define GPA4    (1<<4)
#define GPA5    (1<<5)
#define GPA6    (1<<6)
#define GPA7    (1<<7)

#define GPB_ADDR    (0x13)
#define GPB0    (1<<0)
#define GPB1    (1<<1)
#define GPB2    (1<<2)
#define GPB3    (1<<3)
#define GPB4    (1<<4)
#define GPB5    (1<<5)
#define GPB6    (1<<6)
#define GPB7    (1<<7)

typedef struct
{
    uint8_t GPA_DIR;
    uint8_t GPA_PORT;
    uint8_t GPB_DIR;
    uint8_t GPB_PORT;
}MCP23017_REG_T;

//void init_i2c(void);
//void i2c_send_byte(uint8_t slave_addr,uint8_t reg_addr, uint8_t data);
//void i2c_read_byte(uint8_t slave_addr,uint8_t reg_addr, uint8_t* data);
//void toggle_led(void);
void MCP23017_set_dir(uint8_t port_no, uint8_t pins, uint8_t dir);
void MCP23017_pin_write(uint8_t port_no, uint8_t pins, uint8_t state);
void MCP23017_pin_toggle(uint8_t port_no, uint8_t pins);

void init_mcp23017(void);

//void read_data();

#endif /* MCP23017_TEST3_H_ */