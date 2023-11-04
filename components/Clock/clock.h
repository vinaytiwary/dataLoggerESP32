#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

typedef struct
{
  char yy;
  char mm;
  char dd;
  char hr;
  char min;
  char sec;
  //char GPS_flag;
}time_stamp_t;

typedef struct
{
  char diffHrs;
  char diffMins;
  long diffDays;
}timeDiff_t;

typedef enum
{
JAN = 1,
FEB,
MAR,
APR,
MAY,
JUN,
JUL,
AUG,
SEP,
OCT,
NOV,
DECMB
} Month;

void sync_time_modem(void);
void get_present_time(time_stamp_t *time_stamp);
uint8_t check_RTC_time();
int isLeapYear(int year);
unsigned int getDaysOfMonth(char , unsigned int );
int getDaysOfMonth(int month, int year);
int timestampToMinutes(time_stamp_t ts) ;//neetu parihar;
timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop);
void gettime_from_gps(time_stamp_t *time_stamp);
void gettime_from_gprs(time_stamp_t *time_stamp);
unsigned char check_date_time(char *, char *);

#endif