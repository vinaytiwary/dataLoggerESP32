#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>
#include "my_rtc.h"
#include "time.h"
#include <stdlib.h>

#ifdef RTC_DATA_ATTR
RTC_DATA_ATTR static bool overflow;
#else
static bool overflow;
#endif

long offset;

void pls_work(void)
{
    printf("\npls_work");
}

/*!
    @brief  set the internal RTC time
    @param  sc
            second (0-59)
    @param  mn
            minute (0-59)
    @param  hr
            hour of day (0-23)
    @param  dy
            day of month (1-31)
    @param  mt
            month (1-12)
    @param  yr
            year ie 2021
    @param  ms
            microseconds (optional)
*/
void rtc_setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms) 
{
  // seconds, minute, hour, day, month, year $ microseconds(optional)
  // ie setTime(20, 34, 8, 1, 4, 2021) = 8:34:20 1/4/2021
  struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};      // Initalize to all 0's
  t.tm_year = yr - 1900;    // This is year-1900, so 121 = 2021
  t.tm_mon = mt - 1;
  t.tm_mday = dy;
  t.tm_hour = hr;
  t.tm_min = mn;
  t.tm_sec = sc;
  time_t timeSinceEpoch = mktime(&t);
  setRTCTime(timeSinceEpoch, ms);
}

/*!
    @brief  set time from struct
	@param	tm
			time struct
*/
void rtc_setTimeStruct(struct tm t) 
{ 
	time_t timeSinceEpoch = mktime(&t); 
	setRTCTime(timeSinceEpoch, 0); 
}

/*!
    @brief  set the internal RTC time
    @param  epoch
            epoch time in seconds
    @param  ms
            microseconds (optional)
*/
void setRTCTime(unsigned long epoch, int ms) 
{
  struct timeval tv;
  if (epoch > 2082758399)
  {
	  overflow = true;
	  tv.tv_sec = epoch - 2082758399;  // epoch time (seconds)
  } else 
  {
	  overflow = false;
	  tv.tv_sec = epoch;  // epoch time (seconds)
  }
  tv.tv_usec = ms;    // microseconds
  settimeofday(&tv, NULL);
}

/*!
    @brief  get the internal RTC time as a tm struct
*/
struct tm getTimeStruct()
{
  struct tm timeinfo;
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
  time_t tt = mktime (&timeinfo);
    
  if (overflow){
	  tt += 63071999;
  }
  if (offset > 0)
  {
	tt += (unsigned long) offset;
  } 
  else 
  {
	tt -= (unsigned long) (offset * -1);
  }
  struct tm * tn = localtime(&tt);
  if (overflow)
  {
	  tn->tm_year += 64;
  }
  return *tn;
}

/*!
    @brief  get the time and date 
    @param  mode
            true = Long date format
			false = Short date format
*/
void rtc_getDateTime(bool mode,char* s)
{
	struct tm timeinfo = getTimeStruct();
	// char s[51];
	// char* ret = (char*)malloc(51);
	if (mode)
	{
		strftime(s, 50, "%A, %B %d %Y %H:%M:%S", &timeinfo);
	}
	else
	{
		strftime(s, 50, "%a, %b %d %Y %H:%M:%S", &timeinfo);
	}
	//memcpy(ret,s,sizeof(s));
    //printf("rtc_getDateTime = %s\n",s);  
	//return s;
	//return ret;
}

/*!
    @brief  get the time and date 
    @param  mode
            true = Long date format
			false = Short date format
*/
void rtc_getTimeDate(bool mode,char* s)
{
	struct tm timeinfo = getTimeStruct();
	 //char s[51];
	// char* ret = (char*)malloc(51);
	if (mode)
	{
		strftime(s, 50, "%H:%M:%S %A, %B %d %Y", &timeinfo);
	}
	else
	{
		strftime(s, 50, "%H:%M:%S %a, %b %d %Y", &timeinfo);
	}
    ///printf("rtc_getTimeDate = %s\n",s);
	//return s;
	//return ret;
}

void rtc_getTime(bool mode,char* s)
{
	struct tm timeinfo = getTimeStruct();
	 //char s[51];
	//char* ret = (char*)malloc(51);
	strftime(s, 50, "%H:%M:%S", &timeinfo);
	//memcpy(ret,s,sizeof(s));
    ////printf("rtc_getTime = %s\n",s);
	//return s;
	//return ret;
}

void rtc_getDate(bool mode,char* s)
{
	struct tm timeinfo = getTimeStruct();
	//char s[51];
	//char *ret =(char*)malloc(51);
	if (mode)
	{
		strftime(s, 50, "%A, %B %d %Y", &timeinfo);
	}
	else
	{
		strftime(s, 50, "%a, %b %d %Y", &timeinfo);
	}
	//memcpy(ret,s,sizeof(s));
    //printf("rtc_getDate = %s\n",s);
	//return ret;
}

unsigned long rtc_getMillis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec/1000;
}

unsigned rtc_longgetEpoch()
{
	struct tm timeinfo = getTimeStruct();
	return mktime(&timeinfo);
}

int rtc_getSecond()
{
	struct tm timeinfo = getTimeStruct();
	return timeinfo.tm_sec;
}

int rtc_getMinute()
{
	struct tm timeinfo = getTimeStruct();
	return timeinfo.tm_min;
}
int rtc_getHour(bool mode)
{
	struct tm timeinfo = getTimeStruct();
	if (mode)
	{
		return timeinfo.tm_hour;
	}
	else
	{
		int hour = timeinfo.tm_hour;
		if (hour > 12)
		{
			return timeinfo.tm_hour-12;
		}
		else
		{
			return timeinfo.tm_hour;
		}
		
	}
}
int rtc_getDay()
{
	struct tm timeinfo = getTimeStruct();
	return timeinfo.tm_mday;
}
int rtc_getMonth()
{
	struct tm timeinfo = getTimeStruct();
	return timeinfo.tm_mon;
}
int rtc_getYear()
{
	struct tm timeinfo = getTimeStruct();
	return timeinfo.tm_year+1900;
}




