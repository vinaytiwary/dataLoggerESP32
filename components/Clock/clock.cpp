#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <stdint.h>
#include "clock.h"
#include "gprs.h"
#include "my_rtc.h"
#include "gps.h"
#include "EEPROM.h"
#include "app_comm.h"
#include "UART.h"
#include "Handle_sdcard.h"
#include "time.h"
#include "_debug.h"
#include "error.h"

extern gprs_date_time_t gprs_date_time;
extern gps_date_time_t gps_date_time;
time_stamp_t time_stamp;
timeDiff_t timeDiff = {0, 0, 0};
extern ram_data_t ram_data;

unsigned char check_date_time(char *str, char *arr)
{
	char array[192];
	get_rx_data(array);

	pls_work();

	int i = 0;
	char *ptr = arr;
	*ptr = 0;
	for (i = 0; i < 17; i++)
	{
		if(isdigit(str[i]))
		{
			*ptr *= 10;
			*ptr += (str[i] - '0');
		}
		else
		{
			if ((i + 1) % 3)
			{
				break;
			}
			else
			{
				switch(i)
				{
					case 2:
					case 5:
						if(str[i] == '/')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					case 8:
						if(str[i] == ',')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					case 11:
					case 14:
						if(str[i] == ':')
						{
							ptr++;
							*ptr = 0;
						}
					break;
					default:
					break;
				}
				/*if ((((i == 2) || (i == 5)) && (str[i] == '/')) || ((i == 8) && (str[i] == ',')) || (((i == 11) || (i == 14)) && (str[i] == ':')))
				{
					
				}
				ptr++;
				*ptr = 0;*/
			}
		}
	}
	if (i == 17)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void sync_time_modem(void)
{
	if(!check_RTC_time())
	{

#ifdef DEBUG_SYNC_MODEM
		//printf("\nA");
		UWriteString((char*)"\nTF", UART_PC);
#endif
		if(gprs_date_time.update_time_aval == true)
		{
			gprs_date_time.update_time_aval = false;
#ifdef DEBUG_SYNC_MODEM
			//printf("\nB");
#endif
			if ((gprs_date_time.yy <= time_stamp.yy) && (gprs_date_time.mm <= time_stamp.mm) && (gprs_date_time.dd <= time_stamp.dd) && (gprs_date_time.hr <= time_stamp.hr))
			{
#ifdef DEBUG_SYNC_MODEM
				//printf("\nC");
#endif
				//UWriteString(" No Error in Date", UART0);
			}
			else
			{
				time_stamp_t temp_maintime,temp_gprstime;
				get_present_time(&temp_maintime);
				gettime_from_gprs(&temp_gprstime);
				timeDiff = calcTimeDiff(temp_gprstime,temp_maintime);
				if(timeDiff.diffMins>0 && timeDiff.diffMins<=10 && timeDiff.diffHrs==0 && timeDiff.diffDays==0)
				{
#ifdef DEBUG_SYNC_MODEM
					//printf("\nSame");
#endif
				}
				else
				{
					char date[52];
					memset(date,0,sizeof(date));
					rtc_setTime(gprs_date_time.sec,gprs_date_time.min,gprs_date_time.hr,gprs_date_time.dd,gprs_date_time.mm,(gprs_date_time.yy+2000),0);
					rtc_getDateTime(0,date);
#ifdef DEBUG_SYNC_MODEM
					//printf("\nrtc_set");
					//printf("\nrtc_getDateTime=%s",date);
					printf("\nrtc_gprs=%s",date);
#endif
				}

			}
		}
		else
		{
			if(gps_date_time.update_time_aval == true)
			{
				gps_date_time.update_time_aval = false;
#ifdef DEBUG_SYNC_MODEM
				//printf("\nB2");
#endif
				
				if ((gps_date_time.yy <= time_stamp.yy) && (gps_date_time.mm <= time_stamp.mm) && (gps_date_time.dd <= time_stamp.dd) && (gps_date_time.hr <= time_stamp.hr))
				{
#ifdef DEBUG_SYNC_MODEM
					//printf("\nC2");
#endif
					//UWriteString(" No Error in Date", UART0);
				}
				else
				{
					time_stamp_t temp_maintime,temp_gpstime;
					get_present_time(&temp_maintime);
					gettime_from_gps(&temp_gpstime);
					timeDiff = calcTimeDiff(temp_maintime,temp_gpstime);
					if(timeDiff.diffMins>0 && timeDiff.diffMins<=10 && timeDiff.diffHrs==0 && timeDiff.diffDays==0)
					{
#ifdef DEBUG_SYNC_MODEM
						//printf("\nSame2");
#endif						
					}
					else
					{
						char date[52];
						memset(date,0,sizeof(date));
						rtc_setTime(gps_date_time.sec,gps_date_time.min,gps_date_time.hr,gps_date_time.dd,gps_date_time.mm,(gps_date_time.yy+2000),0);
						rtc_getDateTime(0,date);
#ifdef DEBUG_SYNC_MODEM
						// printf("\nrtc_set2");
						// printf("\nrtc_getDateTime2=%s",date);
						printf("\nrtc_gps=%s",date);
#endif	
					}

				}
			}
			
		}
	}
	
}

void get_present_time(time_stamp_t *time_stamp)
{
	//char str[192];
	//get_rx_data(str);
	//rtc_getDay();

	struct tm timeinfo = getTimeStruct();

	time_stamp->yy = ((timeinfo.tm_year + 1900) % 100);
	time_stamp->mm = (timeinfo.tm_mon + 1);
	time_stamp->dd = timeinfo.tm_mday;
	time_stamp->hr = timeinfo.tm_hour;
	time_stamp->min = timeinfo.tm_min;
	time_stamp->sec = timeinfo.tm_sec;

#if 0
	time_stamp->yy = (rtc_getYear())%100;
    time_stamp->mm = rtc_getMonth()+1;
    time_stamp->dd = rtc_getDay();
    time_stamp->hr = rtc_getHour(true);
    time_stamp->min = rtc_getMinute();
    time_stamp->sec = rtc_getSecond();
#endif

#ifdef DEBUG_RTC_TIME
	printf("\nrd.dt=%02d-%02d-%02d,%02d:%02d:%02d",time_stamp->dd,time_stamp->mm,time_stamp->yy,time_stamp->hr,time_stamp->min,time_stamp->sec);
	/* printf("\nget_year=%d",rtc_getYear());
	printf("\nyear=20%d",time_stamp->yy);
	printf("\nmonth=%d",time_stamp->mm);
	printf("\nday=%d",time_stamp->dd);
	printf("\nhour=%d",time_stamp->hr);
	printf("\nmin=%d",time_stamp->min);
	printf("\nsec=%d",time_stamp->sec); */
#endif
}

uint8_t check_RTC_time()
{
	memset((void*)&ram_data.time, 0, sizeof(ram_data.time));
	get_present_time(&ram_data.time);
  	//temp_time.year= rtc.getYear();
#if 0
  if (((rtc_getYear() >= DEFAULT_YEAR) && (rtc_getYear() <= (DEFAULT_YEAR + YEAR_OFFSET))) &&
      (((rtc_getMonth()+1)>= 1) && ((rtc_getMonth()+1) <= 12)) &&
      ((rtc_getDay() >= 1) && (rtc_getDay() <= 31)) &&
      ((rtc_getHour(true) >= 0) && (rtc_getHour(true) <= 23)) &&
      ((rtc_getMinute() >= 0) && (rtc_getMinute() <= 59)) &&
      ((rtc_getSecond() >= 0) && (rtc_getSecond() <= 59)) /* &&
	  (rtc_getYear()!=2000 && rtc_getMonth()!=1 && rtc_getDay()!=1) */
	  )
#endif
	if(((ram_data.time.yy >= (DEFAULT_YEAR % 100)) && (ram_data.time.yy <= ((DEFAULT_YEAR % 100) + YEAR_OFFSET))) &&
	   ((ram_data.time.mm >= 1) && (ram_data.time.mm <= 12)) &&
	   ((ram_data.time.dd >= 1) && (ram_data.time.dd <= 31)) &&
	   ((ram_data.time.hr >= 0) && (ram_data.time.hr <= 23)) &&
	   ((ram_data.time.min >= 0) && (ram_data.time.min <= 59)) &&
	   ((ram_data.time.sec >= 0) && (ram_data.time.sec <= 59)))
      {
        //time_main.RTC_flag =true;
#ifdef DEBUG_RTC_FLAG
        // printf("\nRTC_flag=true");
        // printf("\nrtc_getYear()=%d",rtc_getYear());
#endif
		set_system_state(SYS_RTC, LOW);
		date_change_e2p();
        return 1;
      }
	set_system_state(SYS_RTC, HIGH);
   //printf("\nRTC_flag=false");
   return 0;
}




int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int getDaysOfMonth(int month, int year)
{
    if (month == FEB)
    {
        return isLeapYear(year) ? 29 : 28;
    }

    switch (month)
    {
        case JAN: case MAR: case MAY: case JUL: case AUG: case OCT: case DECMB:
        return 31;
        case APR: case JUN: case SEP: case NOV:
        return 30;
        default:
        return 0;
    }
}

/*
int getDaysOfMonth(int month, int year)
{
static const int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

if (month == 2 && isLeapYear(year))
{
return 29;
}

return daysInMonth[month];
}*/

int timestampToMinutes(time_stamp_t ts) //neetu parihar
{
    int totalMinutes = 0;

    totalMinutes += ts.min;
    totalMinutes += ts.hr * 60;
    totalMinutes += ts.dd * 24 * 60;
    for (int month = JAN; month < ts.mm; ++month)
    {
        totalMinutes += getDaysOfMonth(month, ts.yy) * 24 * 60;
    }
    for (int year = 1; year < ts.yy; ++year)
    {
        totalMinutes += (isLeapYear(year) ? 366 : 365) * 24 * 60;
    }
#ifdef DEBUG_TIME_DIFF
    UWriteData('\n', UART_PC);
    UWriteInt(totalMinutes,UART_PC);
#endif

    return totalMinutes;
}

timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop) //neetu parihar
{
    timeDiff_t diff = {0, 0, 0};

    int startMinutes = timestampToMinutes(start);
    int stopMinutes = timestampToMinutes(stop);

    int totalMinutes = stopMinutes - startMinutes;

    if (totalMinutes < 0)
    {
        time_stamp_t temp = start;
        start = stop;
        stop = temp;
        totalMinutes = -totalMinutes;
    }
    diff.diffDays = totalMinutes / (24 * 60);
    totalMinutes %= (24 * 60);
    diff.diffHrs = totalMinutes / 60;
    diff.diffMins = totalMinutes % 60;

    return diff;
}

void gettime_from_gps(time_stamp_t *time_stamp)
{
	time_stamp->sec = gps_date_time.sec;
	time_stamp->min = gps_date_time.min;
	time_stamp->hr = gps_date_time.hr;
	time_stamp->dd = gps_date_time.dd;
	time_stamp->mm = gps_date_time.mm;
	time_stamp->yy = gps_date_time.yy;
}

void gettime_from_gprs(time_stamp_t *time_stamp)
{
	time_stamp->sec = gprs_date_time.sec;
	time_stamp->min = gprs_date_time.min;
	time_stamp->hr = gprs_date_time.hr;
	time_stamp->dd = gprs_date_time.dd;
	time_stamp->mm = gprs_date_time.mm;
	time_stamp->yy = gprs_date_time.yy;
}