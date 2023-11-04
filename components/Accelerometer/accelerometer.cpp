#include "accelerometer.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
//#include "i2c_types.h"
#define LIS3DH_I2C_ADDR 0x18
void lis3dh_init()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x20, true);  // CTRL_REG1 register address
    i2c_master_write_byte(cmd, 0x57, true);  // Enable XYZ and set data rate
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

void lis3dh_read_acceleration() 
{
    int16_t x, y, z;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x28, true);  // OUT_X_L register address, multiple read
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, (uint8_t*)&x, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, (uint8_t*)&x + 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, (uint8_t*)&y, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, (uint8_t*)&y + 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, (uint8_t*)&z, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, (uint8_t*)&z + 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // The data is in two's complement, convert to signed integer
    x = (int16_t)x;
    y = (int16_t)y;
    z = (int16_t)z;

    // Apply conversion factor for ±2 g full scale (16 mg per LSB)
    float conversion_factor = 16.0 / 1000.0;  // Convert mg to g
    float accel_x = x * conversion_factor;
    float accel_y = y * conversion_factor;
    float accel_z = z * conversion_factor;

    printf("\nAcceleration (X, Y, Z): %.2f m/s², %.2f m/s², %.2f m/s²\n", accel_x, accel_y, accel_z);
}
