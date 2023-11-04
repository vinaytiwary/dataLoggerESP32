#ifndef GPS_H_
#define GPS_H_

#include "clock.h"

//#define COMBINE_GGA_GNSS_CMD

#define GPS_RETRY_CNT (3)
#define GGA_GNSS_TIMEOUT	(500/WEB_STATE_MC_TIME)
#define CBC_TIMEOUT			(500/WEB_STATE_MC_TIME)

#define LAT_LEN (12)
#define LONG_LEN (13)

#define MAX_DATE_SIZE		(6)
#define MAX_TIME_SIZE		(6)

#ifdef GNS_PKT_EN
#define LATITUDE_DP				(4)//(6)
#else
#define LATITUDE_DP				(6)	//GNS_PKT_EN is defined in gps.h, which ain't included. So we've been using LATITUDE_DP = 6 this whole time. It works so do not change.
#endif

#define SPEED_DP				(2)
#define GPS_LAT_STATUS_BIT		(10)
#define GPS_LONG_STATUS_BIT		(11)
#define GPS_POSITION_STATUS_BIT	(12)
#define GPS_POSITION_BIT		(13)

enum
{
	LAT_DIR_BIT,
	LONG_DIR_BIT,
};

typedef enum
{
	GPS_PASS,
	GPS_FAIL,
	GPS_IN_PRG,
	GPS_WAIT
}gps_status_t;

enum {GPS_NO_NEW_MSG, GPS_MATCH_FAIL, GPS_MATCH_OK} ;

typedef enum
{
    GPS_IDLE,						//0
    GPS_AT_INIT,					//1
	GPS_CMD_QUERY_ENABLE,			//2
	GPS_RSP_QUERY_ENABLE,			//3
    GPS_CMD_ENABLE,					//4
    GPS_RSP_ENABLE,					//5
	GPS_CMD_CBC,					//6
	GPS_RSP_CBC,					//7
	GPS_CMD_GPGGA_START,			//8
	GPS_RSP_GPGGA_START,			//9
    GPS_CMD_LOCATION,				//10
	GPS_RSP_LOCATION,				//11
	GPS_CMD_GPGGA_STOP,				//12
	GPS_RSP_GPGGA_STOP,				//13
#ifdef COMBINE_GGA_GNSS_CMD
	GPS_CMD_GGA_GNSS,				//14
	GPS_RSP_GGA_GNSS,				//15
#endif
}gps_handler_state_t;

typedef struct
{
	unsigned char fix_mode;
	unsigned char date[7];
	unsigned char utc_time[7];
	unsigned char latitude[LAT_LEN];
	unsigned char longitude[LONG_LEN];
	unsigned char sog[6];
	unsigned char cog[6];
	unsigned char quantity_of_gps;
	unsigned char MSL_altitude[10];
	unsigned char N_S;
	unsigned char E_W;
	unsigned char pdop[6];
	unsigned char hdop[6];
	unsigned char vdop[6];
}__attribute__((packed))gns_info_t;

typedef struct
{
	unsigned char fix_quality;
	unsigned char quantity_of_gps;
	unsigned char MSL_altitude[10];
	unsigned char Geoid_separation[10];
	unsigned char NMEA_gga_chk;
}__attribute__((packed))gga_info_t;

typedef struct
{
	gns_info_t gns_info;
	gga_info_t gga_info;
}__attribute__((packed))gps_info_t;

typedef struct
{
	char yy;
	char mm;
	char dd;
	char hr;
	char min;
	char sec;
	char update_time_aval;
}__attribute__((packed))gps_date_time_t;

typedef struct
{
	gps_info_t gps_info;
    gps_info_t backup_gps_info;
	/* unsigned char gps_ready;
	char errcode;
	bool gga_sts;
	bool gnss_sts;
	bool NMEA_enabled;
	bool got_VBATT; */
}__attribute__((packed))gps_t;

typedef struct
{
	unsigned char gps_ready;
	char errcode;
	bool gga_sts;
	bool gnss_sts;
	bool NMEA_enabled;
	bool got_VBATT;
	gps_handler_state_t gps_handler_state;
}__attribute__((packed))gps_statuses_t;

/* typedef struct	//putting this (ram_data_t) here for a while
{
	uint32_t log_num;
	uint32_t Status;							
	uint8_t quantity_of_gps;				
	int32_t Latitude;
	int32_t Longitude;
	uint16_t speed;
	int32_t msl_altitude;
	int32_t geoid_separation;
	int32_t ellipsoid_altitude;
	uint32_t PDOP;
	int32_t sign_status;
	time_stamp_t time;
}__attribute__((packed))ram_data_t; */

gps_status_t gps_handler(void);

bool gga_pkt_parsing(char *);
bool cbc_pkt_parsing(char *tmpstr);
bool gnss_pkt_parsing(char *);
unsigned char calculate_gga_checksum(char *);
void updateGpsDateTimeToBuff(gps_date_time_t *);
void utcTOlocal(gps_date_time_t *);
void get_location(void);
void convert_dmsTOdd(char*, unsigned char);

#endif