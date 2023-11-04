#ifndef _ADC_H_
#define _ADCH_

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define RES1 (100.0)
#define RES2 (13.3)

#define RES_RATIO ((RES1 + RES2)/ RES2)

#define NUM_OF_SAMPLES		(10)				//10

typedef struct
{
	uint16_t arr[NUM_OF_SAMPLES];
	uint16_t av;
}__attribute__((packed))adc_t;

uint16_t ReadVoltage(adc1_channel_t);
uint16_t getADC_avg(adc1_channel_t);
void init_adc();


#endif