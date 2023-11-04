#ifndef COMMON_H_
#define COMMON_H_

#include "freertos/portmacro.h"

#define WEB_COMMS_SCHEDULAR_TIME (50)

#define SETUPA_testing  (0)
#define SETUPB_testing  (1)

#define DEFAULT_YEAR	(2023)		//PP added on 29-06-23: moving this here from gprs.h
#define YEAR_OFFSET		(10)

#ifndef HIGH
#define	HIGH 0x01u
#endif
#ifndef LOW
#define	LOW 0x00u
#endif

static portMUX_TYPE isr_mutex_global = portMUX_INITIALIZER_UNLOCKED;

char *my_ltoa(long, char *, int);

unsigned char count_comma(char *);

void convertAsciiToHex(char *buff, unsigned int len);
void convertAsciiToHex(char *dest,char *src, unsigned int len);
unsigned char hex2int(char *str);

char getChecksum(unsigned char *buff, char len);

#endif