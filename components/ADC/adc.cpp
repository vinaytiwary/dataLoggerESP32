#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
//#include "adc.h"
#include "ADC.h"
#include <math.h>
#include "_debug.h"

adc_t adc;

void init_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

uint16_t ReadVoltage(adc1_channel_t channel)
{
    //uint16_t reading = adc1_get_raw(channel); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095

    uint16_t reading = getADC_avg(channel); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
    double calib_reading = 0; 

#ifdef DEBUG_ADC
    //printf("\navg=%u",reading);
#endif

    if(reading < 1 || reading > 4095) return 0;
    
    calib_reading = (-0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089);

#ifdef DEBUG_ADC    
    //printf("\ncalib1=%f", calib_reading);
#endif

    calib_reading *= RES_RATIO;

#ifdef DEBUG_ADC    
    //printf("\ncalib2=%f", calib_reading);
#endif

    return (uint16_t)(calib_reading * 1000);
}

uint16_t getADC_avg(adc1_channel_t channel)
{
    int i = 0;
	adc.av = 0;
	static int num_of_elements;

    for (i = 0; i < (NUM_OF_SAMPLES - 1); i++)
	{
		adc.arr[NUM_OF_SAMPLES-i-1] = adc.arr[NUM_OF_SAMPLES-i-2];
	}

    adc.arr[0] = adc1_get_raw(channel); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095

    if (num_of_elements < NUM_OF_SAMPLES) num_of_elements++;

    for (i = 0; i < num_of_elements; i++)
	{
		adc.av += adc.arr[i];
#ifdef DEBUG_ADC
        //printf("\nadc.arr[%d]=%u", i, adc.arr[i]);
#endif
	}

#ifdef DEBUG_ADC
    // printf("\nSum=%u",adc.av);
    // printf("\nnum=%d",num_of_elements);
#endif

    adc.av = adc.av/num_of_elements;

    return adc.av; 
}