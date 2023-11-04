#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "esp_timer.h"

#include "gprs.h"
#include "mcp23017.h"
#include "config.h"
#include "pins.h"
#include "UART.h"
#include "app_comm.h"
#include "web_comm.h"
#include "clock.h"
#include "Common.h"
#include "_debug.h"
#include "error.h"
#include "I2C.h"
//#define GPRS_DEBUG

gprs_t gprs;
gprs_date_time_t gprs_date_time;

volatile gprs_tx_data_buff_t gprs_tx_buff;
volatile gprs_rx_data_buff_t gprs_rx_buff;
volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;   //16-2-19 VC: for dynamic allocation
volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

uint32_t sess_idle_strt = 0;

con_status_t http_handler(void)
{
    //char status = 0;
	con_status_t status = CON_IN_PRG;
	//static char readData = TRUE;
	//char fms_server_resp_buff[2];	// use #define for size of buffer
	//static char http_buff[GPRS_TX_BUFFER_MAX];
	//static gprs_http_upload_data_t upload_code = GPRS_NO_DATA;
	//static unsigned char upload_retry_cnt = 0;
	//static unsigned char http_conn_sts = false;
	static unsigned int gprs_conn_retry_time = 0;

    switch(gprs.state)
    {
        case GPRS_IDLE:
        {
#ifdef GPRS_DEBUG
            UWriteString((char*)"G_I", UART_PC);
#endif
            //check configuration in e2p later when we import that library
			set_system_state(SYS_GPRS_DATE, HIGH);
            gprs.state = GPRS_AT_INIT;
        }
        break;

        case GPRS_AT_INIT:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,t=%lld",gprs.state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
            UWriteString((char*)"G_AI", UART_PC);
#endif
            status = /* (char) */modem_initAT();
            if(status == CON_OK)
			{
				gprs.state = GPRS_CONNECT;
			}
			else if(status == CON_FAIL)
			{
				gprs.state = GPRS_IDLE;		//Lets go to idle and wait for trigger to re do
				//setGPRSConnSts(NOT_AVBL);
			}
			else if ((status == CON_IN_PRG) || (status == CON_WAIT))
			{
				//Nothing to do, keep going
			}
        }
        break;

        case GPRS_CONNECT:
        {
#ifdef GPRS_DEBUG
            UWriteString((char*)"G_C", UART_PC);
#endif
            status = /* (char) */gprs_connect();
            if(status == CON_OK)
			{
				//gprs.state = GPRS_AT_INIT;
				gprs.state = GPRS_CONN_STS;	//PP added on 08-08-23.
			}
			else if(status == CON_FAIL)
			{
				gprs.state = GPRS_IDLE;		//Lets go to idle and wait for trigger to re do
				//setGPRSConnSts(NOT_AVBL);
			}
			else if ((status == CON_IN_PRG) || (status == CON_WAIT))
			{
				//Nothing to do, keep going
			}
        }
        break;

        case GPRS_CONN_STS:
        {
            status = /* (char) */gprs_connect_status();
			if(status == CON_OK)
			{
				setGPRSConnSts(AVBL);

				/* if (check_unsent_log() || (getFdmState() == FDM_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)) // 29-1-2019 VC: added for gprs sync
				{
					gprs.state = GPRS_HTTP_INIT;
				}
				else */
				{
					sess_idle_strt = (esp_timer_get_time()/1000);
					gprs.state = GPRS_SESSION_IDLE;
				}
			}
			else if(status == CON_FAIL)
			{
				setGPRSConnSts(NOT_AVBL);
				gprs.state = GPRS_IDLE;		//Lets go to idle and wait for trigger to re do
			}
			else if ((status == CON_IN_PRG) || (status == CON_WAIT))
			{
				//Nothing to do, keep going
			}
        }
        break;

        case GPRS_SESSION_IDLE:
        {
			//status = CON_WAIT;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,t=%lld",gprs.state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"G_SI", UART_PC);
#endif
			// -- If not in dispense mode and configration pending -> ask user for config update
			//Check switch press
			//gprs.state = GPRS_CONFIG;

			uint32_t sess_diff = ((esp_timer_get_time()/1000) - sess_idle_strt);

			//if(gprs_conn_retry_time++ > GPRS_CONN_RETRY_TIME)
			if(sess_diff >= 30000U)
			{
#ifdef DEBUG_WC_MILLIS
                //printf("\nsess=%ld",sess_diff);
#endif
				//status = CON_IN_PRG;
				gprs_conn_retry_time = 0;
				gprs.state = GPRS_CONN_STS;	
			}
			// 13-2-19 VC: need to check for gprs_config mode
			/* if(check_unsent_log() || (getFdmState() == FDM_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG))		// Check for new dispense or refuel logs
			{
				//if (gprs.server_status)
				if (getGPRSConnSts() == AVBL)				// HJ 06-09-2017
				{
					//gprs.state = GPRS_HTTP_INIT;
					if (http_conn_sts == TRUE)
					{
						gprs.state = GPRS_PREPARE_LOG;			// HJ 06-09-2017
					}
					else
					{
						//gprs.state = GPRS_HTTP_INIT;			// HJ 06-09-2017
						gprs.state = GPRS_CONN_STS;
					}
				}
				else
				{
					gprs.state = GPRS_CONNECT;				// HJ 06-09-2017
				}
			} */	            
        }
        break;

        default:
        break;
    }
	return status;
}

con_status_t gprs_connect(void)
{
    con_status_t sts = CON_IN_PRG;
	static char gprs_retry_count = 0;
	static unsigned int timeout = 0;			//Can be used with all commands
	int num_byte = 0;

    static gprs_connct_state_t gprs_connct_state = GPRS_CONNCT_CMD_CPIN;

    switch(gprs_connct_state)
    {
        case GPRS_CONNCT_CMD_CPIN:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
        	UWriteString((char *)"cmd: CPIN", UART_PC);
#endif
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

			//setSIMSts(NOT_AVBL);

            UWriteString((char*)"AT+CPIN?\r\n", GPRS_UART);
            gprs_connct_state = GPRS_CONNCT_RSP_CPIN;
        }
        break;

        case GPRS_CONNCT_RSP_CPIN:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            switch (check_string_nobuf("+CPIN: READY"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
                    UWriteString((char *)"CPIN:f", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_CPIN;

                    if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs_retry_count = 0;

						set_system_state(SYS_GSM_SIM, HIGH);
						
						/* gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CFUN; */
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
						timeout = 0;
					}
                }
                break;

                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
                    UWriteString((char *)"CPIN:k", UART_PC);
#endif
                    //sts = CON_OK;
                    gprs_connct_state = GPRS_CONNCT_CMD_IMEI;
					gprs_retry_count = 0;
					timeout = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
                        UWriteString((char *)"CPIN: t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
                            UWriteString((char *)"CPIN: t2", UART_PC);
#endif
							set_system_state(SYS_GSM_SIM, HIGH);
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_AT; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_IMEI:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
            UWriteString((char *)"cmd: IMEI", UART_PC);
#endif
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

            UWriteString((char*)"AT+GSN\r\n",GPRS_UART);
            gprs_connct_state = GPRS_CONNCT_RSP_IMEI;
        }
        break;

        case GPRS_CONNCT_RSP_IMEI:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];
			char resp = check_string("\r\n", tmpstr, &num_byte);

            switch (resp)
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
                    UWriteString((char *)"IMEI:f", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_IMEI;
					gprs_retry_count++;
					if (gprs_retry_count >= RETRY_CNT) 
					{
						//gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_IMEI;
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;
						gprs_retry_count = 0;
					}
                }
                break;

                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
                    UWriteString((char *)"IMEI:k", UART_PC);
#endif
                    memcpy(gprs.imei, tmpstr, IMEI_LEN);
                    //flushRxBuffer(GPRS_UART);
#ifdef GPRS_DEBUG
                    UWriteString((char*)"gprs.imei =",UART_PC);
                    UWriteString(gprs.imei, UART_PC);
#endif

                    //gprs_connct_state = GPRS_CONNCT_CMD_CFUN; //PP 31-08-23: only for now
                    //gprs_connct_state = GPRS_CONNCT_CMD_CPIN; //PP 31-08-23: only for now
                    gprs_connct_state = GPRS_CONNCT_CMD_CFUN; 

                    //sts = CON_OK;   //PP 31-08-23: only for now

                    gprs_retry_count = 0;
					timeout = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
                        UWriteString((char *)"IMEI:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_IMEI;
						
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
                            UWriteString((char *)"IMEI:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_IMEI; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_CFUN:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CFUN", UART_PC);
#endif
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

            UWriteString((char *)"AT+CFUN?\r\n", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CFUN;
        }
        break;

        case GPRS_CONNCT_RSP_CFUN:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            switch (check_string_nobuf("+CFUN: 1"))
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CFUN:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CFUN;
			
			        if (gprs_retry_count++ >= RETRY_CNT)
					{
						gprs_retry_count = 0;
						
						/* gprs.module_status = NOT_AVBL;
						
						gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CFUN; */
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
						timeout = 0;
					}
                }
                break;

                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
					UWriteString((char *)"CFUN:k", UART_PC);
#endif					
					
					gprs_connct_state = GPRS_CONNCT_CMD_CREG;
					
					gprs_retry_count = 0;
					timeout = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CFUN:t1", UART_PC);
#endif
						timeout = 0;
					
						gprs_connct_state = GPRS_CONNCT_CMD_CFUN;
					
					    if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
							UWriteString((char *)"CFUN:t2", UART_PC);
#endif
							gprs_retry_count = 0;
						
							/* gprs.module_status = NOT_AVBL;
						
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CFUN; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_CREG:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CONN_STS",UART_PC);
#endif
			flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);
			
			UWriteString((char *)"AT+CREG?\r\n", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CREG;
        }
        break;

        case GPRS_CONNCT_RSP_CREG:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            switch (check_string_nobuf("+CREG: 0,1"))
		    {
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
                    UWriteString((char *)"CREG:f", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_CREG;
                
                    if (gprs_retry_count++ >= RETRY_CNT)
                    {
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CREG;
                        sts = CON_FAIL;
                    
                        //gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
                        gprs_retry_count = 0;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
                    UWriteString((char *)"CREG:k", UART_PC);
#endif
                    gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
                    gprs_retry_count = 0;
                    timeout = 0;                    
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef GPRS_DEBUG
                    //UWriteString((char *)"CREG:w", UART_PC);
#endif
                    if(timeout++ >= GPRS_ATE0_TIMEOUT)
                    {
#ifdef GPRS_DEBUG
						UWriteString((char *)"CREG:t1", UART_PC);
#endif
                        timeout = 0;
                        gprs_connct_state = GPRS_CONNCT_CMD_CREG;
                    
                        if (gprs_retry_count++ >= RETRY_CNT)
                        {
#ifdef GPRS_DEBUG
							UWriteString((char *)"CREG:t2", UART_PC);
#endif
                            /* gprs.module_status = NOT_AVBL;
                        
                            gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CREG; */
                            sts = CON_FAIL;
                        
                            //gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_CSQ:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"CSQ sent", UART_PC);
#endif
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

			UWriteString((char *)"AT+CSQ\r", GPRS_UART);
			//gprs.network_status = NOT_AVBL;				//Lets see if we have network
			setGPRSNWSts(NOT_AVBL);
			gprs_connct_state = GPRS_CONNCT_RSP_CSQ;
        }
        break;

        case GPRS_CONNCT_RSP_CSQ:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];
			char resp = check_string("+CSQ: ", tmpstr, &num_byte);

#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"RSP_CSQ:", UART_PC);
			UWriteString(tmpstr, UART_PC);
#endif
			switch (resp)
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CSQ:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
					//flushRxBuffer(GPRS_UART);
					gprs_retry_count++;
					if (gprs_retry_count >= RETRY_CNT)
					{
						//gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CSQ;
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;
						gprs_retry_count = 0;
					}
                }
                break;

                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
                    UWriteString((char *)"CSQ:k", UART_PC);
#endif						
                    //Lets update network strength 
                    gprs.network_strength = (tmpstr[0]-'0')*10 + (tmpstr[1]-'0');
                    
                    if((gprs.network_strength >= MIN_NETWORK_STRENGTH_DB) && (gprs.network_strength <= MAX_NETWORK_STRENGTH_DB))// network strength upto 
                        setGPRSNWSts(AVBL);
                    
                    gprs_connct_state = GPRS_CONNCT_CMD_CGATT;
                    //gprs_connct_state = GPRS_CONNCT_RSP_1_CSQ;
                    gprs_retry_count = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef GPRS_DEBUG
					//UWriteString((char *)"CSQ:w", UART_PC);
#endif					
					if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CSQ:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CSQ;
						
					    if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
							UWriteString((char *)"CSQ:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CSQ; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_CGATT:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CGATT", UART_PC);
#endif
            flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

			UWriteString((char *)"AT+CGATT?\r\n", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CGATT;
        }
        break;

        case GPRS_CONNCT_RSP_CGATT:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"RSP CGATT:\n", UART_PC);
#endif
			switch (check_string_nobuf("+CGATT: 1"))
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CG:f", UART_PC);
#endif							
					gprs_connct_state = GPRS_CONNCT_CMD_CGATT;
				
				    if (gprs_retry_count++ >= RETRY_CNT)
					{
						//gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CGATT;
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
						gprs_retry_count = 0;
					}
                }
                break;
                
                case GPRS_MATCH_OK:
                {
					sts = CON_WAIT;
#ifdef GPRS_DEBUG
					UWriteString((char *)"CG:k", UART_PC);
#endif					
                    gprs_retry_count = 0;
					timeout = 0;
                    gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef GPRS_DEBUG
					//UWriteString((char *)"CG:w", UART_PC);
#endif					
					if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CGATT:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CGATT;
					
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
							UWriteString((char *)"CGATT:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
					
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CGATT; */
							sts = CON_FAIL;
					
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

        case GPRS_CONNCT_CMD_CCLK:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CCLK", UART_PC);
#endif
			flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

            gprs_date_time.update_time_aval = false;
			UWriteString((char *)"AT+CCLK?\r\n", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CCLK;            
        }
        break;

        case GPRS_CONNCT_RSP_CCLK:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];
			gprs_date_time_t temp_date_time;
			char resp = check_string("+CCLK: \"", tmpstr, &num_byte);

#ifdef GPRS_DEBUG		//Anand 27-04-16
			UWriteString((char *)"RSP_CCLK1:", UART_PC);
			UWriteString(tmpstr, UART_PC);
#endif
            switch (resp)
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:f", UART_PC);
#endif					
					set_system_state(SYS_GPRS_DATE, HIGH);
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
					gprs_retry_count++;
					if (gprs_retry_count >= RETRY_CNT) 
					{
						//gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						gprs_connct_state = GPRS_CONNCT_CMD_CPIN;
						gprs_retry_count = 0;
					}
                }   
                break;

                case GPRS_MATCH_OK:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:k", UART_PC);
#endif						
					//Lets update time
					//if (check_date_time(tmpstr, (char *)&gprs_date_time))
					if (check_date_time(tmpstr, (char *)&temp_date_time))
					{
#ifdef GPRS_DEBUG
						UWriteString((char*)"Leave Time", UART_PC);
						UWriteInt(temp_date_time.yy, UART_PC);
						UWriteInt(temp_date_time.mm, UART_PC);
						UWriteInt(temp_date_time.dd, UART_PC);
						UWriteInt(temp_date_time.hr, UART_PC);
						UWriteInt(temp_date_time.min, UART_PC);
						UWriteInt(temp_date_time.sec, UART_PC);
#endif
						if (((temp_date_time.yy >= (DEFAULT_YEAR%100)) && (temp_date_time.yy <= ((DEFAULT_YEAR%100) + YEAR_OFFSET))) && 			// Assuming that RTC will never go below 2020.
						((temp_date_time.mm >= 1) && (temp_date_time.mm <= 12)) &&
						((temp_date_time.dd >= 1) && (temp_date_time.dd <= 31))&&
						((temp_date_time.hr >= 0) && (temp_date_time.hr <= 23)) &&
						((temp_date_time.min >= 0) && (temp_date_time.min <= 59)) &&
						((temp_date_time.sec >= 0) && (temp_date_time.sec <= 59)))
						{
							set_system_state(SYS_GPRS_DATE, LOW);
#ifdef GPRS_DEBUG
							UWriteString((char*)"TIME OK", UART_PC);
#endif
							memcpy(&gprs_date_time, &temp_date_time, sizeof(gprs_date_time_t));
							gprs_date_time.update_time_aval = true;
						}
						else
						{
							set_system_state(SYS_GPRS_DATE, HIGH);
#ifdef GPRS_DEBUG
							UWriteString((char*)"TIME Not OK", UART_PC);
#endif
						}
					}
					sts = CON_OK;
					gprs_connct_state = GPRS_CONNCT_CMD_CPIN;
					gprs_retry_count = 0;
                }   
                break;

                case GPRS_NO_NEW_MSG:
                {
					set_system_state(SYS_GPRS_DATE, HIGH);
#ifdef GPRS_DEBUG
					//UWriteString((char *)"CCLK:w", UART_PC);
#endif					
					if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CCLK:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
						
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
							UWriteString((char *)"CCLK:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							gprs_connct_state = GPRS_CONNCT_CMD_CPIN;			//
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

con_status_t gprs_connect_status(void)
{
    con_status_t sts = CON_IN_PRG;

    static char gprs_retry_count = 0;
	static unsigned int timeout = 0;			//Can be used with all commands
	int num_byte = 0;

    static gprs_connct_state_t gprs_connct_state = GPRS_CONNCT_CMD_CCLK;

    switch(gprs_connct_state)
	{
        case GPRS_CONNCT_CMD_CCLK:
        {
			sts = CON_IN_PRG;
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
#ifdef GPRS_DEBUG
			UWriteString((char *)"cmd: CCLKs", UART_PC);
#endif
			flushTxBuffer(GPRS_UART);
            flushRxBuffer(GPRS_UART);

			gprs_date_time.update_time_aval = false;
			UWriteString((char *)"AT+CCLK?\r\n", GPRS_UART);
			gprs_connct_state = GPRS_CONNCT_RSP_CCLK;
        }
        break;

        case GPRS_CONNCT_RSP_CCLK:
        {
#ifdef DEBUG_MILLIS
            printf("\nS2=%d,%d,t=%lld",gprs.state,gprs_connct_state,(esp_timer_get_time()/1000));
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];
			gprs_date_time_t temp_date_time;
			char resp = check_string("+CCLK: \"", tmpstr, &num_byte);

#ifdef GPRS_DEBUG
			UWriteString((char *)"RSP_CCLK2:", UART_PC);
			UWriteString(tmpstr, UART_PC);
#endif
			switch (resp)
			{
                case GPRS_MATCH_FAIL:
                {
#ifdef GPRS_DEBUG
					UWriteString((char *)"CCLK:f", UART_PC);
#endif					
					gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
					gprs_retry_count++;
					if (gprs_retry_count >= RETRY_CNT) 
					{
						//gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
						sts = CON_FAIL;
						
						//gprs_connct_state = GPRS_CONNCT_CMD_AT;
						//gprs_connct_state = GPRS_CONNCT_CMD_CPIN;	//PP commented on 13-09-23 and forgot to reset this sts. doing this today on 19-09-23
						
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;	//pp added on 19-09-23

						gprs_retry_count = 0;
					}                    
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef GPRS_DEBUG
						UWriteString((char *)"CCLK:k", UART_PC);
#endif					
#ifdef DEBUG_GPRS_INFO
						UWriteString((char*)"\nCCLK:",UART_PC);
						UWriteString(tmpstr,UART_PC);
#endif	
						//Lets update time
						//if (check_date_time(tmpstr, (char *)&gprs_date_time))
						if (check_date_time(tmpstr, (char *)&temp_date_time))
						{
#ifdef GPRS_DEBUG
							UWriteString((char*)"Leave Time", UART_PC);
							UWriteInt(temp_date_time.yy, UART_PC);
							UWriteInt(temp_date_time.mm, UART_PC);
							UWriteInt(temp_date_time.dd, UART_PC);
							UWriteInt(temp_date_time.hr, UART_PC);
							UWriteInt(temp_date_time.min, UART_PC);
							UWriteInt(temp_date_time.sec, UART_PC);
#endif
							if (((temp_date_time.yy >= (DEFAULT_YEAR%100)) && (temp_date_time.yy <= ((DEFAULT_YEAR%100) + YEAR_OFFSET))) && 			// Assuming that RTC will never go below 2020.
							((temp_date_time.mm >= 1) && (temp_date_time.mm <= 12)) &&
							((temp_date_time.dd >= 1) && (temp_date_time.dd <= 31))&&
							((temp_date_time.hr >= 0) && (temp_date_time.hr <= 23)) &&
							((temp_date_time.min >= 0) && (temp_date_time.min <= 59)) &&
							((temp_date_time.sec >= 0) && (temp_date_time.sec <= 59)))
							{
#ifdef GPRS_DEBUG
								UWriteString((char*)"TIME OK", UART_PC);
#endif
								memcpy(&gprs_date_time, &temp_date_time, sizeof(gprs_date_time_t));
								gprs_date_time.update_time_aval = true;
							}
							else
							{
#ifdef GPRS_DEBUG
								UWriteString((char*)"TIME Not OK", UART_PC);
#endif
							}
						}
                        sts = CON_OK;
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
						gprs_retry_count = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef GPRS_DEBUG
					//UWriteString((char *)"CCLK:w", UART_PC);
#endif					
					if(timeout++ >= GPRS_ATE0_TIMEOUT)
					{
#ifdef GPRS_DEBUG
						UWriteString((char *)"CCLK:t1", UART_PC);
#endif
						timeout = 0;
						gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
						
						if (gprs_retry_count++ >= RETRY_CNT)
						{
#ifdef GPRS_DEBUG
							UWriteString((char *)"CCLK:t2", UART_PC);
#endif
							/* gprs.module_status = NOT_AVBL;
							
							gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK; */
							sts = CON_FAIL;
							
							//gprs_connct_state = GPRS_CONNCT_CMD_AT;
							//gprs_connct_state = GPRS_CONNCT_CMD_CPIN;	//pp commented on 19-09-23
							gprs_connct_state = GPRS_CONNCT_CMD_CCLK;	//pp added on 19-09-23
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

        default:
            gprs_connct_state = GPRS_CONNCT_CMD_CCLK;
        break;
    }

    return sts;
}

void simcom_power_reset()
{
/* 	mcp.digitalWrite(MCP_SIM_RST,HIGH);
	delay(500);
	mcp.digitalWrite(MCP_SIM_RST,LOW);
	delay(500); */
    MCP23017_pin_write(GPB_ADDR, MCP_SIM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    MCP23017_pin_write(GPB_ADDR, MCP_SIM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void simcom_power_off()
{
/* 	mcp.pinMode(MCP_SIM_PWREN,OUTPUT);
	mcp.digitalWrite(MCP_SIM_PWREN,LOW);
	delay(500); */
    MCP23017_set_dir(IODIRB, MCP_SIM_PWREN, 0);
    MCP23017_pin_write(GPB_ADDR, MCP_SIM_PWREN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

uint8_t simcom_power_on()
{
    MCP23017_set_dir(IODIRB, MCP_SIM_PWREN, 0);
    MCP23017_pin_write(GPB_ADDR, MCP_SIM_PWREN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    simcom_power_reset();
	uint8_t retry = 0, MCP_portb_read = 0;
    i2c_read_byte(MCP23017_ADDR_BASE, GPB_ADDR, &MCP_portb_read);

    while(!(MCP_portb_read & MCP_SIM_1V8))
    {
        MCP23017_pin_write(GPB_ADDR, MCP_SIM_ONOFF, 1);
        vTaskDelay(pdMS_TO_TICKS(2000));
        MCP23017_pin_write(GPB_ADDR, MCP_SIM_ONOFF, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));
        i2c_read_byte(MCP23017_ADDR_BASE, GPB_ADDR, &MCP_portb_read);
        if (retry==5) 
        {
            break;            
        }
		retry++;
    }

    i2c_read_byte(MCP23017_ADDR_BASE, GPB_ADDR, &MCP_portb_read);
    if (MCP_portb_read & MCP_SIM_1V8)
    {
#ifdef GPRS_DEBUG
        printf("\nSIMCOM_SUCCESS");
#endif
        return SIMCOM_SUCCESS;
    }
    return SIMCOM_FAILED;
}

void init_modem()
{
    if(!simcom_power_on())
    {
        printf("\nMDM_PWR_FAIL");
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds

    initUart(GPRS_UART);
}

int get_rx_data(char *copy_here)
{
	int retval = 0;
	//communication(GPRS_UART, FALSE);	
	if (copy_here)
	{
        /* if((getFdmState() == FDM_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG))
		{
            memcpy (copy_here, (const void *)gprs_temp_rx_buff.buff, gprs_temp_rx_buff.index);
			copy_here[gprs_temp_rx_buff.index] = '\0';
			
			retval = gprs_temp_rx_buff.index;
			gprs_temp_rx_buff.index = 0;
        }
        else */
        {
            memcpy (copy_here, (const void *)gprs_rx_buff.buffer, gprs_rx_buff.index);
            copy_here[gprs_rx_buff.index] = '\0';
            
            retval = gprs_rx_buff.index;
            gprs_rx_buff.index = 0;

            /* retval = wifi_Rx_Buff.index;
            wifi_Rx_Buff.index = 0; */
        }
    }
    return retval;
}

char check_string(const char *str, char *copy_here, int* numbytes)
{
	char retval = GPRS_NO_NEW_MSG;
	int i, /*numbytes=0,*/ j,ip_str_len;
	char *lock_ptr = NULL;
	
	ip_str_len = strlen(str);
	if (!copy_here) 
	{
		retval = IS_FAILURE;
		return retval;
	}	
	/*if(get_frmwr_update_state() == HEX_FILE_UPDATE_PRG)
	{
		lock_ptr = (char*)&gprs_temp_rx_buff.locked;
	}
	else*/
	//{
		lock_ptr = (char*)&gprs_rx_buff.locked;
	//}
	if(*lock_ptr == LOCKED )
	{
		*lock_ptr = UNLOCKED;
		*numbytes = get_rx_data(copy_here);
	
		//If we dont have anything to check it means we have to return whatever we got
		if((str[0]) != '\0')
		{			
			if(*numbytes > 0)
			{
				for (i=0; i < *numbytes; i++) 
				{
					if (!memcmp(&copy_here[i], str, ip_str_len)) break;
				}
				if(i >= *numbytes) 
				{
					return retval = GPRS_MATCH_FAIL;
				}		
				retval = GPRS_MATCH_OK;
				for(j = 0; j < (*numbytes - i - ip_str_len); j++)
				{
					copy_here[j] = copy_here[j + i + ip_str_len];
				}
				*numbytes = j;
				copy_here[j] = '\0';
			}
		}
		else
		{
			retval = GPRS_MATCH_OK;
		}
	}
	return (retval);
}

char check_string_nobuf(const char *str)
{
	int len = 0;
	char tmpstr[GPRS_RX_BUFFER_MAX];
	
	return check_string(str, tmpstr, &len);
}

gprs_status_t getGPRSNWSts(void)
{
	return gprs.network_status;
}

void setGPRSNWSts(gprs_status_t sts)
{
	gprs.network_status = sts;
}

gprs_status_t getGPRSConnSts(void)		//Anand 25.04.2014
{
	return gprs.connect_sts;
}

void setGPRSConnSts(gprs_status_t sts)		//Anand 25.04.2014
{
	gprs.connect_sts = sts;
}

gprs_status_t getSIMSts(void)
{
	return gprs.sim_sts;
}

void setSIMSts(gprs_status_t val)
{
	gprs.sim_sts = val;
}