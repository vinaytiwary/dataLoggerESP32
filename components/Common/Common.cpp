#include <stdlib.h>
#include <string.h>

#include "Common.h"

#define BUFSIZE (sizeof(long) * 8 + 1)

char *my_ltoa(long N, char *str, int base)
{
    register int i = 2;
    long uarg;
    char *tail, *head = str, buf[BUFSIZE];

    if (36 < base || 2 > base)
        base = 10;                    /* can only use 0-9, A-Z        */
    tail = &buf[BUFSIZE - 1];           /* last character position      */
    *tail-- = '\0';

    if (10 == base && N < 0L)
    {
        *head++ = '-';
        uarg    = -N;
    }
    else  uarg = N;

    if (uarg)
    {
        for (i = 1; uarg; ++i)
        {
                register ldiv_t r;

                r       = ldiv(uarg, base);
                *tail-- = (char)(r.rem + ((9L < r.rem) ?
                                ('A' - 10L) : '0'));
                uarg    = r.quot;
        }
    }
    else  *tail-- = '0';

    memcpy(head, ++tail, i);
    return str;
}

unsigned char count_comma(char *arr)
{
	unsigned int i, cnt = 0;
	for (i=0; i<=strlen(arr); i++)
	{
		if(arr[i] == ',')
		{
			cnt++;
		}
	}
	return cnt;
}

void convertAsciiToHex(char *buff, unsigned int len)
{
	unsigned int i, j = 0;
	unsigned char temp_buff[len / 2];
	for(i = 0; i < len; i += 2)
	{
		temp_buff[j++] = hex2int(&buff[i]);
	}
	memset(buff, 0, len);
	memcpy(buff, temp_buff, (len / 2));
}

void convertAsciiToHex(char *dest,char *src, unsigned int len)
{
	unsigned int i, j = 0;
	unsigned char temp_buff[len/2];

	if(len%2 != 0)
	{
		len -= 1;
	}
	for(i = 0; i < len; i+=2)
	{
		temp_buff[j++] = hex2int(&src[i]);
		
	}
	memset(dest,0,len);
	//memcpy(buff,temp_buff,len/2);
	memcpy(dest,temp_buff,len/2);
}

unsigned char hex2int(char *str)
{
	unsigned char retval = 0,i;
	//unsigned char temp = 0;
	for(i = 0; i < 2; i++)
	{
		retval <<= 4;
		if(*str >= '0' && *str <= '9')
		{
			retval += *str - '0';
		}
		else
		{
			switch(*str)
			{
				case 'A':
				case 'a': retval += 10;
				break;
				case 'B':
				case 'b': retval += 11;
				break;
				case 'C':
				case 'c': retval += 12;
				break;
				case 'D':
				case 'd': retval += 13;
				break;
				case 'E':
				case 'e': retval +=14;
				break;
				case 'F':
				case 'f': retval += 15;
				break;
				default:
				break;
			}
		}
		//retval += temp;
		/*if(i == 0)
		{
			retval = temp*16;
		}
		else
		{
			retval += temp;
		}*/
		str++;
	}
	return retval;
}

/*unsigned*/ char getChecksum(unsigned char* buff, char len)
{
	/*unsigned*/ char chksum=0;
	int i=0;

	for(i=0 ; i<len ; i++)
	{
		chksum+=buff[i];
	}
	
	chksum = (~chksum) + 1;

	return chksum;
}