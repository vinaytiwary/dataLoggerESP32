#ifndef MY_RTC_H_
#define MY_RTC_H_

//#include <time.h>

void pls_work(void);
void rtc_setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms);
void rtc_setTimeStruct(struct tm t);  
struct tm getTimeStruct(void);  //PP added on 20-09-23
void setRTCTime(unsigned long epoch, int ms); 
void rtc_getDateTime(bool mode,char* s);
void rtc_getTimeDate(bool mode,char* s);
void rtc_getTime(bool mode,char* s);
void rtc_getDate(bool mode,char* s);
unsigned long rtc_getMillis();
unsigned rtc_longgetEpoch();
int rtc_getSecond();
int rtc_getMinute();
int rtc_getHour(bool mode);
int rtc_getDay();
int rtc_getMonth();
int rtc_getYear();



#endif