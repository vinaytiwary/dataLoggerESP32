#ifndef _HANDLE_SDCARD_
#define _HANDLE_SDCARD_

#include "clock.h"

#define SPI_DMA_CHAN  host.slot

typedef struct	
{
	uint32_t log_num;
	uint8_t quantity_of_gps;				
	int32_t Latitude;
	int32_t Longitude;
	uint16_t speed;
	int32_t msl_altitude;
	int32_t geoid_separation;
	int32_t ellipsoid_altitude;
	uint32_t PDOP;
	int32_t sign_status;
	uint16_t main_supply;
	uint16_t v_batt;
	uint32_t Status;							
	time_stamp_t time;
}__attribute__((packed))ram_data_t;

void init_sdcard();

void deinitialize_sdcard();

void reinitialize_sdcard();

void printHeader();

void SD_file_Save();

void get_random_data();

void update_ram_data(void);

void Get_Curr_sdData(const char *filename, int min);

char read_sdcard(const char *filename, long row_number);

char timer_check(int ms);

void check_switch_pressed(void);

void check_card_detect();

#endif