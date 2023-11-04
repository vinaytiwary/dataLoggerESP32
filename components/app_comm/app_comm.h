#ifndef __APP_COMM_H__
#define __APP_COMM_H__

#include "config.h"

#define DEFAULT_FRAME_TIMEOUT	(3)			//50 msec
#define GPRS_FRAME_TIMEOUT		(3)			//30 msec
#define DECRYPT_FRAME_TIMEOUT	(3)			//30 msec

void UWriteBytes(unsigned char *, int, int);
void UWriteInt(unsigned long, char);
char UWriteString(char *str,int no);						//write a string on uart
void Usendbuffer(int);
void checkforUARTFrameTimeout(int);
void flushTxBuffer(int);
void flushRxBuffer(int);

#endif