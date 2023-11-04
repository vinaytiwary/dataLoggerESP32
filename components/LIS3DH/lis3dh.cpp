#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "esp_log.h"
#include "driver/i2c.h"
#include "soc/i2c_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

//#include "driver/gpio.h"

#include "I2C.h"
#include "lis3dh.h"
#include "_debug.h"

lis3dh_sensor_t lis3dh_sensor;

lis3dh_raw_data_t lis3dh_raw;
lis3dh_float_data_t lis3dh_float;

/**
 * Scaling factors for the conversion of raw sensor data to floating point g
 * values. Scaling factors are from mechanical characteristics in datasheet.
 *
 *  scale/sensitivity  resolution
 *       +-1g           1 mg/digit
 *       +-2g           2 mg/digit
 *       +-4g           4 mg/digit
 *      +-16g          12 mg/digit
 */
/* const */ static double  LIS3DH_SCALES[4] = { 0.001, 0.002, 0.004, 0.012 };

#if defined(LIS3DH_DEBUG_LEVEL_2)
#define debug(s, f, ...) printf("%s %s: " s "\n", "LIS3DH", f, ## __VA_ARGS__)
//#define debug_dev(s, f, d, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "LIS3DH", f, d->bus, d->addr, ## __VA_ARGS__)
#define debug_dev(s, f, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "LIS3DH", f, I2C_NUM_0, LIS3DH_I2C_ADDRESS_1, ## __VA_ARGS__)
#else
#define debug(s, f, ...)
#define debug_dev(s, f, ...)
#endif

#if defined(LIS3DH_DEBUG_LEVEL_1) || defined(LIS3DH_DEBUG_LEVEL_2)
#define error(s, f, ...) printf("%s %s: " s "\n", "LIS3DH", f, ## __VA_ARGS__)
//#define error_dev(s, f, d, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "LIS3DH", f, d->bus, d->addr, ## __VA_ARGS__)
#define error_dev(s, f, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "LIS3DH", f, I2C_NUM_0, LIS3DH_I2C_ADDRESS_1, ## __VA_ARGS__)
#else
#define error(s, f, ...)
#define error_dev(s, f, ...)
#endif

/* #define lis3dh_update_reg(addr,type,elem,value) \
        { \
            struct type __reg; \
            if (!lis3dh_i2c_read ((addr), (uint8_t*)&__reg, 1)) \
                return NULL; \
            __reg.elem = (value); \
            if (!lis3dh_i2c_write ((addr), (uint8_t*)&__reg, 1)) \
                return NULL; \
        } */
#define lis3dh_update_reg(addr,type,elem,value) \
        { \
            struct type __reg; \
            lis3dh_i2c_read ((addr), (uint8_t*)&__reg, 1);\
            __reg.elem = (value); \
            lis3dh_i2c_write ((addr), (uint8_t*)&__reg, 1);\
        }

#define I2C_AUTO_INCREMENT (0x80)

/* static */ bool lis3dh_i2c_read(uint8_t reg, uint8_t *data, uint16_t len)
{
    //if (!dev || !data) return false;
    if (!data) return false;

    //will figure out how this dbg works later
    debug_dev ("Read %d byte from i2c slave register %02x.", __FUNCTION__, len, reg);

    if (len > 1)
        reg |= I2C_AUTO_INCREMENT;
    
    //int result = i2c_slave_read(dev->bus, dev->addr, &reg, data, len);
    int result = i2c_slave_read((uint8_t)I2C_NUM_0, (uint8_t)LIS3DH_I2C_ADDRESS_1, &reg, data, len);

    if (result)
    {
        //add a dbg here.

        // dev->error_code |= (result == -EBUSY) ? LIS3DH_I2C_BUSY : LIS3DH_I2C_READ_FAILED;
        error_dev ("Error %d on read %d byte from I2C slave register %02x.",
                    __FUNCTION__, result, len, reg);
        return false;
    }

#ifdef LIS3DH_DEBUG_LEVEL_2
    printf("LIS3DH %s: Read following bytes: ", __FUNCTION__);
    printf("%02x: ", reg & 0x7f);
    for (int i=0; i < len; i++)
        printf("%02x ", data[i]);
    printf("\n");
#endif

    return true;
}


/* static */ bool lis3dh_i2c_write(uint8_t reg, uint8_t *data, uint16_t len)
{
    //if (!dev || !data) return false;
    if (!data) return false;

    //will figure out how this dbg works later
    debug_dev ("Write %d byte to i2c slave register %02x.", __FUNCTION__, len, reg);

    if (len > 1)
        reg |= I2C_AUTO_INCREMENT;

    //int result = i2c_slave_write(dev->bus, dev->addr, &reg, data, len);
    int result = i2c_slave_write((uint8_t)I2C_NUM_0, (uint8_t)LIS3DH_I2C_ADDRESS_1, &reg, data, len);

    if (result)
    {
        //add a dbg here.

        // dev->error_code |= (result == -EBUSY) ? LIS3DH_I2C_BUSY : LIS3DH_I2C_WRITE_FAILED;
        error_dev ("Error %d on write %d byte to i2c slave register %02x.",
                    __FUNCTION__, result, len, reg);
        return false;
    }

#ifdef LIS3DH_DEBUG_LEVEL_2
    printf("LIS3DH %s: Wrote the following bytes: ", __FUNCTION__);
    printf("%02x: ", reg & 0x7f);
    for (int i=0; i < len; i++)
        printf("%02x ", data[i]);
    printf("\n");
#endif

    return true;
}

void lis3dh_init_sensor(void)
{
    memset(&lis3dh_sensor, 0, sizeof(lis3dh_sensor_t));

    lis3dh_sensor.bus = I2C_NUM_0;
    lis3dh_sensor.addr = LIS3DH_I2C_ADDRESS_1;
    lis3dh_sensor.error_code = LIS3DH_OK;
    lis3dh_sensor.scale      = lis3dh_scale_2_g;
    lis3dh_sensor.fifo_mode  = lis3dh_bypass;
    lis3dh_sensor.fifo_first = true;


    // check availability of the sensor
    if (!lis3dh_is_available ())
    {
        error_dev ("Sensor is not available.", __FUNCTION__);
        return;
    }

    // reset the sensor
    if (!lis3dh_reset())
    {
        error_dev ("Could not reset the sensor device.", __FUNCTION__);
        return;
    }
    
    // enable high res mode, disable low power mode
    lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, HR, 1);
    lis3dh_update_reg (LIS3DH_REG_CTRL1, lis3dh_reg_ctrl1, LPen, 0);

    // configure HPF and reset the reference by dummy read
    lis3dh_config_hpf (lis3dh_hpf_normal, 0, true, true, true, true);
    lis3dh_get_hpf_ref ();

    // set fullscale range: 2, 4, 8 or 16 G!
    lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, FS, lis3dh_scale_2_g);

    // enable block data update
    lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, BDU, 1);

    // enable ble
    //lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, BLE, 1);

    // LAST STEP: Finally set scale and mode to start measurements
    lis3dh_set_scale(lis3dh_scale_2_g);
    lis3dh_set_mode (lis3dh_odr_50, lis3dh_high_res, true, true, true);
}

/**
 * @brief   Check the chip ID to test whether sensor is available
 */
/* static */ bool lis3dh_is_available(void)
{
    uint8_t chip_id;

    if(!lis3dh_i2c_read(LIS3DH_REG_WHO_AM_I, &chip_id, 1))
        return false;

    if(chip_id != LIS3DH_CHIP_ID)
    {
        //add a dbg here.
        error_dev ("Chip id %02x is wrong, should be %02x.",
                    __FUNCTION__, chip_id, LIS3DH_CHIP_ID);
        return false;
    }

    return true;
}

/* static */ bool lis3dh_reset ()
{
    // if (!dev) return false;

    // dev->error_code = LIS3DH_OK;

    uint8_t reg[8] = { 0 };
    
    // initialize sensor completely including setting in power down mode
    lis3dh_i2c_write (LIS3DH_REG_TEMP_CFG , reg, 8);
    lis3dh_i2c_write (LIS3DH_REG_FIFO_CTRL, reg, 1);
    lis3dh_i2c_write (LIS3DH_REG_INT1_CFG , reg, 1);
    lis3dh_i2c_write (LIS3DH_REG_INT1_THS , reg, 2);
    lis3dh_i2c_write (LIS3DH_REG_INT2_CFG , reg, 1);
    lis3dh_i2c_write (LIS3DH_REG_INT2_THS , reg, 2);
    lis3dh_i2c_write (LIS3DH_REG_CLICK_CFG, reg, 1);
    lis3dh_i2c_write (LIS3DH_REG_CLICK_THS, reg, 4);
    
    return true;
}

bool lis3dh_config_hpf (lis3dh_hpf_mode_t mode, uint8_t cutoff,
                        bool data, bool click, bool int1, bool int2)
{
    //if (!dev) return false;

    //dev->error_code = LIS3DH_OK;

    struct lis3dh_reg_ctrl2 reg;
    
    reg.HPM  = mode;
    reg.HPCF = cutoff;
    reg.FDS  = data;
    reg.HPCLICK = click;
    reg.HPIS1   = int1;
    reg.HPIS2   = int2;
    
    if (!lis3dh_i2c_write(LIS3DH_REG_CTRL2, (uint8_t*)&reg, 1))
    {   
        error_dev ("Could not configure high pass filter", __FUNCTION__);
        //dev->error_code |= LIS3DH_CONFIG_HPF_FAILED;
        return false;
    }

    return true;
}

bool lis3dh_set_hpf_ref(int8_t ref)
{
    //if (!dev) return false;

    //dev->error_code = LIS3DH_OK;

    if (!lis3dh_i2c_write (LIS3DH_REG_REFERENCE, (uint8_t*)&ref, 1))
    {   
        error_dev ("Could not set high pass filter reference", __FUNCTION__);
        //dev->error_code |= LIS3DH_CONFIG_HPF_FAILED;
        return false;
    }

    return true;
}

int8_t lis3dh_get_hpf_ref()
{
    //if (!dev) return 0;

    //dev->error_code = LIS3DH_OK;

    int8_t ref;
    
    if (!lis3dh_i2c_read (LIS3DH_REG_REFERENCE, (uint8_t*)&ref, 1))
    {   
        error_dev ("Could not get high pass filter reference", __FUNCTION__);
        //dev->error_code |= LIS3DH_CONFIG_HPF_FAILED;
        return 0;
    }

    return ref;
}

bool lis3dh_set_scale (lis3dh_scale_t scale)
{
    // if (!dev) return false;
    
    // dev->error_code = LIS3DH_OK;
    // dev->scale = scale;
    
    // read CTRL4 register and write scale
    lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, FS, scale);
    
    return true;
}

bool lis3dh_set_mode (lis3dh_odr_mode_t odr, lis3dh_resolution_t res,
                      bool x, bool y, bool z)
{
    // if (!dev) return false;

    // dev->error_code = LIS3DH_OK;
    // dev->res = res;

    struct lis3dh_reg_ctrl1 reg;
    uint8_t old_odr;

    // read current register values
    if (!lis3dh_i2c_read (LIS3DH_REG_CTRL1, (uint8_t*)&reg, 1))
        return false;
   
    old_odr = reg.ODR;
    
    // set mode
    reg.Xen  = x;
    reg.Yen  = y;
    reg.Zen  = z;
    reg.ODR  = odr;
    reg.LPen = (res == lis3dh_low_power);

    lis3dh_update_reg (LIS3DH_REG_CTRL4, lis3dh_reg_ctrl4, 
                       HR, (res == lis3dh_high_res));
    
    if (!lis3dh_i2c_write (LIS3DH_REG_CTRL1, (uint8_t*)&reg, 1))
        return false;
    
    // if sensor was in power down mode it takes at least 100 ms to start in another mode
    if (old_odr == lis3dh_power_down && odr != lis3dh_power_down)
        vTaskDelay (15);

    return true;
}

bool lis3dh_new_data ()
{
    // if (!dev) return false;

    // dev->error_code = LIS3DH_OK;

    if (lis3dh_sensor.fifo_mode == lis3dh_bypass)
    {
        struct lis3dh_reg_status status;
        
        if (!lis3dh_i2c_read (LIS3DH_REG_STATUS, (uint8_t*)&status, 1))
        {
            error_dev ("Could not get sensor status", __FUNCTION__);
            return false;
        }
        return status.ZYXDA;
    }
    else
    {
        struct lis3dh_reg_fifo_src fifo_src;
        
        if (!lis3dh_i2c_read (LIS3DH_REG_FIFO_SRC, (uint8_t*)&fifo_src, 1))
        {
            error_dev ("Could not get fifo source register data", __FUNCTION__);
            return false;
        }
        return !fifo_src.EMPTY;
    }
}


bool lis3dh_read_output(lis3dh_raw_data_t* data)
{
    // abort if not in bypass mode
    if (lis3dh_sensor.fifo_mode != lis3dh_bypass)
    {
        //dev->error_code = LIS3DH_SENSOR_IN_BYPASS_MODE;
        error_dev ("Sensor is in FIFO mode, use lis3dh_get_*_data_fifo to get data",
                   __FUNCTION__);
        return false;
    }

    // read raw data sample
    if (!lis3dh_i2c_read (LIS3DH_REG_OUT_X_L, (uint8_t*)data, 6))
    {
        error_dev ("Could not get raw data sample", __FUNCTION__);
        //dev->error_code |= LIS3DH_GET_RAW_DATA_FAILED;
       return false;
    }

    //printf("\nraw=");
    printBytes((unsigned char*)&data, sizeof(lis3dh_raw_data_t));

    return true;
}

bool lis3dh_read_float(lis3dh_float_data_t* data)
{

    //if (!dev || !data) return false;

    lis3dh_raw_data_t raw;
    
    if (!lis3dh_read_output (&raw))
        return false;

    data->ax = LIS3DH_SCALES[lis3dh_sensor.scale] * (raw.ax >> 4);
    data->ay = LIS3DH_SCALES[lis3dh_sensor.scale] * (raw.ay >> 4);
    data->az = LIS3DH_SCALES[lis3dh_sensor.scale] * (raw.az >> 4);

    printf("0x%02X,0x%02X,0x%02X,", (raw.ax >> 4),(raw.ay >> 4),(raw.az >> 4));

    return true;
}

void read_lis3dh()
{
    static uint32_t lis_strt_T = 0;
    lis3dh_float_data_t  data;

    lis_strt_T = (esp_timer_get_time()/1000);

    /* if (lis3dh_new_data () && lis3dh_read_float (&data))
    {
        // max. full scale is +-16 g and best resolution is 1 mg, i.e. 5 digits
        // printf("%.3f LIS3DH (xyz)[g] ax=%+7.3f ay=%+7.3f az=%+7.3f\n",
        //         (double)sdk_system_get_time()*1e-3, 
        //         data.ax, data.ay, data.az);

        printf("%+7.3f,%+7.3f,%+7.3f,", data.ax, data.az, data.ax);
    } */
    if(lis3dh_new_data())
    {
        uint8_t data[6];
        int16_t x = 0, y = 0, z = 0;
        float output = 1; 

        memset(data, 0, 6);
        lis3dh_i2c_read (LIS3DH_REG_OUT_X_L, /* (uint8_t*) */data, 6);
        // printf("\n");
        // for(int i = 0; i < 6; i++)
        // {
        //   printf("0x%02X,", data[i]);
        // }

        // x = (((int16_t)data[1] << 8) + ((int16_t)data[0])) >> 4; //doing so will lose the negative value as >>4 operation takes precedence
        // y = (((int16_t)data[3] << 8) + ((int16_t)data[2])) >> 4;
        // z = (((int16_t)data[5] << 8) + ((int16_t)data[4])) >> 4;

        x = (((int16_t)data[1] << 8) + ((int16_t)data[0]));
        y = (((int16_t)data[3] << 8) + ((int16_t)data[2]));
        z = (((int16_t)data[5] << 8) + ((int16_t)data[4]));

        x >>= 4;
        y >>= 4;
        z >>= 4;

        // x = (x & 0x800)? (~x + 1) * -1 : x;  //you only need to do this if your variable is unsigned
        // y = (y & 0x800)? (~y + 1) * -1 : y;
        // z = (z & 0x800)? (~z + 1) * -1 : y;

        //printf("0x%02X,0x%02X,0x%02X,", x,y,z);

        //printf("%+7.3f,%+7.3f,%+7.3f,", (double)x * (0.9765F), (double)y * (0.9765F), (double)z * (0.9765F));
        //printf("%+7.3f,%+7.3f,%+7.3f,", x * (0.001F), y * (0.001F), z * (0.001F));
        //printf("%f,%f,%f,", x * (9.81*0.000976), y * (9.81*0.000976), z * (9.81*0.000976));
#ifdef DEBUG_LIS3DH_READ
        printf("\nXYZ = %+03.6f, %+03.6f, %+03.6f\n",x * (9.81*0.000976), y * (9.81*0.000976), z * (9.81*0.000976));
        //printf("\nLt=%lld", (esp_timer_get_time()/1000) - lis_strt_T);
#endif
    }
}

// esp-open-rtos SDK function wrapper

uint32_t sdk_system_get_time ()
{
    struct timeval time;
    gettimeofday(&time,0);
    return time.tv_sec*1e6 + time.tv_usec;
}

void printBytes(unsigned char* str, int len)
{
	while(len)
	{
		printf("0x%02X,", *str);
//		printf("\nstr = %c", *str);
//		printf("\nlen = %d", len);
		str++;
		len--;
	}
}