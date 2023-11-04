#include <stdlib.h>
#include <string.h>

#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "esp_timer.h"

#include "app_comm.h"
#include "UART.h"
#include "gprs.h"
#include "gps.h"
#include "config.h"
#include "Common.h"
#include "web_comm.h"
#include "_debug.h"
#include "error.h"

extern gprs_t gprs;
extern gps_t gps;
extern gps_statuses_t gps_statuses;
//extern gps_state_t gps_handler_state;
//extern bool NMEA_enabled;
//conn_state_t conn_state = CONNECT_LOCATION;   //PP commented on 05-10-23
conn_state_t conn_state = CONNECT_BEGIN;    //PP added on 05-10-23 (among other webcomm, gprs, gps chnges regarding their return statuses)
unsigned int gps_read_timeout = 0;
unsigned int gprs_read_timeout = 0;

con_status_t modem_initAT(void)
{
    static con_status_t sts = CON_IN_PRG;
    //static gprs_connct_state_t ATcmd_sts = GPRS_CONNCT_CMD_AT;
    static gprs_connct_state_t gprs_connct_state = GPRS_CONNCT_CMD_AT;
    static char gprs_retry_count = 0;
    static unsigned int timeout = 0;

    switch(gprs_connct_state)
    {
        case GPRS_CONNCT_CMD_AT:
        {
            sts = CON_IN_PRG;
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);
#ifdef GPRS_DEBUG
            UWriteString((char*)"cmd: AT", UART_PC);
#endif
           UWriteString((char*)"AT\r\n",GPRS_UART);
           gprs_connct_state = GPRS_CONNCT_RSP_AT;
        }
        break;

        case GPRS_CONNCT_RSP_AT:
        {
            switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"AT:f", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_AT;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						/* gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT; */
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_CMD_AT;			//
						gprs_retry_count = 0;
					}
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    sts = CON_WAIT;
#ifdef GPRS_DEBUG
					UWriteString((char *)"AT:k", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
					gprs_retry_count = 0;
					timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
                    gprs_connct_state = GPRS_CONNCT_CMD_AT; //PP added on 05-10-23 (among other webcomm, gprs, gps chnges regarding their return statuses)
                    if(timeout++ >= GPRS_AT_TIMEOUT)
					{
#ifdef GPRS_DEBUG
					    UWriteString((char *)"AT:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_AT;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
					        UWriteString((char *)"AT:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT; */
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_CMD_AT;			//
							gprs_retry_count = 0;
						}
					}
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_ECHO_OFF:
        {
            sts = CON_IN_PRG;
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: ECHO_OFF", UART_PC);
#endif
            UWriteString((char*)"ATE0\r\n",GPRS_UART);
            gprs_connct_state = GPRS_CONNCT_RSP_ECHO_OFF;
        }
        break;

        case GPRS_CONNCT_RSP_ECHO_OFF:
        {
            switch (check_string_nobuf("OK"))
			{
				case (GPRS_MATCH_FAIL):
                {
#ifdef GPRS_DEBUG
				    UWriteString((char *)"ECHO_OFF: f", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
				
					if (gprs_retry_count++ >= RETRY_CNT)
					{
						/* gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT; */
						sts = CON_FAIL;
						
						gprs_connct_state = GPRS_CONNCT_CMD_AT;			//
						gprs_retry_count = 0;
					}
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef GPRS_DEBUG
				UWriteString((char *)"ECHO_OFF: K", UART_PC);
#endif
                    sts = CON_OK;
                    gprs_connct_state = GPRS_CONNCT_CMD_AT;
					gprs_retry_count = 0;
					timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
                   if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
				        UWriteString((char *)"ECHO_OFF: t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
				             UWriteString((char *)"ECHO_OFF: t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT; */
							sts = CON_FAIL;
							
							gprs_connct_state = GPRS_CONNCT_CMD_AT;			//
							gprs_retry_count = 0;
						}
					} 
                }
                break;

                default:
                break;

            }
        }

        default:
        break;
    }
    return sts;
}

# if 0
void manage_gps_gprs(void)
{
    gps_status_t sts = GPS_IN_PRG;
    switch (conn_state)
	{
        case CONNECT_LOCATION:
        {
            sts = gps_handler();
            if(sts == GPS_PASS)
            {
                set_system_state(SYS_GPS_PASS, LOW);
#ifdef DEBUG_WEB_COMM
                UWriteString((char *)"\nGPRS", UART_PC);
#endif
                if(gprs.state == GPRS_IDLE || gprs.state == GPRS_AT_INIT)
                {
                    gprs.state = GPRS_CONNECT;
                }

                conn_state = CONNECT_DATA_UPLOAD;
                gprs_read_timeout = 0;
            }
            else //dangerous if @startup we have'nt gotten gps lock yet and we switch to GPRS b4 turnig NMEA off
            {
                //if(gps_statuses.got_VBATT)
                {
                    set_system_state(SYS_GPS_PASS, HIGH);
                }
                
                //(gps_statuses.NMEA_enabled):could be checked using handler state but that won't tell us if the response matched i.e. if it's actually enabled or not.
                if ((gprs_read_timeout++ >= GPS_FAIL_TIMEOUT) && (!gps_statuses.NMEA_enabled/*  && gps_statuses.gps_handler_state == GPS_RSP_GPGGA_STOP */))
                {
#ifdef DEBUG_WEB_COMM
                    UWriteString((char *)"\nGPRS_T", UART_PC);
#endif					
                    conn_state = CONNECT_DATA_UPLOAD;
                    gprs_read_timeout = 0;
                
                }
            }

        }
        break;

        case CONNECT_DATA_UPLOAD:
        {
            http_handler();
            if((gps_read_timeout++ >= GPS_READ_RATE_C/* gps.timeout */))
            {
#ifdef DEBUG_WEB_COMM
                UWriteString((char *)"\nGPS", UART_PC);
#endif		
                //if (gprs.state != GPRS_LOGS_UPLOAD)
                {
                    conn_state = CONNECT_LOCATION;    //won't be reset to CONNECT_LOCATION untill we reach 1 second scheduler.
                    gps_read_timeout = 0;
                }	
            }
        }
        break;

        default:
        break;
    }
}
#endif
#if 1
void manage_gps_gprs(void)
{
    static uint32_t gprs_strt = 0;
    static uint32_t gps_strt = 0;

    static uint32_t gps_diff = 0;
    static uint32_t gprs_diff = 0;

    gps_status_t gps_sts = GPS_IN_PRG;
    con_status_t gprs_sts = CON_IN_PRG;
    //con_status_t init_sts = CON_IN_PRG;

    switch(conn_state)
    {
        case CONNECT_BEGIN:
        {
            set_system_state(SYS_GPS_PASS, HIGH);
            set_system_state(SYS_GPS_DATE, HIGH);
            set_system_state(SYS_GPRS_DATE, HIGH);

            con_status_t init_sts = CON_IN_PRG;
            init_sts = /* (char) */modem_initAT();
            if(init_sts == CON_OK)
			{  
                gps_strt = (esp_timer_get_time()/1000);
                gps_statuses.gps_handler_state = GPS_CMD_QUERY_ENABLE;
                gprs.state = GPRS_CONNECT;

				conn_state = CONNECT_LOCATION;
			}
        }
        break;

        case CONNECT_LOCATION:
        {
            //uint32_t gps_strt = (esp_timer_get_time()/1000);
            gps_sts = gps_handler();

            if (((gps_sts == GPS_WAIT) || (gps_sts == GPS_PASS)) && (!gps_statuses.NMEA_enabled))
            {
                //set_system_state(SYS_GPS_PASS, LOW);  //PP 05-10-23: now happening only gps_handler if GPS_PASS
                gps_diff = (esp_timer_get_time()/1000) - gps_strt;
#ifdef DEBUG_WEB_COMM
                UWriteString((char *)"\nGPRS", UART_PC);
#endif
#ifdef DEBUG_WC_MILLIS
                //printf("\ngps_d1=%lld",(esp_timer_get_time()/1000) - gps_strt);
                //printf("\ngps_d1=%ld,%ld",gps_diff,(gprs_diff + gprs_read_timeout));
                printf("\ngps_d1=%ld",gps_diff);
#endif
                /* if(gprs.state == GPRS_IDLE || gprs.state == GPRS_AT_INIT)
                {
                    gprs.state = GPRS_CONNECT;
                } */
                //gprs_diff = 0;
                gprs_strt = (esp_timer_get_time()/1000);
                conn_state = CONNECT_DATA_UPLOAD;
                gprs_read_timeout = 0;
            }
            else
            {
                set_system_state(SYS_GPS_PASS, HIGH);

                //uint16_t gps_fail_tout = gps_statuses.gps_handler_state < GPS_CMD_CBC? 200:1000;

                //gprs_diff = (esp_timer_get_time()/1000) - gprs_strt;
                //gprs_read_timeout++;
                gprs_read_timeout += 50;
                
/* #ifdef DEBUG_WC_MILLIS
                //printf("\ngps_d2=%ld,%ld",gprs_diff,(gprs_diff + gprs_read_timeout));
                printf("\ngprs_d3=%ld,%ld",gprs_diff,(gprs_diff + gprs_read_timeout));
#endif */
                
                //if((gprs_diff + gprs_read_timeout >= gps_fail_tout) && (!gps_statuses.NMEA_enabled))
                if((gprs_diff + gprs_read_timeout >= 1000) && (!gps_statuses.NMEA_enabled))
                //if ((gprs_read_timeout++ >= GPS_FAIL_TIMEOUT) && (!gps_statuses.NMEA_enabled))
                {
                    gps_diff = (esp_timer_get_time()/1000) - gps_strt;
#ifdef DEBUG_WEB_COMM
                    UWriteString((char *)"\nGPRS_T", UART_PC);
#endif				
#ifdef DEBUG_WC_MILLIS
                    //printf("\ngps_d3=%lld",(esp_timer_get_time()/1000) - gps_strt);
                    //printf("\ngps_d2=%ld,%ld",gps_diff,(gprs_diff + gprs_read_timeout));
                    printf("\ngps_d2=%ld",gps_diff);
#endif	
                    //gprs_diff = 0;
                    gprs_strt = (esp_timer_get_time()/1000);
                    conn_state = CONNECT_DATA_UPLOAD;
                    gprs_read_timeout = 0;
                
                }
            }
        }
        break;

        case CONNECT_DATA_UPLOAD:
        {
            //uint32_t gprs_strt = (esp_timer_get_time()/1000);
            gprs_sts = http_handler();

            if((gprs_sts == CON_WAIT) || (gprs_sts == CON_OK)) //(gprs_sts != CON_IN_PRG)??
            {
                gprs_diff = (esp_timer_get_time()/1000) - gprs_strt;
#ifdef DEBUG_WEB_COMM
                UWriteString((char *)"\nGPS", UART_PC);
#endif		
#ifdef DEBUG_WC_MILLIS
                //printf("\ngprs_d1=%lld",(esp_timer_get_time()/1000) - gprs_strt);
                //printf("\ngprs_d1=%ld,%ld",gprs_diff,(gps_diff + gps_read_timeout));
                printf("\ngprs_d1=%ld",gprs_diff);
#endif
                //if (gprs.state != GPRS_LOGS_UPLOAD)
                {
                    //gps_diff = 0;
                    gps_strt = (esp_timer_get_time()/1000);
                    conn_state = CONNECT_LOCATION;    //won't be reset to CONNECT_LOCATION untill we reach 1 second scheduler.
                    gps_read_timeout = 0;
                }	
            }
            else
            {
                //uint16_t gprs_fail_tout = gprs.state < GPRS_CONN_STS? 200:1000;

                //gps_diff = (esp_timer_get_time()/1000) - gps_strt;
                //gps_read_timeout++;
                gps_read_timeout += 50;

/* #ifdef DEBUG_WC_MILLIS
                //printf("\ngprs_d2=%ld,%ld",gps_diff,(gps_diff + gps_read_timeout));
                printf("\ngps_d3=%ld,%ld",gps_diff,(gps_diff + gps_read_timeout));
#endif */
                //if((gps_diff + gps_read_timeout >= gprs_fail_tout) && ((gprs_sts != CON_IN_PRG)||(gprs.state == GPRS_SESSION_IDLE)))
                if((gps_diff + gps_read_timeout >= 1000) && ((gprs_sts != CON_IN_PRG)||(gprs.state == GPRS_SESSION_IDLE)))
                //if((gps_read_timeout++ >= GPS_READ_RATE_C) && ((gprs_sts != CON_IN_PRG)||(gprs.state == GPRS_SESSION_IDLE)))
                {
                    gprs_diff = (esp_timer_get_time()/1000) - gprs_strt;
#ifdef DEBUG_WEB_COMM
                    UWriteString((char *)"\nGPS_T", UART_PC);
#endif		
#ifdef DEBUG_WC_MILLIS
                    //printf("\ngprs_d3=%lld",(esp_timer_get_time()/1000) - gprs_strt);
                    //printf("\ngprs_d2=%ld,%ld",gprs_diff,(gps_diff + gps_read_timeout));
                    printf("\ngprs_d2=%ld",gprs_diff);
#endif
                    //if (gprs.state != GPRS_LOGS_UPLOAD)
                    {
                        //gps_diff = 0;
                        gps_strt = (esp_timer_get_time()/1000);
                        conn_state = CONNECT_LOCATION;    //won't be reset to CONNECT_LOCATION untill we reach 1 second scheduler.
                        gps_read_timeout = 0;
                    }	
                } 
            }
        }
        break;

        default:
        break;
    }
}
#endif