#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "esp_timer.h"

#include "config.h"
#include "UART.h"
#include "gprs.h"
#include "app_comm.h"
#include "gps.h"
#include "web_comm.h"
#include "common.h"
#include "clock.h"
#include "Handle_sdcard.h"
#include "GPIO.h"
#include "pins.h"
#include "_debug.h"
#include "error.h"
#include "I2C.h"
//#include "adc.h"

extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
//gps_state_t gps_handler_state = GPS_IDLE;
gps_t gps;
extern ram_data_t ram_data;
gps_date_time_t gps_date_time;
gps_statuses_t gps_statuses = {0,0,false,false,false,false,GPS_IDLE};
//bool NMEA_enabled = false;  //this could be checked using handler state but that won't tell us if the response matched i.e. if it's actually enabled or not.

void get_location(void)
{
    char temp[8];
	uint8_t i = 0, j = 0;
	
	unsigned char local_lat[LAT_LEN], local_long[LONG_LEN];
	
	uint32_t make_fraction = 1;//, local_log_num = ram_data.log_num;

    //if (((gps.gps_info.gns_info.latitude[0] != '\0') || (gps.gps_info.gns_info.longitude[0] != '\0'))) 
	//{		
#ifdef DEBUG_GET_LOC
		printf("\nGETLOC1");
#endif		
		//memset(&ram_data, 0, sizeof(ram_data_t) - (sizeof(time_stamp_t)+(sizeof(uint32_t)*2)));	//for testing

        //ram_data.log_num = local_log_num;
        
        if(gps.gps_info.gga_info.quantity_of_gps)
        {
        	memset(&ram_data.quantity_of_gps, 0, sizeof(ram_data.quantity_of_gps));
        	ram_data.quantity_of_gps = gps.gps_info.gga_info.quantity_of_gps;
		
#ifdef DEBUG_GET_LOC
			printf("\nrd.s=%d", ram_data.quantity_of_gps);
#endif
        }
        
        if((gps.gps_info.gns_info.sog[0] != '\0') && (isdigit(gps.gps_info.gns_info.sog[0])))
        {
        	memset(&ram_data.speed, 0, sizeof(ram_data.speed));
        	
        	memset((void *)temp, 0, sizeof(temp));
			// speed
			for (i = 0;(gps.gps_info.gns_info.sog[i] != '.'); i++)
			{
				temp[i] = gps.gps_info.gns_info.sog[i];
			}
			temp[i] = '\0';
			ram_data.speed = atoi(temp);
	
#ifdef DEBUG_GET_LOC
			printf("\nrd.speed1=%d", ram_data.speed);
#endif
			
			memset((void*) temp, 0, sizeof(temp));
			for( i = i + 1; j < SPEED_DP; i++)
			{
				temp[j++] = gps.gps_info.gns_info.sog[i];	
			}		
			temp[j] = '\0';
			ram_data.speed = (ram_data.speed * 100) + atoi(temp);
			
#ifdef DEBUG_GET_LOC
			printf("\nrd.speed2=%d", ram_data.speed);
#endif
        }
        
        if((gps.gps_info.gns_info.latitude[0] != '\0') /*&& (isdigit(gps.gps_info.gns_info.latitude[0]))*/)
        {
        	memset(&ram_data.Latitude, 0, sizeof(ram_data.Latitude));
        	memset(local_lat, 0, LAT_LEN);
        	memcpy(local_lat, gps.gps_info.gns_info.latitude, LAT_LEN);
        	convert_dmsTOdd((char*)local_lat, gps.gps_info.gns_info.N_S);
        	
#ifdef DEBUG_GET_LOC
			printf("\nconvLat=%s", local_lat);
#endif
			// latitude
			memset((void *)temp, 0,  sizeof(temp));
			
			//for (i = 0, j = 0;i <= strlen((char *)gps.gps_info.gns_info.latitude); i++)
			for (i = 0, j = 0;i <= strlen((char *)local_lat); i++)
			{
#ifdef DEBUG_GET_LOC
				//printf("\ngps.gps_info.gns_info.latitude[i]=%c", gps.gps_info.gns_info.latitude[i]);
#endif
				//if(gps.gps_info.gns_info.latitude[i] != '.')
				if(local_lat[i] != '.')
				{
					//temp[j++] = gps.gps_info.gns_info.latitude[i];
					temp[j++] = local_lat[i];
				}
			}
			ram_data.Latitude = strtol(temp,NULL,10);//*10;
#ifdef DEBUG_GET_LOC
			printf("\nrd.Lat1=%ld", ram_data.Latitude);
			printf("\nrd.Lat2=%+03ld.%06lu", ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L));
#endif		
        }
        
        if((gps.gps_info.gns_info.longitude[0] != '\0') /*&& (isdigit(gps.gps_info.gns_info.longitude[0]))*/)
        {
        	memset(&ram_data.Longitude, 0, sizeof(ram_data.Longitude));
        	memset(local_long, 0, LONG_LEN);
        	memcpy(local_long, gps.gps_info.gns_info.longitude, LONG_LEN);
			convert_dmsTOdd((char*)local_long, gps.gps_info.gns_info.E_W);
			
#ifdef DEBUG_GET_LOC
			printf("\nconvLong=%s", local_long);
#endif			
			// longitude	
			memset(temp,0,sizeof(temp));
			
			//for (i = 0, j = 0;i <= strlen((char *)gps.gps_info.gns_info.longitude); i++)
			for (i = 0, j = 0;i <= strlen((char *)local_long); i++)
			{
#ifdef DEBUG_GET_LOC
				//printf("\ngps.gps_info.gns_info.longitude[i]=%c", gps.gps_info.gns_info.longitude[i]);
#endif
				//if(gps.gps_info.gns_info.longitude[i] != '.')
				if(local_long[i] != '.')
				{
					//temp[j++] = gps.gps_info.gns_info.longitude[i];
					temp[j++] = local_long[i];
				}
			}
			ram_data.Longitude = strtol(temp,NULL,10);//*10;
#ifdef DEBUG_GET_LOC
			printf("\nrd.Long1=%ld", ram_data.Longitude);
			printf("\nrd.Long2=%+04ld.%06lu", ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L));
#endif	
        }
		
		if((gps.gps_info.gga_info.MSL_altitude[0] != '\0') /*&& (isdigit(gps.gps_info.gga_info.MSL_altitude[0]))*/)
		{
			memset(&ram_data.msl_altitude, 0, sizeof(ram_data.msl_altitude));
			memset(temp,0,sizeof(temp));
		
			for (i = 0, j = 0;i <= strlen((char *)gps.gps_info.gga_info.MSL_altitude); i++)
			{
#ifdef DEBUG_GET_LOC
				//printf("\ngps.gps_info.gga_info.MSL_altitude[i]=%c", gps.gps_info.gga_info.MSL_altitude[i]);
#endif
				if(gps.gps_info.gga_info.MSL_altitude[i] != '.')
				{
					temp[j++] = gps.gps_info.gga_info.MSL_altitude[i];
				}
			}
			ram_data.msl_altitude = strtol(temp,NULL,10)*10;
#ifdef DEBUG_GET_LOC
			printf("\nrd.m_a1=%ld", ram_data.msl_altitude);
			printf("\nrd.m_a2=%+05ld.%02ld",ram_data.msl_altitude/100, abs(ram_data.msl_altitude%100));
#endif
		}
		
		if((gps.gps_info.gga_info.Geoid_separation[0] != '\0') /*&& (isdigit(gps.gps_info.gga_info.Geoid_separation[0]))*/)
		{
			memset(&ram_data.geoid_separation, 0, sizeof(ram_data.geoid_separation));
			memset(temp,0,sizeof(temp));
		
			for (i = 0, j = 0;i <= strlen((char *)gps.gps_info.gga_info.Geoid_separation); i++)
			{
#ifdef DEBUG_GET_LOC
				//printf("\ngps.gps_info.gga_info.Geoid_separation[i]=%c", gps.gps_info.gga_info.Geoid_separation[i]);
#endif
				if(gps.gps_info.gga_info.Geoid_separation[i] != '.')
				{
					temp[j++] = gps.gps_info.gga_info.Geoid_separation[i];
				}
			}
			ram_data.geoid_separation = strtol(temp,NULL,10)*10;
#ifdef DEBUG_GET_LOC
			printf("\nrd.g_s1=%ld", ram_data.geoid_separation);
			printf("\nrd.g_s2=%+05ld.%02ld", ram_data.geoid_separation/100, abs(ram_data.geoid_separation%100));
#endif
		}	
		
		if(ram_data.msl_altitude && ram_data.geoid_separation)
		{
			memset(&ram_data.ellipsoid_altitude, 0, sizeof(ram_data.ellipsoid_altitude));
			ram_data.ellipsoid_altitude = ram_data.msl_altitude + ram_data.geoid_separation;
#ifdef DEBUG_GET_LOC
			printf("\nrd.e_a1=%ld", ram_data.ellipsoid_altitude);
			printf("\nrd.e_a2=%+05ld.%02ld", ram_data.ellipsoid_altitude/100, abs(ram_data.ellipsoid_altitude%100));
#endif
		}

		if((gps.gps_info.gns_info.pdop[0] != '\0') && (isdigit(gps.gps_info.gns_info.pdop[0])))
		{
			memset(&ram_data.PDOP, 0, sizeof(ram_data.PDOP));
			memset(temp,0,sizeof(temp));
		
			for (i = 0, j = 0;i <= strlen((char *)gps.gps_info.gns_info.pdop); i++)
			{
#ifdef DEBUG_GET_LOC
			    //printf("\ngps.gps_info.gns_info.pdop[i]=%c", gps.gps_info.gns_info.pdop[i]);
#endif
				if(gps.gps_info.gns_info.pdop[i] != '.')
				{
					temp[j++] = gps.gps_info.gns_info.pdop[i];
				}
			}
#ifdef DEBUG_GET_LOC
			printf("\nPDOP_temp=%s", temp);
#endif
			ram_data.PDOP = strtol(temp,NULL,10)*10;
#ifdef DEBUG_GET_LOC
			printf("\nrd.PDOP1=%lu", ram_data.PDOP);
			printf("\nrd.PDOP2=%03lu.%02lu", ram_data.PDOP/100, ram_data.PDOP%100);
#endif
		}
    //}
    /*
    else
	{
#ifdef DEBUG_GET_LOC
		printf("\nGETLOC2");
		printf("\nrcvLat=%s", gps.gps_info.gns_info.latitude);
		printf("\nrcvLong=%s", gps.gps_info.gns_info.longitude);
#endif
				
	}*/
}

void convert_dmsTOdd(char* location_buff, unsigned char sign)
{
	double loc_dms = /*(float)*/atof(location_buff);
#ifdef DEBUG_GET_LOC
    printf("\nLocBuff=%s, loc_dms = %f", location_buff, loc_dms);
#endif
	
	float loc_dd = (floor(loc_dms / 100) + fmod(loc_dms, 100.) / 60) *
            (sign == 'N' || sign == 'E' ? 1 : -1);
            
#ifdef DEBUG_GET_LOC
//    printf("\nloc_dms/100 = %f", loc_dms/100);
//    printf("\nfloor(loc_dms / 100) = %f", floor(loc_dms / 100));
//    printf("\nfmod(loc_dms, 100) = %f", fmod(loc_dms, 100));
//    printf("\nfmod(loc_dms, 100.)/60 = %f", fmod(loc_dms, 100)/60);
//    printf("\n(floor(loc_dms / 100) + fmod(loc_dms, 100.)/60) = %f", (floor(loc_dms / 100) + fmod(loc_dms, 100)/60));
//    printf("\n(floor(loc_dms / 100) + fmod(loc_dms, 100.) / 60) * (sign == 'N' || sign == 'E' ? 1 : -1) = %f", (floor(loc_dms / 100) + fmod(loc_dms, 100.) / 60) *(sign == 'N' || sign == 'E' ? 1 : -1));
    printf("\nloc_dd = %f", loc_dd);
#endif    
	
	memset(location_buff, 0, strlen(location_buff));
	sprintf(location_buff,"%f", loc_dd);	

#ifdef DEBUG_GET_LOC
    printf("\nLocBuff=%s", location_buff);
#endif	
}

gps_status_t gps_handler(void)
{
    static char gps_retry_cnt = 0;
	gps_status_t ret_sts = GPS_IN_PRG;
	static unsigned int timeout = 0;
	//static char gns_gga_retry = 0; 

    char status = 0;

    switch(gps_statuses.gps_handler_state)
    {
        case GPS_IDLE:
        {
            gps_statuses.got_VBATT = false;
            gps_statuses.gga_sts = false;
            gps_statuses.gnss_sts = false;
            gps_statuses.NMEA_enabled = false;

            gps_statuses.gps_handler_state = GPS_AT_INIT;
            set_system_state(SYS_GPS_PASS, HIGH);
            // set_system_state(SYS_GGA, HIGH);
	        // set_system_state(SYS_GNSS, HIGH);
        }
        break;

        case GPS_AT_INIT:
        {
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
            UWriteString((char*)"GP_AI", UART_PC);
#endif
            status = (char)modem_initAT();
            if(status == CON_OK)
			{
				//gps_statuses.gps_handler_state = GPS_CMD_ENABLE;
                gps_statuses.gps_handler_state = GPS_CMD_QUERY_ENABLE;   //PP added on 08-09-23. we need to check if it's already enabled first
			}
			else if(status == CON_FAIL)
			{
                ret_sts = GPS_FAIL;
				gps_statuses.gps_handler_state = GPS_IDLE;		//Lets go to idle and wait for trigger to re do
			}
			else if ((status == CON_IN_PRG) || (status == CON_WAIT))
			{
				//Nothing to do, keep going
			}
        }
        break;

        case GPS_CMD_QUERY_ENABLE:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: CGPS?", UART_PC);
#endif
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            UWriteString((char *)"AT+CGPS?\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_QUERY_ENABLE;            
        }
        break;

        case GPS_RSP_QUERY_ENABLE:
        {
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\ngENQ_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            switch (check_string_nobuf("+CGPS: 1,1"))
		    {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPS_DEBUG
			        UWriteString((char *)"CGPS:f", UART_PC);
#endif                   
                    gps_statuses.gps_handler_state = GPS_CMD_ENABLE;
                }
                break;

                case GPRS_MATCH_OK:
                {
                    ret_sts = GPS_WAIT;
#ifdef GPS_DEBUG
			        UWriteString((char *)"CGPS?:k", UART_PC);
#endif
                    gps_retry_cnt = 0;
                    timeout = 0;
                    //gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;  //PP commeneted on 12-09-23 for testing GGA_GNSS_CMD
#ifdef COMBINE_GGA_GNSS_CMD
                    gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS; //PP added on 12-09-23 for testing GGA_GNSS_CMD
#else 
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START; 
                    gps_statuses.gps_handler_state = GPS_CMD_CBC;
#endif
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
#ifdef GPS_DEBUG
                        UWriteString((char*)"CGPS?:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_QUERY_ENABLE;
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"CGPS?:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }                   
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPS_CMD_ENABLE:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: CGPS_EN", UART_PC);
#endif
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            UWriteString((char *)"AT+CGPS=1\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_ENABLE;
        }
        break;

        case GPS_RSP_ENABLE:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\ngEN_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            switch (check_string_nobuf("OK"))
		    {
                case GPRS_MATCH_FAIL:   //PP 06-09-23 to do: add query cmd for CGPS in case of CGPS match fail
                {
#ifdef GPS_DEBUG
			        UWriteString((char *)"CGPS:f", UART_PC);
#endif
                    gps_statuses.gps_handler_state = GPS_CMD_QUERY_ENABLE;   //PP added on 08-09-23. We should check if gps is already enabled if the response is "ERROR".

                    /* gps_statuses.gps_handler_state = GPS_CMD_ENABLE;  //PP commented on 08-09-23: since adding a new cmd "GPS_CMD_QUERY_ENABLE"

                    if (gps_retry_cnt++ >= GPS_RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        
                        gps_statuses.gps_handler_state = GPS_AT_INIT; 
                        timeout = 0;
                    } */
                }
                break;

                case GPRS_MATCH_OK:
                {
                    ret_sts = GPS_WAIT;
#ifdef GPS_DEBUG
			        UWriteString((char *)"CGPS:k", UART_PC);
#endif
                    gps_retry_cnt = 0;
                    timeout = 0;
                    //gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
                    //gps_handler_state_state = GPS_CMD_GPGGA_START;  //PP commeneted on 12-09-23 for testing GGA_GNSS_CMD
#ifdef COMBINE_GGA_GNSS_CMD
                    gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS; //PP added on 12-09-23 for testing GGA_GNSS_CMD
#else 
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START; 
                    gps_statuses.gps_handler_state = GPS_CMD_CBC;
#endif
                }
                break;

                case GPRS_NO_NEW_MSG:
                {                    
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
#ifdef GPS_DEBUG
			            UWriteString((char*)"CGPS:t1", UART_PC);
#endif
                        timeout = 0;
                        //gps_statuses.gps_handler_state = GPS_CMD_LOCATION;   //?? why cmd LCN if CGPS does not respond? copy paste k galat nateeje!
                        gps_statuses.gps_handler_state = GPS_CMD_ENABLE; //PP 08-09-23: fixed this.
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"CGPS:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPS_CMD_CBC:
        {
            ret_sts = GPS_IN_PRG;

            //GPIO.out ^= (1<<ESP_CAN_TX);
            //GPIO.out |= (1<<ESP_CAN_TX);
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif

#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: CBC", UART_PC);
#endif            
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);
            //prev_charger_sts = disable_charger();

            //gps_statuses.gga_sts = false;

            UWriteString((char*)"AT+CBC\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_CBC;
        }
        break;

        case GPS_RSP_CBC:
        {
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\ncbc_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];			
			int num_bytes = 0;
            
            char resp = check_string("+CBC: ", tmpstr,&num_bytes);

            
            switch (resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPS_DEBUG
				    UWriteString((char*)"CBC:f", UART_PC);
#endif
				    gps_statuses.gps_handler_state = GPS_CMD_CBC;

                    gps_retry_cnt++;
                    if (gps_retry_cnt >= GPS_RETRY_CNT)
                    {
                        //gps_statuses.errcode = GPS_ERR_OFFSET + GPS_RSP_STATUS;
                        ret_sts = GPS_FAIL;
                        gps_retry_cnt = 0;
                        gps_statuses.gps_handler_state = GPS_AT_INIT;

                        /* //if(!prev_charger_sts)
                        {
                            prev_charger_sts = enable_charger();
                        } */

                        //gps_statuses.gps_ready = FALSE;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
                    //ret_sts = GPS_WAIT;
#ifdef GPS_DEBUG
                    UWriteString((char*)"CBC:k", UART_PC);
#endif           
#ifdef DEBUG_GPS_INFO
                    UWriteString((char*)"\nCBC:",UART_PC);
                    UWriteString(tmpstr,UART_PC);
#endif       
                    timeout = 0;
                    gps_retry_cnt = 0;
                    //gps_statuses.NMEA_enabled = true;
                    /* gps_statuses.gga_sts = gga_pkt_parsing(tmpstr); */
                    cbc_pkt_parsing(tmpstr);
                    gps_statuses.got_VBATT = true;
/*                     //if(!strstr(tmpstr, "$GPGGA,,,,,,0,,,,,,,,*66"))
                    if(!strstr(tmpstr, ",,,,,0,,,,,,,,*66"))
                    {
                        gps_statuses.gga_sts = true;
                    } */
/* 
#ifdef DEBUG_GPS_PARSING
                    printf("\ngga_parse=%d", gga_pkt_parsing(&tmpstr));
#endif
 */
                    /* if(gps_statuses.gnss_sts)
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    } */
#ifdef COMBINE_GGA_GNSS_CMD
                    gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS; //PP added on 12-09-23 for testing GGA_GNSS_CMD
#else 
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START; 
                    gps_statuses.gps_handler_state = GPS_CMD_LOCATION; 
#endif
                    /* //if(!prev_charger_sts)
                    {
                        prev_charger_sts = enable_charger();
                    } */
                   // gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    //if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    if(timeout++ >= CBC_TIMEOUT)
                    {
#ifdef GPS_DEBUG
			            UWriteString((char*)"CBC:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_CBC;
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"CBC:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;

                            /* //if(!prev_charger_sts)
                            {
                                prev_charger_sts = enable_charger();
                            } */

                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPS_CMD_GPGGA_START:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
            gps_statuses.NMEA_enabled = true;
#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: GGA1", UART_PC);
#endif            
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            //gps_statuses.gga_sts = false;

            UWriteString((char*)"AT+CGPSINFOCFG=1,1\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_GPGGA_START;
        }
        break;

        case GPS_RSP_GPGGA_START:
        {
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\nGGA1_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];			
			int num_bytes = 0;

/* #ifdef DEBUG_GPS_INFO
            
            UWriteString((char*)"\ni1=",UART_PC);
            UWriteInt(gprs_rx_buff.index, UART_PC);
            UWriteString((char*)"\nGGA:",UART_PC);	
            UWriteString(tmpstr,UART_PC);
#endif */
            
            char resp = check_string("\r\nOK\r\n", tmpstr,&num_bytes);

            switch (resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPS_DEBUG
				    UWriteString((char*)"GGA1:f", UART_PC);
#endif
				    gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;

                    gps_retry_cnt++;
                    if (gps_retry_cnt >= GPS_RETRY_CNT)
                    {
                        //gps_statuses.errcode = GPS_ERR_OFFSET + GPS_RSP_STATUS;
                        ret_sts = GPS_FAIL;
                        gps_retry_cnt = 0;
                        gps_statuses.gps_handler_state = GPS_AT_INIT;
                        gps_statuses.gps_ready = false;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef GPS_DEBUG
                    UWriteString((char*)"GGA1:k", UART_PC);
#endif         

#ifdef DEBUG_GPS_INFO           
                    UWriteString((char*)"\nGGA:",UART_PC);	
                    UWriteString(tmpstr,UART_PC);
                    UWriteString((char*)"\nGGA_i:",UART_PC);
                    UWriteInt(strlen(tmpstr), UART_PC);
#endif

                    timeout = 0;
                    gps_retry_cnt = 0;
                    //gps_statuses.NMEA_enabled = true;
                    gps_statuses.gga_sts = gga_pkt_parsing(tmpstr);

/*                     //if(!strstr(tmpstr, "$GPGGA,,,,,,0,,,,,,,,*66"))
                    if(!strstr(tmpstr, ",,,,,0,,,,,,,,*66"))
                    {
                        gps_statuses.gga_sts = true;
                    } */
/* 
#ifdef DEBUG_GPS_PARSING
                    printf("\ngga_parse=%d", gga_pkt_parsing(&tmpstr));
#endif
 */
                    /* if(gps_statuses.gnss_sts)
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    } */

                    if((gps_statuses.gnss_sts)&&(gps_statuses.gga_sts))   //might come here from GNSS or GGA STOP
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else if((!gps_statuses.gnss_sts)&&(gps_statuses.gga_sts)) //might come here from GNSS
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    }
                    else    //might come here from GNSS
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
#ifdef GPS_DEBUG
			            UWriteString((char*)"GGA1:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"GGA1:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPS_CMD_LOCATION:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: LCN", UART_PC);
#endif
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            gps_date_time.update_time_aval = false;

            //gps_statuses.gnss_sts = false;

            UWriteString((char*)"AT+CGNSSINFO\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_LOCATION;
        }
        break;

        case GPS_RSP_LOCATION:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\nGNS_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];			
			int num_bytes = 0;

/* #ifdef DEBUG_GPS_INFO
            UWriteString((char*)"\ni2=",UART_PC);
            UWriteInt(gprs_rx_buff.index, UART_PC);
            UWriteString((char*)"\nGNS:",UART_PC);	
            UWriteString(tmpstr,UART_PC);
#endif */

            char resp = check_string("+CGNSSINFO: ", tmpstr,&num_bytes);

            switch (resp)
            {
                case GPRS_MATCH_FAIL:
                {
                    set_system_state(SYS_GPS_DATE, HIGH);
#ifdef GPS_DEBUG
				    UWriteString((char*)"LCN:f", UART_PC);
#endif
				    gps_statuses.gps_handler_state = GPS_CMD_LOCATION;

                    gps_retry_cnt++;
                    if (gps_retry_cnt >= GPS_RETRY_CNT)
                    {
                        //gps_statuses.errcode = GPS_ERR_OFFSET + GPS_RSP_STATUS;
                        ret_sts = GPS_FAIL;
                        gps_retry_cnt = 0;
                        gps_statuses.gps_handler_state = GPS_AT_INIT;
                        gps_statuses.gps_ready = false;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef GPS_DEBUG
                    UWriteString((char*)"LCN:k", UART_PC);
#endif                  

#ifdef DEBUG_GPS_INFO
                    UWriteString((char*)"\nGNS:",UART_PC);	
                    UWriteString(tmpstr,UART_PC);
                    UWriteString((char*)"\nGNS_i:",UART_PC);
                    UWriteInt(strlen(tmpstr), UART_PC);
#endif                    

                    timeout = 0;
                    gps_retry_cnt = 0;
                    gps_statuses.gnss_sts = gnss_pkt_parsing(tmpstr);

                    if(gps_statuses.gnss_sts)
                    {
#ifdef DEBUG_GET_LOC
                        printf("\ng_D1:%s",gps.gps_info.gns_info.date);
                        printf("\ng_T1:%s",gps.gps_info.gns_info.utc_time);
#endif

                        updateGpsDateTimeToBuff(&gps_date_time);

#ifdef DEBUG_GET_LOC
                        printf("\ng_D2:%02d-%02d-%02d",gps_date_time.dd,gps_date_time.mm,gps_date_time.yy);
                        printf("\ng_T2:%02d:%02d:%02d",gps_date_time.hr,gps_date_time.min,gps_date_time.sec);
#endif
                    }

                    /* if(gps_statuses.gga_sts)
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
                    } */
                    if((gps_statuses.gga_sts)&&(gps_statuses.gnss_sts))   //might come here from GGA start
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else if((!gps_statuses.gga_sts)&&(gps_statuses.gnss_sts)) //might come here from GGA stop
                    {
                        //ret_sts = GPS_WAIT;
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
                    }
                    else if((gps_statuses.gga_sts) && (!gps_statuses.gnss_sts))   //might come here from GGA start
                    {
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                    }
                    else    //might come here from GGA stop or CBC
                    {
                        //ret_sts = GPS_WAIT;
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
                    }

/* 
                    //if(!strstr(tmpstr, "+CGNSSINFO: ,,,,,,,,,,,,,,,"))
                    if(!strstr(tmpstr, ",,,,,,,,,,,,,,,"))
                    {
                        //gps_statuses.gga_sts = true; //estupida!!
                        gps_statuses.gnss_sts = true;
                    } */
/* 
#ifdef DEBUG_GPS_PARSING
                    printf("\ngnss_parse=%d", gnss_pkt_parsing(&tmpstr));
#endif
 */
                    //ret_sts = GPS_PASS;   //PP 06-09-23: trying new cmd seq, shifting this if GGA stop response is matched.
                    //gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;

                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;

                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
                        set_system_state(SYS_GPS_DATE, HIGH);
#ifdef GPS_DEBUG
			            UWriteString((char*)"LCN:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_LOCATION;
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"LCN:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }
                }
                break;
            
                default:
                break;
            }
        }
        break;

        case GPS_CMD_GPGGA_STOP:
        {
            ret_sts = GPS_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
		    UWriteString((char *)"cmd: GGA2", UART_PC);
#endif
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            UWriteString((char*)"AT+CGPSINFOCFG=0,1\r\n", GPRS_UART);
#ifdef GPS_DEBUG
		    UWriteString((char *)gprs_tx_buff.buffer, UART_PC);
#endif
		    gps_statuses.gps_handler_state = GPS_RSP_GPGGA_STOP;
        }
        break;

        case GPS_RSP_GPGGA_STOP:
        {
#ifdef DEBUG_MILLIS
            printf("\nS1=%d,t=%lld",gps_statuses.gps_handler_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPS_DEBUG
            UWriteString((char*)"\nGGA2_r:", UART_PC);
            UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
            switch (check_string_nobuf("OK"))
		    {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPS_DEBUG
			        UWriteString((char *)"GGA2:f", UART_PC);
#endif
                    gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;

                    if (gps_retry_cnt++ >= GPS_RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        ret_sts = GPS_FAIL;
                        
                        gps_statuses.gps_handler_state = GPS_AT_INIT; /*GPRS_CONNCT_RESET*/			//
                        timeout = 0;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef GPS_DEBUG
                    UWriteString((char*)"GGA2:k", UART_PC);
#endif                  
                    timeout = 0;
                    gps_retry_cnt = 0;
                    gps_statuses.NMEA_enabled = false;

                    //GPIO.out &= ~(1<<ESP_CAN_TX);

                    //ret_sts = GPS_PASS;   
                    if(gps_statuses.gga_sts && gps_statuses.gnss_sts)
                    {
                        set_system_state(SYS_GPS_PASS, LOW);
                        gps_statuses.gga_sts = false;
                        gps_statuses.gnss_sts = false;
                        ret_sts = GPS_PASS;
                        gps_statuses.gps_ready = true;
#ifndef COMBINE_GGA_GNSS_CMD                        
                        gps_statuses.gps_handler_state = GPS_CMD_CBC;  //Vinay added on 26-09-23
#endif  
//#ifdef GPS_DEBUG
#ifdef DEBUG_GGA_GNSS_COMB
                        UWriteString((char*)"\nGPS_OK",UART_PC);
#endif
                    }
                    //else if(!gps_statuses.gnss_sts && gps_statuses.gga_sts)
                    else if(!gps_statuses.gga_sts && gps_statuses.gnss_sts)
                    {
#ifdef DEBUG_GGA_GNSS_COMB
                        UWriteString((char*)"\n1GGA,GNSS=",UART_PC);
                        UWriteInt(gps_statuses.gga_sts, UART_PC);
                        UWriteData(',',UART_PC);
                        UWriteInt(gps_statuses.gnss_sts,UART_PC);
                        //printf("\nGGA,GNSS=%d,%d",gps_statuses.gga_sts,gps_statuses.gnss_sts);
#endif

#ifndef COMBINE_GGA_GNSS_CMD
                       // gps_statuses.gps_handler_state = GPS_CMD_LOCATION; //PP commented on 12-09-23 for testing GGA_GNSS_CMD
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START; //PP commented on 12-09-23 for testing GGA_GNSS_CMD
#endif
                    }
                    else    //either gga is not available or both are'nt available
                    {
                        ret_sts = GPS_WAIT;
#ifdef DEBUG_GGA_GNSS_COMB
                        UWriteString((char*)"\n2GGA,GNSS=",UART_PC);
                        UWriteInt(gps_statuses.gga_sts, UART_PC);
                        UWriteData(',',UART_PC);
                        UWriteInt(gps_statuses.gnss_sts,UART_PC);
                        //printf("\nGGA,GNSS=%d,%d",gps_statuses.gga_sts,gps_statuses.gnss_sts);
#endif
                        
#ifndef COMBINE_GGA_GNSS_CMD                        
                        //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;  //PP commented on 12-09-23 for testing GGA_GNSS_CMD
                        gps_statuses.gps_handler_state = GPS_CMD_CBC;  //Vinay added on 26-09-23
#endif
                    }
                    //ret_sts = GPS_PASS;
                    //gps_statuses.gps_handler_state = GPS_CMD_GPGGA_START;
#ifdef COMBINE_GGA_GNSS_CMD
                    gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS; //PP added on 12-09-23 for testing GGA_GNSS_CMD
#endif
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
#ifdef GPS_DEBUG
			            UWriteString((char*)"GGA2:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef GPS_DEBUG
			                UWriteString((char*)"GGA2:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

#ifdef COMBINE_GGA_GNSS_CMD
        case GPS_CMD_GGA_GNSS:
        {
            gps_statuses.NMEA_enabled = true;
#ifdef DEBUG_GGA_GNSS_COMB
		    UWriteString((char *)"cmd: GLCN", UART_PC);
#endif
            flushRxBuffer(GPRS_UART);
			flushTxBuffer(GPRS_UART);

            UWriteString((char*)"AT+CGPSINFOCFG=1,1;+CGNSSINFO\r\n", GPRS_UART);
		    gps_statuses.gps_handler_state = GPS_RSP_GGA_GNSS;
        }
        break;

        case GPS_RSP_GGA_GNSS:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];			
			int num_bytes = 0;

            memset(tmpstr, 0, sizeof(tmpstr));  //PP added on: 13-09-23 for testing

            //char resp = check_string("+CGNSSINFO: ", tmpstr,&num_bytes);
            char resp = check_string("\r\n+", tmpstr,&num_bytes);

#ifdef DEBUG_GGA_GNSS_COMB
            // UWriteString((char *)"tmpstr:", UART_PC);
            // UWriteString(tmpstr, UART_PC);
            UWriteString((char *)"t_i:", UART_PC);
            UWriteInt(strlen(tmpstr), UART_PC);
#endif

            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GGA_GNSS_COMB
			        UWriteString((char *)"GLCN:f", UART_PC);
#endif
                    gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS;

                    if (gps_retry_cnt++ >= GPS_RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        ret_sts = GPS_FAIL;
                        
                        gps_statuses.gps_handler_state = GPS_AT_INIT; /*GPRS_CONNCT_RESET*/			//
                        timeout = 0;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef DEBUG_GGA_GNSS_COMB
			        UWriteString((char *)"GLCN:k", UART_PC);
#endif
                    timeout = 0;
                    gps_retry_cnt = 0;
                    //gps_statuses.NMEA_enabled = true;

                    /* if(!strstr(tmpstr, "CGNSSINFO: ,,,,,,,,,,,,,,,"))
                    //if(!strstr(tmpstr, ",,,,,,,,,,,,,,,"))
                    {
                        //gps_statuses.gga_sts = true; //estupida!!
                        gps_statuses.gnss_sts = true;
                    }

                    if(!strstr(tmpstr, "$GPGGA,,,,,,0,,,,,,,,*66"))
                    //if(!strstr(tmpstr, ",,,,,0,,,,,,,,*66"))
                    {
                        gps_statuses.gga_sts = true;
                    } */

                    gps_statuses.gnss_sts = gnss_pkt_parsing(tmpstr);

                    char *p;
                    p = strstr(tmpstr, "$GPGGA,");
                    if(p)
                    {
                        gps_statuses.gga_sts = gga_pkt_parsing(p);
                    }

                    gps_statuses.gps_handler_state = GPS_CMD_GPGGA_STOP;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GGA_GNSS_TIMEOUT)
                    {
#ifdef DEBUG_GGA_GNSS_COMB
			            UWriteString((char*)"GLCN:t1", UART_PC);
#endif
                        timeout = 0;
                        gps_statuses.gps_handler_state = GPS_CMD_GGA_GNSS;
                        //flushRxBuffer(GPRS_UART);
                        if(gps_retry_cnt++ >= GPS_RETRY_CNT)
                        {
#ifdef DEBUG_GGA_GNSS_COMB
			                UWriteString((char*)"GLCN:t2", UART_PC);
#endif
                            gps_retry_cnt = 0;
                            ret_sts = GPS_FAIL;
                            gps_statuses.gps_handler_state = GPS_AT_INIT;
                        }
                    }
                }
                break;

                default:
                break;
            }

        }
        break;
#endif
        default:
        break;
    }

    return ret_sts;
}

#if 1
bool gga_pkt_parsing(char * tmpstr)
{
    //reference: $GPGGA,115610.00,2615.042797,N,07300.430485,E,1,06,1.2,198.5,M,-46.0,M,,*44\r\n

    uint8_t cnt = 0,  i = 0, j = 0, k = 0;
	char tmp[4] = {0};

#ifdef DEBUG_GPS_PARSING
    printf("\nGGA to parse:\n%s",tmpstr);
#endif
    
    if(count_comma(tmpstr) != 14)
	{
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str1");
#endif	
		return 0;	
	}

    char *r;
    r = strstr(tmpstr, "*");
    
    if(r)
    {   
		//r++;   
		//printf("ptr : %c",*r);  
        
		//*(r++); //skips '*' before checksum.
        //*r += 1; //skips '*' before checksum.
        //*r = (&*r + 1); //skips '*' before checksum.
        //r[k++];
        k++;
        
        memset(tmp, 0, sizeof(tmp));
        //for(cnt = 0; ((*r != '\r')&&(cnt < 2)); cnt++)
        for(cnt = 0; ((r[k] != '\r')&&(cnt < 2)); cnt++)
        //for(cnt = 0; ((*(r) != '\r')&&(cnt < 2)); cnt++)
        {
            //tmp[cnt] = *r++;
            tmp[cnt] = r[k++];
            //tmp[cnt] = *(r++);
#ifdef DEBUG_GPS_PARSING
            printf("\ntmp[%d]=%c", cnt,tmp[cnt]);
#endif            
        }
        
#ifdef DEBUG_GPS_PARSING
        printf("\ntmp1=%s",tmp);
#endif
        convertAsciiToHex(tmp, sizeof(tmp));
        
#ifdef DEBUG_GPS_PARSING
	    printf("\ntmp2=%s",tmp);
#endif
        memcpy(&gps.gps_info.gga_info.NMEA_gga_chk, tmp, sizeof(unsigned char));
        
#ifdef DEBUG_GPS_PARSING
        printf("\nNMEA_gga_chk_r = %X", gps.gps_info.gga_info.NMEA_gga_chk);
#endif
        if(gps.gps_info.gga_info.NMEA_gga_chk != calculate_gga_checksum(&tmpstr[0]))
	    {
#ifdef DEBUG_GPS_PARSING
            printf("\nNMEA_gga_chk_c = %X", calculate_gga_checksum(&tmpstr[0]));
#endif
		    return 0;
	    }
	    else if(gps.gps_info.gga_info.NMEA_gga_chk == 0x66)
	    {
#ifdef  DEBUG_GPS_PARSING
            printf("\nNULL_str");
#endif
            return 0;
	    }
    }
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str2");
#endif
        return 0;
    }
    
    for (;tmpstr[i++] != ',';){};	//skips  NMEA msg ID
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips  utc time
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips  latitude
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif    

    for (;tmpstr[i++] != ',';){};	//skips latitude indicator
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif
	
    for (;tmpstr[i++] != ',';){};	//skips longitude
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips longitude indicator
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    if(tmpstr[i] == '0')
    {
#ifdef DEBUG_GPS_PARSING
    	printf("no fix");
#endif
        return 0;
    }
    else
    {
#ifdef DEBUG_GPS_PARSING
        //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif        
    }

    for (;tmpstr[i++] != ',';){};	//skips fix status's comma after checking it
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
    memset(tmp, 0, sizeof(tmp));
	for(cnt = 0; (tmpstr[j] != ',' && j < i + 2); cnt++)
	{
#ifdef DEBUG_GPS_PARSING
		//printf("\n2tmpstr[%d]=%c", j,tmpstr[j]);
#endif
		tmp[cnt] = tmpstr[j++];
	}
#ifdef DEBUG_GPS_PARSING
	//printf("\ntmp=%s",tmp);
#endif
	
	//memset((void *)&gps.gps_info.gga_info, 0, sizeof(gga_info_t));	//only clearing it after all the error checking is done and no reason to return 0
	
	if((cnt >= 2) && (tmp[0] != '\0'))
	{
		gps.gps_info.gga_info.quantity_of_gps = 0;
		gps.gps_info.gga_info.quantity_of_gps = atoi(tmp);
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str3");
#endif
        return 0;
    }
	
#ifdef DEBUG_GPS_PARSING  
	//printf("\nsat_tmp=")  ;
	//printBytes((unsigned char*)tmp,cnt);
	//printf("\ncnt=%d",cnt);
	printf("\nsat = %d", gps.gps_info.gga_info.quantity_of_gps);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after number of satellites
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips HDOP
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
    for (cnt = 0; ((tmpstr[j++] != ',') && (j <= i + sizeof(gps.gps_info.gga_info.MSL_altitude))); cnt++){};
    
    if(cnt)
    {
    	memset(gps.gps_info.gga_info.MSL_altitude,0,sizeof(gps.gps_info.gga_info.MSL_altitude));
    	memcpy ((void *) &gps.gps_info.gga_info.MSL_altitude, (void *) &tmpstr[i], cnt);		
    }
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str4");
#endif
        return 0;
    }
    
#ifdef DEBUG_GPS_PARSING    
	printf("\nMSL_alt=%s",gps.gps_info.gga_info.MSL_altitude);
	//printf("\ncnt=%d",cnt);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after MSL altitude
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips MSL altitude's unit (usually 'M' meaning meters)
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
    for (cnt = 0; ((tmpstr[j++] != ',') && (j <= (i + sizeof(gps.gps_info.gga_info.Geoid_separation)))); cnt++){};	//sometimes we don't recieve full string even if we recieve all the commas
    
	if(cnt)
	{
		memset(gps.gps_info.gga_info.Geoid_separation, 0, sizeof(gps.gps_info.gga_info.Geoid_separation));
		memcpy ((void *) &gps.gps_info.gga_info.Geoid_separation, (void *) &tmpstr[i], cnt);			
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str5");
#endif
        return 0;
    }

#ifdef DEBUG_GPS_PARSING
    printf("\nGeoid=%s",gps.gps_info.gga_info.Geoid_separation);
    //printf("\ncnt=%d",cnt);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

/*
    for (;tmpstr[i++] != ',';){};	//skips comma after Geoid separation
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips Geoid separation's unit (usually 'M' meaning meters)
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif
*/
    return 1;

}

bool cbc_pkt_parsing(char * tmpstr)
{
    uint8_t cnt = 0,  i = 0, j = 0;
	char tmp[6] = {0};

    for (i=0; tmpstr[i]!='V'; i++)
    {
        tmp[i] =  tmpstr[i];
    }
    tmp[i] = '\0';
    ram_data.v_batt= ((atof(tmp)+0.35)*1000);
#ifdef DEBUG_GPS_INFO
    //printf("\nv_batt=%d",ram_data.v_batt);
#endif
    return 1;
}

bool gnss_pkt_parsing(char * tmpstr)
{
	//reference: 2,06,04,02,2615.042797,N,07300.430485,E,090923,115610.0,198.5,0.0,,1.5,1.2,0.9\r\n\r\nOK\r\n
	
	uint8_t cnt = 0,  i = 0, j = 0;
	char tmp[4] = {0};

#ifdef DEBUG_GPS_PARSING
    printf("\nGNSS to parse:\n%s",tmpstr);
#endif

#ifdef  COMBINE_GGA_GNSS_CMD
    for (;tmpstr[i++] != ' ';){};	//skips  "CGNSSINFO: "
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif
#endif

    if((tmpstr[i] < '2') || !isdigit(tmpstr[i]) || (count_comma(tmpstr) != 15))
	{
		return 0;
	}

    for (;tmpstr[i++] != ',';){};	//skips  GPS satellites
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips  GLONASS satellites
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips  BEIDOU satellites
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

	for (;tmpstr[i++] != ',';){};	//skips  BEIDOU satellites
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

	//memset((void *)&gps.gps_info.gns_info, 0, sizeof(gns_info_t));	//only clearing it after all the error checking is done and no reason to return 0

#ifdef DEBUG_GPS_PARSING
    //printf("La");
#endif
	j = i;
    for (cnt = 0; ((tmpstr[j++] != ',') && (j < i + LAT_LEN)); cnt++){};
    
    if(cnt >= 11)
    {
    	memset(gps.gps_info.gns_info.latitude,0,LAT_LEN);
    	memcpy ((void *) &gps.gps_info.gns_info.latitude, (void *) &tmpstr[i], cnt);
    }
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str1");
#endif
        return 0;
    }
			
#ifdef DEBUG_GPS_PARSING
	//printf("\nLat_temp=");
	//printBytes((unsigned char*)&tmpstr[i],cnt);
	//printf("\ncnt=%d",cnt);
    printf("\nLat=%s",gps.gps_info.gns_info.latitude);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after latitude
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

	if((tmpstr[i] != '\0') && (tmpstr[i] != ','))
	{
		gps.gps_info.gns_info.N_S = 0;
		gps.gps_info.gns_info.N_S = tmpstr[i];
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str2");
#endif
        return 0;
    }
    
#ifdef DEBUG_GPS_PARSING
	printf("\nN_S=%c",gps.gps_info.gns_info.N_S);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after latitude indicator
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif    

    j = i;
    for (cnt = 0; ((tmpstr[j++] != ',') && j < i + LONG_LEN); cnt++){};
    
    if(cnt >= 12)
    {
    	memset(gps.gps_info.gns_info.longitude,0,LONG_LEN);
    	memcpy ((void *) &gps.gps_info.gns_info.longitude, (void *) &tmpstr[i], cnt);
    }
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str3");
#endif
        return 0;
    }
    	
#ifdef DEBUG_GPS_PARSING
	//printf("\nLong_temp=");
	//printBytes((unsigned char*)&tmpstr[i],cnt);
	//("\ncnt=%d",cnt);
    printf("\nLong=%s",gps.gps_info.gns_info.longitude);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif 

   for (;tmpstr[i++] != ',';){};	//skips comma after longitude   //why give error only on this line??
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif   

	if((tmpstr[i] != '\0') && (tmpstr[i] != ','))
	{
		gps.gps_info.gns_info.E_W = 0;
		gps.gps_info.gns_info.E_W = tmpstr[i];		
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str4");
#endif
        return 0;
    }

#ifdef DEBUG_GPS_PARSING
	printf("\nE_W=%c",gps.gps_info.gns_info.E_W);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after longitude indicator
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif  

	j = i;
	for(cnt = 0;((tmpstr[j++] != ',') && (j <= i + MAX_DATE_SIZE)); cnt++){};
	
	if(cnt >= MAX_DATE_SIZE)
	{
		memset(gps.gps_info.gns_info.date, 0, MAX_DATE_SIZE+1);
		memcpy((void *)&gps.gps_info.gns_info.date[0], (void *)&tmpstr[i], MAX_DATE_SIZE);
		
		
		i += MAX_DATE_SIZE;
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str5");
#endif
        return 0;
    }
	
#ifdef DEBUG_GPS_PARSING
    printf("\nDate:%s",gps.gps_info.gns_info.date);
	//printf("\nd_cnt:%d", cnt);
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after date
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

	j = i;
	for(cnt = 0;((tmpstr[j++] != ',') && (j <= i + MAX_TIME_SIZE + 1)); cnt++){};	//j < i + MAX_TIME_SIZE + 1 even tho there r 2 characters after utc time becoz i is already at the 1st digit of utc_time
	
	if(cnt >= MAX_TIME_SIZE)	//6 digits + '.' & '0' 2 characters after 6 digits of utc time.
	{
		memset(gps.gps_info.gns_info.utc_time, 0, MAX_TIME_SIZE+1);
		memcpy((void *)&gps.gps_info.gns_info.utc_time[0], (void *)&tmpstr[i], MAX_TIME_SIZE);
					

		
		i += MAX_TIME_SIZE;;
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str6");
#endif
        return 0;
    }
	
#ifdef DEBUG_GPS_PARSING
    printf ("\nTime:%s",gps.gps_info.gns_info.utc_time);
	//printf("\nt_cnt:%d", cnt);
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

/*
    memcpy ((void *) &gps.gps_info.gns_info.date[0], (void *) &tmpstr[i], MAX_DATE_SIZE);
#ifdef DEBUG_GPS_PARSING
    printf("\nDate:%s",gps.gps_info.gns_info.date);
#endif

    i += MAX_DATE_SIZE;

#ifdef DEBUG_GPS_PARSING
    printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after date
#ifdef DEBUG_GPS_PARSING
    printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    memcpy ((void *) &gps.gps_info.gns_info.utc_time[0], (void *) &tmpstr[i], MAX_TIME_SIZE);
#ifdef DEBUG_GPS_PARSING
    printf ("\nTime:%s",gps.gps_info.gns_info.utc_time);
#endif

    i += MAX_TIME_SIZE;

#ifdef DEBUG_GPS_PARSING
    printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif
*/

    for (;tmpstr[i++] != ',';){};	//skips comma after utc time
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
    for (cnt = 0; ((tmpstr[j++] != ',') && (j <= i + sizeof(gps.gps_info.gns_info.MSL_altitude))); cnt++){};
    
    if(cnt >= 5)
    {
    	memset(gps.gps_info.gns_info.MSL_altitude, 0, sizeof(gps.gps_info.gns_info.MSL_altitude));
    	memcpy ((void *) &gps.gps_info.gns_info.MSL_altitude, (void *) &tmpstr[i], cnt);
    }
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str7");
#endif
        return 0;
    }
        	
#ifdef DEBUG_GPS_PARSING
	printf("\nMSL_alt=%s",gps.gps_info.gns_info.MSL_altitude);
	//printf("\ncnt=%d",cnt);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after MSL_altitude
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
	for(cnt = 0; ((tmpstr[j++] != ',') && (j <= i + sizeof(gps.gps_info.gns_info.sog)));cnt++){};
		
	if(cnt >= 3)
	{
		memset(gps.gps_info.gns_info.sog, 0, sizeof(gps.gps_info.gns_info.sog));
		memcpy((void *)&gps.gps_info.gns_info.sog[0], (void *)&tmpstr[i], cnt);
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str8");
#endif
        return 0;
    }
	
#ifdef DEBUG_GPS_PARSING
	printf("\nsog=%s",gps.gps_info.gns_info.sog);
	//("\ncnt=%d",cnt);
   // printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips comma after speed
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    for (;tmpstr[i++] != ',';){};	//skips cog
#ifdef DEBUG_GPS_PARSING
    //printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
	for(cnt = 0;((tmpstr[j++] != ',') && (j <= (i + sizeof(gps.gps_info.gns_info.pdop))));cnt++){};	//sometimes we don't recieve full string even if we recieve all the commas
	
	if(cnt >= 3)
	{
		memset(gps.gps_info.gns_info.pdop, 0, sizeof(gps.gps_info.gns_info.pdop));
		memcpy((void *)&gps.gps_info.gns_info.pdop[0], (void *)&tmpstr[i], cnt);
	}
    else
    {
#ifdef  DEBUG_GPS_PARSING
        printf("\nincomplt_str9");
#endif
        return 0;
    }
	
#ifdef DEBUG_GPS_PARSING
	printf("\npdop=%s",gps.gps_info.gns_info.pdop);
	//printf("\ncnt=%d",cnt);
    //printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif

/*
    for (;tmpstr[i++] != ',';){};	//skips comma after pdop
#ifdef DEBUG_GPS_PARSING
    printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
	for(cnt = 0;tmpstr[j++] != ',';cnt++){};
	memcpy((void *)&gps.gps_info.gns_info.hdop[0], (void *)&tmpstr[i], cnt);
#ifdef DEBUG_GPS_PARSING
	printf("\nhdop=%s",gps.gps_info.gns_info.hdop);
    printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif 

    for (;tmpstr[i++] != ',';){};	//skips comma after hdop
#ifdef DEBUG_GPS_PARSING
    printf("\ntmpstr[%d]=%c", i,tmpstr[i]);
#endif

    j = i;
	//for(cnt = 0;tmpstr[j++] != ',';cnt++){};
	for(cnt = 0;((tmpstr[j++] != '\r') && (j < (i + sizeof(gps.gps_info.gns_info.vdop))));cnt++){};
	memcpy((void *)&gps.gps_info.gns_info.vdop[0], (void *)&tmpstr[i], cnt);
#ifdef DEBUG_GPS_PARSING
	printf("\nvdop=%s",gps.gps_info.gns_info.vdop);
    printf("\ntmpstr[%d]=%s", i,&tmpstr[i]);
#endif
*/
    return 1;
}

unsigned char calculate_gga_checksum(char *sentence) 
{
  unsigned char checksum = 0;
  for (int i = 1; sentence[i] != '*'; i++) 
  {
    checksum ^= sentence[i];
  }
#ifdef DEBUG_GPS_PARSING
    printf("\nchk=%X", checksum);
#endif
  return checksum;
}

#endif

void updateGpsDateTimeToBuff(gps_date_time_t *date_time)
{
    gps_date_time_t temp_time;
#ifdef GNS_PKT_EN
	temp_time.yy = (((gps.gps_info.gns_info.date[2] -'0') * 10) + ((gps.gps_info.gns_info.date[3] - '0')));
	temp_time.mm = (((gps.gps_info.gns_info.date[4] -'0') * 10) + ((gps.gps_info.gns_info.date[5] - '0')));
	temp_time.dd = (((gps.gps_info.gns_info.date[6] - '0')* 10) + ((gps.gps_info.gns_info.date[7] - '0')));
#else
	temp_time.dd = (((gps.gps_info.gns_info.date[0] -'0') * 10) + ((gps.gps_info.gns_info.date[1] - '0')));
	temp_time.mm = (((gps.gps_info.gns_info.date[2] -'0') * 10) + ((gps.gps_info.gns_info.date[3] - '0')));
	temp_time.yy = (((gps.gps_info.gns_info.date[4] - '0')* 10) + ((gps.gps_info.gns_info.date[5] - '0')));
#endif

#ifdef DEBUG_GET_LOC
		//printf("\ntemp_date1:%02d-%02d-%02d",temp_time.dd,temp_time.mm,temp_time.yy);		
#endif

	temp_time.hr = (((gps.gps_info.gns_info.utc_time[0] - '0') * 10) + ((gps.gps_info.gns_info.utc_time[1] - '0')));
	temp_time.min = (((gps.gps_info.gns_info.utc_time[2] - '0') * 10) + ((gps.gps_info.gns_info.utc_time[3] - '0')));
	temp_time.sec = (((gps.gps_info.gns_info.utc_time[4] - '0') * 10) + ((gps.gps_info.gns_info.utc_time[5] - '0')));

#ifdef DEBUG_GET_LOC
		//printf("\ntemp_time1:%02d:%02d:%02d",temp_time.hr,temp_time.min,temp_time.sec);		
#endif

    if (((temp_time.dd <= 0) || (temp_time.dd > 31)) ||
	((temp_time.mm <= 0) || (temp_time.mm > 12)) ||
	((temp_time.yy < (DEFAULT_YEAR%100)) || (temp_time.yy > (DEFAULT_YEAR%100) + YEAR_OFFSET))||			// Assuming that RTC will never go below 2016.
	((temp_time.hr < 0) || (temp_time.hr > 23)) ||
	((temp_time.min < 0) || (temp_time.min > 59)) ||
	((temp_time.sec < 0) || (temp_time.sec > 59)))
    {
        set_system_state(SYS_GPS_DATE, HIGH);
    }
    else
    {
        utcTOlocal(&temp_time);
        if (((temp_time.yy >= (DEFAULT_YEAR%100)) && (temp_time.yy <= (DEFAULT_YEAR%100) + YEAR_OFFSET)) &&
		((temp_time.mm >= 1) && (temp_time.mm <= 12)) &&
		((temp_time.dd >= 1) && (temp_time.dd <= 31)) &&
		((temp_time.hr >= 0) && (temp_time.hr <= 23)) &&
		((temp_time.min >= 0) && (temp_time.min <= 59)) &&
		((temp_time.sec >= 0) && (temp_time.sec <= 59)))
        {
            date_time->yy = temp_time.yy;
            date_time->mm = temp_time.mm;
            date_time->dd = temp_time.dd;
            date_time->hr = temp_time.hr;
            date_time->min = temp_time.min;
            date_time->sec = temp_time.sec;

            gps_date_time.update_time_aval = true;
            set_system_state(SYS_GPS_DATE, LOW);

    	 }
        else
        {
          set_system_state(SYS_GPS_DATE, HIGH);  
        }
    }
}

void utcTOlocal(gps_date_time_t *timeT) 
{
    int DayNum;
    
	timeT->hr += 5;
	timeT->min += 30;
	

    if(timeT->min > 59)
	{
		int m;
		m = (timeT->min / 60);
		timeT->hr = (timeT->hr + m);
		timeT->min = (timeT->min % 60);
	}
	if(timeT->hr > 23)
	{
		int h;
		h = (timeT->hr / 24);
		timeT->dd = (timeT->dd + h);
		timeT->hr = (timeT->hr % 24);
	}
	if((timeT->mm == 1) || (timeT->mm == 3) || (timeT->mm == 5) || (timeT->mm == 7) || (timeT->mm == 8) || (timeT->mm == 10) || (timeT->mm == 12))
	{
		DayNum = 31;
	}
	if((timeT->mm == 4) || (timeT->mm == 6) || (timeT->mm == 9) || (timeT->mm == 11))
	{
		DayNum = 30;
	}
	if(timeT->mm == 2)
	{
	    if ((timeT->yy % 4) == 0)
	    {
	        if((timeT->yy % 100) == 0)
	        {
	            if((timeT->yy % 400) == 0)
	            {
	                DayNum = 29;
	            }
	            else
        		{
        			DayNum = 28;
        		}
	        }
	        else
    		{
    			DayNum = 29;
    		}
	    }
        else
        {
            DayNum = 28;
        }
    
    }
    if(timeT->dd > DayNum)
	{
		int d;
		d = (timeT->dd / DayNum);
		timeT->mm = (timeT->mm + d);
		timeT->dd = (timeT->dd % DayNum);
	}
	if(timeT->mm > 12)
	{
		int M;
		M = (timeT->mm / 12);
		timeT->yy = (timeT->yy + M);
		timeT->mm = (timeT->mm % 12);
	}
}
