#ifndef _ERROR_H_
#define _ERROR_H_

/* typedef enum
{
	SYS_RTC,					//Bit 0: RTC corrupted				1: RTC reset/corrupted, 0: RTC ok
	SYS_E2P,					//Bit 1: E2P corrupted				1: E2P checksum not matched, 0: E2P ok
	SYS_GPS_PASS,					//Bit 2: GPS						1: GPS not available, 0: GPS available
	SYS_GPS_DATE,				//Bit 3: GPS Date Corrupted			1: GPS date out of range/corrupted, 0: GPS date ok
	SYS_SD_CARD,				//Bit 4: SD Card					1: SD Card not detected, 0: SD Card detected
	SYS_SD_CARD_INIT,
	//SYS_SD_CARD_IF,					//Bit 6: SD Card OP Cmd				1: SD Card OP Cmd unsuccessful, initialization failed, 0: SD Card OP Cmd successful
	//SYS_SD_CARD_HC,			//Bit 5: SD Card IF Cmd				1: SD Card If Cmd unsuccessful, voltage range is not 2.7 to 3.6V, initialization failed, 0: SD Card IF Cmd successfull 
	//SYS_SD_CARD_OP,				//Bit 7: SD Card HC/OCR Cmd			1: SD Card not High Capacity, initialization failed, 0: SD Card HC/OCR Cmd successful
	//SYS_SD_CARD_FAT32,			//Bit 8: SD Card File Format		1: SD Card does not have FAT32 file format, initialization failed, 0: SD Card has FAT32 file format 
	//SYS_BATT_STS1,				//Bit 9: Battery charging sts1		Bit 9, Bit 10 = 0,0: Battery discharging, not low yet. Bit 9, Bit 10 = 0,1: Battery charging, full voltage.
	//SYS_BATT_STS2,				//Bit 10: Battery charging sts2		Bit 9, Bit 10 = 1,0: Battery charging, not full yet. Bit 9, Bit 10 = 1,1: Battery discharging, low voltage.
	SYS_CHARGING,
	SYS_BATT_LOW,
	SYS_BATT_FULL,
	SYS_GSM_SIM,
}GDL_err_sts_bits_t; */ //PP commented on 30-09-23_12-40PM

typedef enum
{
	SYS_RTC,					//Bit 0: RTC corrupted				1: RTC reset/corrupted, 0: RTC ok
	SYS_E2P,					//Bit 1: E2P corrupted				1: E2P checksum not matched, 0: E2P ok
	SYS_GPS_PASS,				//Bit 2: GPS						1: GPS not available, 0: GPS (both GGA & GNSS) available
    SYS_GGA,					//Bit 3: GGA						1: GGA not avbl, 0: GGA avbl
	SYS_GNSS,					//Bit 4: GNSS						1: GNSS not avbl, 0: GNSS avbl
	SYS_GPS_DATE,				//Bit 5: GPS Date Corrupted			1: GPS date out of range/corrupted, 0: GPS date ok
	SYS_GSM_SIM,				//Bit 6: GSM SIM					1: Sim not inserted, 0: Sim inserted
	SYS_GPRS_DATE,				//Bit 7: GPRS Date Corrupted		1: GPRS date out of range/corrupted, 0: GPRS date ok
	SYS_SD_CARD,				//Bit 8: SD Card					1: SD Card not detected, 0: SD Card detected
	SYS_SD_CARD_INIT,			//Bit 9: SD Card					1: SD Card init fail, 0: SD Card init ok
	SYS_CHARGING,				//Bit 10: Batt Charging				1: Charger Connected, 0: Charger Disconnected
	SYS_BATT_LOW,				//Bit 11: Batt low					1: Battery below low threshold voltage, 0: Battery above low threshold voltage	/ Charging
	SYS_BATT_FULL,				//Bit 12: Batt low					1: Battery above Full threshold voltage, 0: Battery below Full threshold voltage / Charging
	SYS_SD_CARD_STATE,          //Bit 13: SD Card W/R states		1:SD States(W+R), 0:SD states(W)	
	SYS_SD_DETECT,               //bIT 14: SD Card detect            1: SD Card not detect, 0:SD Card detect
	SYS_SD_TIMEOUT,   			//Bit 15: SD Card Timeout           1:SD time out complete, 0:SD time out not complete
	SYS_SD_EJECT, 				//Bit 16: SDCard eject				1:SD eject, 0:SD not eject
}GDL_err_sts_bits_t;

void set_system_state(GDL_err_sts_bits_t data, unsigned char sts);
unsigned long get_system_status(void);

void check_system_status(void);

char Check_card_detection(esp_err_t err);

#endif
