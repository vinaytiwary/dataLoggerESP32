#ifndef WEB_COMM_H_
#define WEB_COMM_H_

#include "Common.h"
#include "gprs.h"

#define WEB_STATE_MC_TIME	(WEB_COMMS_SCHEDULAR_TIME)
#define GPS_FAIL_TIMEOUT	(1000/WEB_STATE_MC_TIME)
//#define GPS_READ_RATE_C		((10 * 1000)/ WEB_STATE_MC_TIME)
//#define GPS_READ_RATE_C		(1000/WEB_STATE_MC_TIME)
#define GPS_READ_RATE_C		(200/WEB_STATE_MC_TIME)

typedef enum
{
	//CONNECT_POWER_ON,		// 24/1/19 SK:
	CONNECT_BEGIN,			//PP added on 05-10-23 (among other webcomm, gprs, gps chnges regarding their return statuses)
	CONNECT_LOCATION,
	CONNECT_DATA_UPLOAD,
	CONNECT_LOCATION_WAIT,	// PP(02-08-22)
}conn_state_t;


con_status_t modem_initAT(void);
void manage_gps_gprs(void);

#endif