#ifndef GPRS_H_
#define GPRS_H_

#include "Common.h"

#define IS_SUCCESS 1
#define IS_FAILURE 0

#define MIN_NETWORK_STRENGTH_DB (2)					//HJ 01-02-2019 Change from 10 to 2
#define MAX_NETWORK_STRENGTH_DB (30)

#define GPRS_APN_LEN				(32)
#define GPRS_URL_LEN				(32)
#define GPRS_UNUSED_LEN				(31)
#define GPRS_TX_BUFFER_MAX 192
//#define GPRS_RX_BUFFER_MAX 128		
#define GPRS_RX_BUFFER_MAX 180		

#define LOCKED 1
#define UNLOCKED 0

#define lock(a) {a = LOCKED;}
#define unlock(a) {a = UNLOCKED;}

#define IMEI_LEN				(15)

#define RETRY_CNT 				(3)
#define HTTP_RETRY_CNT			(3)
#define UPLOAD_RETRY_CNT		(3)

#define GPRS_STATE_MC_TIME		(WEB_COMMS_SCHEDULAR_TIME)

#define GPRS_AT_TIMEOUT			(30000/GPRS_STATE_MC_TIME)		//30 sec (only at startup. observed timeout = 25 sec)
#define GPRS_ATE0_TIMEOUT		(200/GPRS_STATE_MC_TIME)		//100 ms max if startup is done and AT cmd has been reponded to.
#define GPRS_CONN_RETRY_TIME	(30000/GPRS_STATE_MC_TIME)		//30Sec			

typedef enum {GPRS_NO_NEW_MSG, GPRS_MATCH_FAIL, GPRS_MATCH_OK} match_t;

typedef enum
{
	NOT_AVBL,
	AVBL,
}gprs_status_t;

typedef enum
{
	CON_OK,
	CON_FAIL,
	CON_IN_PRG,
	CON_WAIT
}con_status_t;

typedef enum
{
	HTTP_PASS,
	HTTP_FAIL,
	HTTP_IN_PRG
}http_status_t;

typedef enum {
	GPRS_RX_IDLE=0,
	GPRS_RX_INPROG,
} GPRS_rx_states;

typedef enum
{
	GPRS_IDLE,
	GPRS_AT_INIT,
	GPRS_CONNECT,
	GPRS_CONN_STS,
	GPRS_SESSION_IDLE	
}gprs_handler_state_t;

typedef enum
{
	GPRS_CONNCT_CMD_AT,
	GPRS_CONNCT_RSP_AT,
	GPRS_CONNCT_CMD_ECHO_OFF,
	GPRS_CONNCT_RSP_ECHO_OFF,
	GPRS_CONNCT_CMD_CPIN,
	GPRS_CONNCT_RSP_CPIN,
	GPRS_CONNCT_CMD_IMEI,
	GPRS_CONNCT_RSP_IMEI,
	GPRS_CONNCT_CMD_CFUN,
	GPRS_CONNCT_RSP_CFUN,
	GPRS_CONNCT_CMD_CREG,
	GPRS_CONNCT_RSP_CREG,
	GPRS_CONNCT_CMD_CSQ,
	GPRS_CONNCT_RSP_CSQ,
	GPRS_CONNCT_CMD_CGATT,
	GPRS_CONNCT_RSP_CGATT,
	GPRS_CONNCT_CMD_CCLK,
	GPRS_CONNCT_RSP_CCLK,
}gprs_connct_state_t;

typedef struct  {
	unsigned char elapsed;
	GPRS_rx_states state;
}gprs_rx_isr_handler_t;

typedef struct  {
	char locked;
	char buffer[GPRS_RX_BUFFER_MAX];
	unsigned int index;
}gprs_rx_data_buff_t;

typedef struct  {
	char locked;
	char buffer[GPRS_TX_BUFFER_MAX];
	unsigned int index;
}gprs_tx_data_buff_t;

typedef struct
{
	char locked;
	char *buff;
	unsigned int index;
}gprs_temp_rx_buff_t;

typedef struct  
{
	char yy;
	char mm;
	char dd;
	char hr;
	char min;
	char sec;
	char update_time_aval;
}gprs_date_time_t;

typedef struct
{
	char imei[IMEI_LEN];
	gprs_handler_state_t state;				//For state machine
	gprs_status_t module_status;
	
	unsigned char network_strength;			//chk for data type
	gprs_status_t network_status;
	
	gprs_status_t server_status;
	gprs_status_t connect_sts;
	gprs_status_t sim_sts;
	
	unsigned char gprs_config_sts;			// to show eeprom configration error
	
	char errcode;
}gprs_t;

uint8_t simcom_power_on(void);
void simcom_power_reset(void);
void simcom_power_off(void);

void init_modem();
int get_rx_data(char *);
char check_string(const char *, char *, int*);
char check_string_nobuf(const char *);

con_status_t gprs_connect(void);
con_status_t gprs_connect_status(void);
//void http_handler(void);
con_status_t http_handler(void);

gprs_status_t getSIMSts(void);
void setSIMSts(gprs_status_t);	

gprs_status_t getGPRSNWSts(void);
void setGPRSNWSts(gprs_status_t);

gprs_status_t getGPRSConnSts(void);
void setGPRSConnSts(gprs_status_t);	

#endif