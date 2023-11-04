#include "esp_system.h"
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "error.h"
#include "Handle_sdcard.h"
#include "GPIO.h"
#include "UART.h"
#include "app_comm.h"
#include "gps.h"
#include "gprs.h"
#include "_debug.h"

extern gps_t gps;
extern gprs_t gprs;
extern gps_statuses_t gps_statuses;
extern ram_data_t ram_data;
extern gprs_date_time_t gprs_date_time;
extern gps_date_time_t gps_date_time;
extern uint8_t e2p_read_date;

void set_system_state(GDL_err_sts_bits_t data, unsigned char sts)
{
	if (sts)
	{
		ram_data.Status |= (1 << data);
	} 
	else
	{
		ram_data.Status &= ~(1 << data);
	}
}

unsigned long get_system_status(void)
{
	return ram_data.Status;
}

void check_system_status(void)
{
	char sts_display[16];
	memset((void *)sts_display, 0, sizeof(sts_display));

	if (ram_data.time.yy == 00)
	{
		set_system_state(SYS_RTC, HIGH);
	}

	/* if (!gps_statuses.gps_ready)
	{
		set_system_state(SYS_GPS_PASS, HIGH);
	}
	else
	{
		set_system_state(SYS_GPS_PASS, LOW);
	} */

	set_system_state(SYS_GGA, gps_statuses.gga_sts);
	set_system_state(SYS_GNSS, gps_statuses.gnss_sts);

	//set_system_state(SYS_GPRS_DATE, gprs_date_time.update_time_aval);
	//set_system_state(SYS_GPS_DATE, gps_date_time.update_time_aval);

	set_system_state(SYS_CHARGING, get_charger_sts());

	/* if(get_charger_sts() == NOT_CHARGING)
	{
		printf("\nbatt=%u", ram_data.v_batt);
	} */

	sprintf(sts_display, "\nSTS:0x%08lX",ram_data.Status);
	
	//UWriteBytes((unsigned char*)sts_display, sizeof(sts_display), UART_PC);
}

char Check_card_detection(esp_err_t err) 
{
    if (err != ESP_OK) 
    {
        if (err == ESP_ERR_TIMEOUT) 
        {
			printf("\nsd card not detected");
            return 0;
        } 
		else 
        {
            //printf("Error: %s\n", esp_err_to_name(err));
			printf("\nSD Card detected");
			return 1;
        }
    }
	return 1;
}



