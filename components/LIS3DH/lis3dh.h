#ifndef __LIS3DH_H__
#define __LIS3DH_H__

// Uncomment one of the following defines to enable debug output
// #define LIS3DH_DEBUG_LEVEL_1    // only error messages
// #define LIS3DH_DEBUG_LEVEL_2    // debug and error messages

// LIS3DH addresses (also used for LIS2DH, LIS2DH12 and LIS2DE12)
#define LIS3DH_I2C_ADDRESS_1           0x18  // SDO pin is low
#define LIS3DH_I2C_ADDRESS_2           0x19  // SDO pin is high

// LIS3DE addresse (also used for LIS2DE)
#define LIS3DE_I2C_ADDRESS_1           0x28  // SDO pin is low
#define LIS3DE_I2C_ADDRESS_2           0x29  // SDO pin is high

// LIS3DH chip id
#define LIS3DH_CHIP_ID                 0x33  // LIS3DH_REG_WHO_AM_I<7:0>

// Definition of error codes
#define LIS3DH_OK                      0
#define LIS3DH_NOK                     -1

#define LIS3DH_INT_ERROR_MASK          0x000f
#define LIS3DH_DRV_ERROR_MASK          0xfff0

// Error codes for I2C and SPI interfaces ORed with LIS3DH driver error codes
#define LIS3DH_I2C_READ_FAILED         1
#define LIS3DH_I2C_WRITE_FAILED        2
#define LIS3DH_I2C_BUSY                3
#define LIS3DH_SPI_WRITE_FAILED        4
#define LIS3DH_SPI_READ_FAILED         5
#define LIS3DH_SPI_BUFFER_OVERFLOW     6

// LIS3DH driver error codes ORed with error codes for I2C and SPI interfaces
#define LIS3DH_WRONG_CHIP_ID              ( 1 << 8)
#define LIS3DH_WRONG_BANDWIDTH            ( 2 << 8)
#define LIS3DH_GET_RAW_DATA_FAILED        ( 3 << 8)
#define LIS3DH_GET_RAW_DATA_FIFO_FAILED   ( 4 << 8)
#define LIS3DH_WRONG_INT_TYPE             ( 5 << 8)
#define LIS3DH_CONFIG_INT_SIGNALS_FAILED  ( 6 << 8)
#define LIS3DH_CONFIG_INT_FAILED          ( 7 << 8)
#define LIS3DH_INT_SOURCE_FAILED          ( 8 << 8)
#define LIS3DH_CONFIG_HPF_FAILED          ( 9 << 8)
#define LIS3DH_ENABLE_HPF_FAILED          (10 << 8)
#define LIS3DH_CONFIG_CLICK_FAILED        (11 << 8)
#define LIS3DH_CLICK_SOURCE_FAILED        (12 << 8)
#define LIS3DH_GET_ADC_DATA_FAILED        (13 << 8)
#define LIS3DH_SENSOR_IN_BYPASS_MODE      (14 << 8)
#define LIS3DH_SENSOR_IN_FIFO_MODE        (15 << 8)
#define LIS3DH_ODR_TOO_HIGH               (16 << 8)

// register addresses
#define LIS3DH_REG_STATUS_AUX    0x07
#define LIS3DH_REG_OUT_ADC1_L    0x08
#define LIS3DH_REG_OUT_ADC1_H    0x09
#define LIS3DH_REG_OUT_ADC2_L    0x0a
#define LIS3DH_REG_OUT_ADC2_H    0x0b
#define LIS3DH_REG_OUT_ADC3_L    0x0c
#define LIS3DH_REG_OUT_ADC3_H    0x0d
#define LIS3DH_REG_INT_COUNTER   0x0e
#define LIS3DH_REG_WHO_AM_I      0x0f
#define LIS3DH_REG_TEMP_CFG      0x1f
#define LIS3DH_REG_CTRL1         0x20
#define LIS3DH_REG_CTRL2         0x21
#define LIS3DH_REG_CTRL3         0x22
#define LIS3DH_REG_CTRL4         0x23
#define LIS3DH_REG_CTRL5         0x24
#define LIS3DH_REG_CTRL6         0x25
#define LIS3DH_REG_REFERENCE     0x26
#define LIS3DH_REG_STATUS        0x27
#define LIS3DH_REG_OUT_X_L       0x28
#define LIS3DH_REG_OUT_X_H       0x29
#define LIS3DH_REG_OUT_Y_L       0x2a
#define LIS3DH_REG_OUT_Y_H       0x2b
#define LIS3DH_REG_OUT_Z_L       0x2c
#define LIS3DH_REG_OUT_Z_H       0x2d
#define LIS3DH_REG_FIFO_CTRL     0x2e
#define LIS3DH_REG_FIFO_SRC      0x2f
#define LIS3DH_REG_INT1_CFG      0x30
#define LIS3DH_REG_INT1_SRC      0x31
#define LIS3DH_REG_INT1_THS      0x32
#define LIS3DH_REG_INT1_DUR      0x33
#define LIS3DH_REG_INT2_CFG      0x34
#define LIS3DH_REG_INT2_SRC      0x35
#define LIS3DH_REG_INT2_THS      0x36
#define LIS3DH_REG_INT2_DUR      0x37
#define LIS3DH_REG_CLICK_CFG     0x38
#define LIS3DH_REG_CLICK_SRC     0x39
#define LIS3DH_REG_CLICK_THS     0x3a
#define LIS3DH_REG_TIME_LIMIT    0x3b
#define LIS3DH_REG_TIME_LATENCY  0x3c
#define LIS3DH_REG_TIME_WINDOW   0x3d

// register structure definitions
struct lis3dh_reg_status 
{
    uint8_t XDA   :1;      // STATUS<0>   X axis new data available
    uint8_t YDA   :1;      // STATUS<1>   Y axis new data available
    uint8_t ZDA   :1;      // STATUS<2>   Z axis new data available
    uint8_t ZYXDA :1;      // STATUS<3>   X, Y and Z axis new data available
    uint8_t XOR   :1;      // STATUS<4>   X axis data overrun
    uint8_t YOR   :1;      // STATUS<5>   Y axis data overrun 
    uint8_t ZOR   :1;      // STATUS<6>   Z axis data overrun
    uint8_t ZYXOR :1;      // STATUS<7>   X, Y and Z axis data overrun
};

#define LIS3DH_ANY_DATA_READY    0x0f    // LIS3DH_REG_STATUS<3:0>

struct lis3dh_reg_ctrl1 
{
    uint8_t Xen  :1;       // CTRL1<0>    X axis enable
    uint8_t Yen  :1;       // CTRL1<1>    Y axis enable
    uint8_t Zen  :1;       // CTRL1<2>    Z axis enable
    uint8_t LPen :1;       // CTRL1<3>    Low power mode enable
    uint8_t ODR  :4;       // CTRL1<7:4>  Data rate selection
};

struct lis3dh_reg_ctrl2 
{
    uint8_t HPIS1   :1;    // CTRL2<0>    HPF enabled for AOI on INT2
    uint8_t HPIS2   :1;    // CTRL2<1>    HPF enabled for AOI on INT2
    uint8_t HPCLICK :1;    // CTRL2<2>    HPF enabled for CLICK
    uint8_t FDS     :1;    // CTRL2<3>    Filter data selection
    uint8_t HPCF    :2;    // CTRL2<5:4>  HPF cutoff frequency
    uint8_t HPM     :2;    // CTRL2<7:6>  HPF mode
};

struct lis3dh_reg_ctrl3 
{
    uint8_t unused     :1; // CTRL3<0>  unused
    uint8_t I1_OVERRUN :1; // CTRL3<1>  FIFO Overrun interrupt on INT1
    uint8_t I1_WTM1    :1; // CTRL3<2>  FIFO Watermark interrupt on INT1
    uint8_t IT_DRDY2   :1; // CTRL3<3>  DRDY2 (ZYXDA) interrupt on INT1
    uint8_t IT_DRDY1   :1; // CTRL3<4>  DRDY1 (321DA) interrupt on INT1
    uint8_t I1_AOI2    :1; // CTRL3<5>  AOI2 interrupt on INT1
    uint8_t I1_AOI1    :1; // CTRL3<6>  AOI1 interrupt on INT1
    uint8_t I1_CLICK   :1; // CTRL3<7>  CLICK interrupt on INT1
};

struct lis3dh_reg_ctrl4 
{
    uint8_t SIM :1;        // CTRL4<0>   SPI serial interface selection
    uint8_t ST  :2;        // CTRL4<2:1> Self test enable
    uint8_t HR  :1;        // CTRL4<3>   High resolution output mode
    uint8_t FS  :2;        // CTRL4<5:4> Full scale selection
    uint8_t BLE :1;        // CTRL4<6>   Big/litle endian data selection
    uint8_t BDU :1;        // CTRL4<7>   Block data update
};

struct lis3dh_reg_ctrl5 
{
    uint8_t D4D_INT2 :1;   // CTRL5<0>   4D detection enabled on INT1
    uint8_t LIR_INT2 :1;   // CTRL5<1>   Latch interrupt request on INT1
    uint8_t D4D_INT1 :1;   // CTRL5<2>   4D detection enabled on INT2
    uint8_t LIR_INT1 :1;   // CTRL5<3>   Latch interrupt request on INT1
    uint8_t unused   :2;   // CTRL5<5:4> unused
    uint8_t FIFO_EN  :1;   // CTRL5<6>   FIFO enabled
    uint8_t BOOT     :1;   // CTRL5<7>   Reboot memory content
};

struct lis3dh_reg_ctrl6 
{
    uint8_t unused1  :1;   // CTRL6<0>   unused
    uint8_t H_LACTIVE:1;   // CTRL6<1>   Interrupt polarity
    uint8_t unused2  :1;   // CTRL6<2>   unused
    uint8_t I2_ACT   :1;   // CTRL6<3>   ?
    uint8_t I2_BOOT  :1;   // CTRL6<4>   ?
    uint8_t I2_AOI2  :1;   // CTRL6<5>   AOI2 interrupt on INT1
    uint8_t I2_AOI1  :1;   // CTRL6<6>   AOI1 interrupt on INT1
    uint8_t I2_CLICK :1;   // CTRL6<7>   CLICK interrupt on INT2
};

struct lis3dh_reg_fifo_ctrl
{
    uint8_t FTH :5;        // FIFO_CTRL<4:0>  FIFO threshold
    uint8_t TR  :1;        // FIFO_CTRL<5>    Trigger selection INT1 / INT2
    uint8_t FM  :2;        // FIFO_CTRL<7:6>  FIFO mode
};

struct lis3dh_reg_fifo_src
{
    uint8_t FFS       :5;  // FIFO_SRC<4:0>  FIFO samples stored
    uint8_t EMPTY     :1;  // FIFO_SRC<5>    FIFO is empty
    uint8_t OVRN_FIFO :1;  // FIFO_SRC<6>    FIFO buffer full
    uint8_t WTM       :1;  // FIFO_SRC<7>    FIFO content exceeds watermark
};

struct lis3dh_reg_intx_cfg
{
    uint8_t XLIE :1;   // INTx_CFG<0>    X axis below threshold enabled
    uint8_t XHIE :1;   // INTx_CFG<1>    X axis above threshold enabled
    uint8_t YLIE :1;   // INTx_CFG<2>    Y axis below threshold enabled
    uint8_t YHIE :1;   // INTx_CFG<3>    Y axis above threshold enabled
    uint8_t ZLIE :1;   // INTx_CFG<4>    Z axis below threshold enabled
    uint8_t ZHIE :1;   // INTx_CFG<5>    Z axis above threshold enabled
    uint8_t SIXD :1;   // INTx_CFG<6>    6D/4D orientation detecetion enabled
    uint8_t AOI  :1;   // INTx_CFG<7>    AND/OR combination of interrupt events
};

struct lis3dh_reg_intx_src
{
    uint8_t XL    :1;  // INTx_SRC<0>    X axis below threshold enabled
    uint8_t XH    :1;  // INTx_SRC<1>    X axis above threshold enabled
    uint8_t YL    :1;  // INTx_SRC<2>    Y axis below threshold enabled
    uint8_t YH    :1;  // INTx_SRC<3>    Y axis above threshold enabled
    uint8_t ZL    :1;  // INTx_SRC<4>    Z axis below threshold enabled
    uint8_t ZH    :1;  // INTx_SRC<5>    Z axis above threshold enabled
    uint8_t IA    :1;  // INTx_SRC<6>    Interrupt active
    uint8_t unused:1;  // INTx_SRC<7>    unused
};


struct lis3dh_reg_click_cfg
{
    uint8_t XS    :1;  // CLICK_CFG<0>    X axis single click enabled
    uint8_t XD    :1;  // CLICK_CFG<1>    X axis double click enabled
    uint8_t YS    :1;  // CLICK_CFG<2>    Y axis single click enabled
    uint8_t YD    :1;  // CLICK_CFG<3>    Y axis double click enabled
    uint8_t ZS    :1;  // CLICK_CFG<4>    Z axis single click enabled
    uint8_t ZD    :1;  // CLICK_CFG<5>    Z axis double click enabled
    uint8_t unused:2;  // CLICK_CFG<7:6>  unused
};

/**
 * @brief   Output data rates (ODR), related to resolution modes
 */
typedef enum {

    lis3dh_power_down = 0,  // power down mode
    lis3dh_odr_1,           // high resolution / normal / low power   1 Hz
    lis3dh_odr_10,          // high resolution / normal / low power  10 Hz
    lis3dh_odr_25,          // high resolution / normal / low power  25 Hz
    lis3dh_odr_50,          // high resolution / normal / low power  50 Hz
    lis3dh_odr_100,         // high resolution / normal / low power 100 Hz
    lis3dh_odr_200,         // high resolution / normal / low power 200 Hz
    lis3dh_odr_400,         // high resolution / normal / low power 400 Hz
    lis3dh_odr_1600,        // low power mode 1.6 kHz
    lis3dh_odr_5000,        // normal 1.25 kHz / low power 5 kHz

} lis3dh_odr_mode_t;

/**
 * @brief   Resolution modes, related to output data rates (ODR)
 */
typedef enum {

    lis3dh_low_power,       // low power mode resolution ( 8 bit data)
    lis3dh_normal,          // normal mode resolution    (10 bit data)
    lis3dh_high_res         // high resolution mode      (12 bit data)

} lis3dh_resolution_t;

/**
 * @brief   Full scale measurement range
 */
typedef enum {

    lis3dh_scale_2_g = 0,     // default
    lis3dh_scale_4_g,
    lis3dh_scale_8_g,
    lis3dh_scale_16_g

} lis3dh_scale_t;


/**
 * @brief   FIFO mode
 */
typedef enum {

    lis3dh_bypass = 0,     // default
    lis3dh_fifo   = 1,
    lis3dh_stream = 2,
    lis3dh_trigger= 3

} lis3dh_fifo_mode_t;


/**
 * @brief   Interrupt signals
 */
typedef enum {

    lis3dh_int1_signal = 0,
    lis3dh_int2_signal = 1    

} lis3dh_int_signal_t;
 
 
/**
 * @brief   Inertial event interrupt generators
 */
typedef enum {

    lis3dh_int_event1_gen = 0,
    lis3dh_int_event2_gen = 1    

} lis3dh_int_event_gen_t;


/**
 * @brief   Interrupt types for interrupt signals INT1/INT2
 */
typedef enum {

    lis3dh_int_data_ready,     // data ready for read interrupt (only INT1)

    lis3dh_int_fifo_watermark, // FIFO exceeds the threshold (only INT1)
    lis3dh_int_fifo_overrun,   // FIFO is completely filled (only INT1)
    
    lis3dh_int_event1,         // inertial event interrupt 1
    lis3dh_int_event2,         // inertial event interrupt 2

    lis3dh_int_click           // click detection interrupt
    
} lis3dh_int_type_t;


/**
 * @brief   Data ready and FIFO status interrupt source for INT1
 */
typedef struct {

    bool data_ready;      // true when acceleration data are ready to read

    bool fifo_watermark;  // true when FIFO exceeds the FIFO threshold
    bool fifo_overrun;    // true when FIFO is completely filled
    
} lis3dh_int_data_source_t;


/**
 * @brief   Inertial interrupt generator configuration for INT1/INT2
 *
 * Inertial events are: wake-up, free-fall, 6D/4D detection.
 */
typedef struct {

    enum {                    // interrupt mode

        lis3dh_wake_up,       // AOI = 0, 6D = 0
        lis3dh_free_fall,     // AOI = 1, 6D = 0

        lis3dh_6d_movement,   // AOI = 0, 6D = 1, D4D = 0
        lis3dh_6d_position,   // AOI = 1, 6D = 1, D4D = 0

        lis3dh_4d_movement,   // AOI = 0, 6D = 1, D4D = 1
        lis3dh_4d_position,   // AOI = 1, 6D = 1, D4D = 1
    
    } mode;            

    uint8_t  threshold;       // threshold used for comparison for all axes

    bool     x_low_enabled;   // x lower than threshold interrupt enabled
    bool     x_high_enabled;  // x higher than threshold interrupt enabled
    
    bool     y_low_enabled;   // y lower than threshold interrupt enabled
    bool     y_high_enabled;  // y higher than threshold interrupt enabled

    bool     z_low_enabled;   // z lower than threshold interrupt enabled
    bool     z_high_enabled;  // z higher than threshold interrupt enabled

    bool     latch;           // latch the interrupt when true until the
                              // interrupt source has been read
                              
    uint8_t  duration;        // duration in 1/ODR an interrupt condition has
                              // to be given before the interrupt is generated
} lis3dh_int_event_config_t;


/**
 * @brief   Inertial event source type for interrupt generator INT1/INT2 
 */
typedef struct {

    bool    active:1;     // true - one ore more events occured
    
    bool    x_low :1;     // true - x lower than threshold event
    bool    x_high:1;     // true - x higher than threshold event

    bool    y_low :1;     // true - z lower than threshold event
    bool    y_high:1;     // true - z higher than threshold event

    bool    z_low :1;     // true - z lower than threshold event
    bool    z_high:1;     // true - z higher than threshold event
    
} lis3dh_int_event_source_t;


/**
 * @brief   Click interrupt configuration for interrupt signals INT1/INT2 
 */
typedef struct {

    bool    x_single;       // x-axis single tap interrupt enabled
    bool    x_double;       // x-axis double tap interrupt enabled
    
    bool    y_single;       // y-axis single tap interrupt enabled
    bool    y_double;       // y-axis double tap interrupt enabled

    bool    z_single;       // z-axis single tap interrupt enabled
    bool    z_double;       // z-axis double tap interrupt enabled

    uint8_t  threshold;     // threshold used for comparison for all axes

    bool     latch;         // latch the interrupt when true until the
                            // interrupt source has been read
                          
    uint8_t  time_limit;    // maximum time interval between the start and the
                            // end of a cick (accel increases and falls back)
    uint8_t  time_latency;  // click detection is disabled for that time after 
                            // a was click detected (in 1/ODR)
    uint8_t  time_window;   // time interval in which the second click has to
                            // to be detected in double clicks (in 1/ODR)

} lis3dh_int_click_config_t;


/**
 * @brief   Click interrupt source for interrupt signals INT1/INT2 
 */
typedef struct {

    bool    x_click:1;    // click detected in x direction
    bool    y_click:1;    // click detected in y direction
    bool    z_click:1;    // click detected in z direction

    bool    sign   :1;    // click sign (0 - posisitive, 1 - negative)

    bool    s_click:1;    // single click detected
    bool    d_click:1;    // double click detected

    bool    active :1;    // true - one ore more event occured

} lis3dh_int_click_source_t;


/**
 * @brief   INT1, INT2 signal activity level
 */
typedef enum {

    lis3dh_high_active = 0,
    lis3dh_low_active

} lis3dh_int_signal_level_t;
    
    
/**
 * @brief   Raw data set as two complements
 */
typedef struct {

    int16_t ax; // acceleration on x axis
    int16_t ay; // acceleration on y axis
    int16_t az; // acceleration on z axis

} lis3dh_raw_data_t;


/**
 * @brief   Raw data FIFO type
 */
typedef lis3dh_raw_data_t lis3dh_raw_data_fifo_t[32];


/**
 * @brief   Floating point output value set in g
 */
typedef struct {

    float ax;   // acceleration on x axis
    float ay;   // acceleration on y axis
    float az;   // acceleration on z axis

} lis3dh_float_data_t;


/**
 * @brief   Floating point output value FIFO type
 */
typedef lis3dh_float_data_t lis3dh_float_data_fifo_t[32];


/**
 * @brief   HPF (high pass filter) modes
 */
typedef enum {

    lis3dh_hpf_normal = 0, // normal mode (reset by reading reference)
    lis3dh_hpf_reference,  // reference signal for filtering
    lis3dh_hpf_normal_x,   // normal mode
    lis3dh_hpf_autoreset   // autoreset on interrupt Activity

} lis3dh_hpf_mode_t;


/**
 * @brief   LIS3DH sensor device data structure type
 */
typedef struct {

    int       error_code;           // error code of last operation

    uint8_t   bus;                  // I2C = x, SPI = 1
    uint8_t   addr;                 // I2C = slave address, SPI = 0

    uint8_t   cs;                   // ESP8266, ESP32: GPIO used as SPI CS
                                    // __linux__: device index

    lis3dh_scale_t      scale;      // full range scale (default 2 g)
    lis3dh_resolution_t res;        // resolution used
    
    lis3dh_fifo_mode_t  fifo_mode;  // FIFO operation mode (default bypass)
    bool                fifo_first; // first FIFO access
      
} lis3dh_sensor_t;

/* static */ bool lis3dh_i2c_read(uint8_t reg, uint8_t *data, uint16_t len);

/* static */ bool lis3dh_i2c_write(uint8_t reg, uint8_t *data, uint16_t len);

void lis3dh_init_sensor(void);

/* static */ bool lis3dh_is_available(void);

bool lis3dh_reset(void);

bool lis3dh_config_hpf(lis3dh_hpf_mode_t mode, uint8_t cutoff, bool data, bool click, bool int1, bool int2);

bool lis3dh_set_hpf_ref(int8_t ref);

int8_t lis3dh_get_hpf_ref();

bool lis3dh_set_scale(lis3dh_scale_t scale);

bool lis3dh_set_mode(lis3dh_odr_mode_t odr, lis3dh_resolution_t res, bool x, bool y, bool z);

bool lis3dh_read_output(lis3dh_raw_data_t *data);

bool lis3dh_read_float(lis3dh_float_data_t *data);

void read_lis3dh();

uint32_t sdk_system_get_time();

void printBytes(unsigned char *str, int len);

#endif


