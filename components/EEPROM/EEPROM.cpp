#include <stdio.h>
#include <string.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "EEPROM.h"
#include "my_rtc.h"
#include "Handle_sdcard.h"
#include "Common.h"
#include "UART.h"
#include "app_comm.h"
#include "_debug.h"
#include "error.h"

static const char *E2P_TAG = "Flash EEPROM Example"; 
uint8_t read_data[DATA_SIZE];
size_t read_data_size = DATA_SIZE;
esp_err_t ret;
e2p_date_t e2p_date;

e2p_sd_wr_cfg_t e2p_sd_wr_cfg;

uint8_t e2p_read_date = false;

extern ram_data_t ram_data;

void init_eeprom() 
{
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Open NVS namespace
    nvs_handle_t nvs_handle;
    ret = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS open failed");
        return;
    }
}

void eepromWriteDate() 
{
    /* e2p_date.dd = rtc_getDay();
    e2p_date.mm = rtc_getMonth()+1;
    e2p_date.QQ = (rtc_getMinute()/2) +1;
    e2p_date.yy =rtc_getYear()%100;
    e2p_date.chksum = 0x10; */
    /* e2p_date.dd =16;
    e2p_date.mm =9;
    e2p_date.QQ = 3;
    e2p_date.yy = 23;
    e2p_date.chksum =25; */
    
    e2p_date.chksum = getChecksum((unsigned char*)(&e2p_date), (sizeof(e2p_date_t) - 1));

    nvs_handle_t nvs_handle;
    ret = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS open failed");
        return;
    }
    /* ret = */ nvs_set_blob(nvs_handle, EEPROM_START_ADDRESS, &e2p_date/* data_to_write */, DATA_SIZE);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS write failed");
        nvs_close(nvs_handle);
        return;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS commit failed");
        nvs_close(nvs_handle);
        return;
    }
#ifdef DEBUG_E2P_DATE
    printf("e2p_wr = %d%d%d%d\n",e2p_date.dd,e2p_date.mm,e2p_date.yy,e2p_date.QQ);
#endif
}

char eepromReadDate() 
{
    int i=0;
    char read_ok = 1;
    nvs_handle_t nvs_handle;
    ret = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    for (i=0; i<3;i++)
    {
        nvs_get_blob(nvs_handle, EEPROM_START_ADDRESS, &e2p_date, &read_data_size);
        //e2p_date.chksum = 0;    //PP 19-09-23: for testing
        if(e2p_date.chksum == getChecksum((unsigned char*)&e2p_date,sizeof(e2p_date_t)-1))
		{
			break;
		}
#ifdef DEBUG_E2P_DATE
        printf("\ne2p_c=%d,e2p_r=%d\n",getChecksum((unsigned char*)&e2p_date,sizeof(e2p_date_t)-1), e2p_date.chksum);
#endif
    }
    if(i>=3)
	{
#ifdef DEBUG_E2P_DATE
        printf("\ne2pF");
#endif
		read_ok = 0;
	}
   /*  ret = nvs_get_blob(nvs_handle, EEPROM_START_ADDRESS, &e2p_date, &read_data_size);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS read failed");
        nvs_close(nvs_handle);
        return;
    } */
    //printf("write_data= %s\n",e2p_date);
    // Print read data
    //ESP_LOGI(E2P_TAG, "Read data:");
#ifdef DEBUG_E2P_DATE
    printf("e2p_rd = %d %d %d %d ",e2p_date.dd,e2p_date.mm,e2p_date.yy,e2p_date.QQ);
    for (int i = 0; i < 3; i++) 
    {
        printf("%d ", e2p_date.reserved[i]);
    }
    printf("%d", e2p_date.chksum);
    printf("\n");
#endif

    nvs_close(nvs_handle);   // Close NVS handle
    return read_ok;
}

void date_change_e2p()             //Vinay 02-08-2023           
{
    if(e2p_read_date)
    {
        set_system_state(SYS_E2P, LOW);
        if (((ram_data.time.yy == 00) && (ram_data.time.mm == 01) && (ram_data.time.dd == 01)))
        {
#ifdef DEBUG_E2P_DATE
            //printf("rtc=default");
#endif
        }
        else if((e2p_date.mm == ram_data.time.mm) && (e2p_date.dd == ram_data.time.dd) && (e2p_date.QQ == (ram_data.time.hr/6)+1))
        {   
#ifdef DEBUG_E2P_DATE
            //printf("rtc_correct");
#endif
        }
        else
        {
            if ((e2p_date.QQ != (/* rtc_getMinute() */(ram_data.time.hr/6)+1)))
            { 
                //memset(&e2p_date, 0, sizeof(e2p_date_t));
                e2p_date.yy = ram_data.time.yy;
                e2p_date.mm = ram_data.time.mm;
                e2p_date.dd = ram_data.time.dd;
                e2p_date.QQ = (/* rtc_getMinute() */(ram_data.time.hr/6)+1);

#ifdef DEBUG_E2P_DATE
                printf("e2p_qq!=H/6+1"); 
#endif
                eepromWriteDate();
            }
            else if(e2p_date.dd != ram_data.time.dd)
            {  
                //memset(&e2p_date, 0, sizeof(e2p_date_t));  
                e2p_date.yy = ram_data.time.yy;
                e2p_date.mm = ram_data.time.mm;
                e2p_date.dd = ram_data.time.dd;
                e2p_date.QQ = (/* rtc_getMinute() */(ram_data.time.hr/6)+1);
#ifdef DEBUG_E2P_DATE
                printf("e2p_D!=RTC_D"); 
#endif
                eepromWriteDate();
            }
            else if(e2p_date.mm != ram_data.time.mm)
            {      
                //memset(&e2p_date, 0, sizeof(e2p_date_t));      
                e2p_date.mm = ram_data.time.mm;
                e2p_date.yy = ram_data.time.yy;
                e2p_date.dd = ram_data.time.dd;
                e2p_date.QQ = (/* rtc_getMinute() */(ram_data.time.hr/6)+1);
#ifdef DEBUG_E2P_DATE
                printf("e2p_M!=RTC_M");     
#endif
                eepromWriteDate();
            }
            else if (e2p_date.yy != ram_data.time.yy)
            {     
                //memset(&e2p_date, 0, sizeof(e2p_date_t));        
                e2p_date.yy = ram_data.time.yy;
                e2p_date.mm = ram_data.time.mm;
                e2p_date.dd = ram_data.time.dd;
                e2p_date.QQ = (/* rtc_getMinute() */(ram_data.time.hr/6)+1);
#ifdef DEBUG_E2P_DATE
                printf("e2p_Y!=RTC_Y");     
#endif
                eepromWriteDate();    
            }
        }
    }
    else
    {
        set_system_state(SYS_E2P, HIGH);
#ifdef DEBUG_E2P_DATE
		UWriteString((char*)"\ne2pR", UART_PC);
		UWriteInt(e2p_date.dd, UART_PC);
		UWriteData('-', UART_PC);
		UWriteInt(e2p_date.mm, UART_PC);
		UWriteData('-', UART_PC);
		UWriteInt(e2p_date.yy, UART_PC);
		UWriteData(',', UART_PC);
		UWriteInt(e2p_date.QQ, UART_PC);
#endif		
		if (((ram_data.time.yy == 00) && (ram_data.time.mm == 01) && (ram_data.time.dd == 01)))
		{
			//set_system_state(SYS_RTC, HIGH);
#ifdef DEBUG_E2P_DATE
			UWriteString((char*)"\nrtcR+e2pR", UART_PC);
#endif					
		}
		else
		{
			//set_system_state(SYS_RTC, LOW);
			//memset(e2p_date, 0, sizeof(e2p_date_t));
			e2p_date.yy = ram_data.time.yy;
            e2p_date.mm = ram_data.time.mm;
            e2p_date.dd = ram_data.time.dd;
            e2p_date.QQ = (/* rtc_getMinute() */(ram_data.time.hr/6)+1);
			
			eepromWriteDate();

#ifdef DEBUG_E2P_DATE
			UWriteString((char*)"\ne2pDS:", UART_PC);
			UWriteInt(e2p_date.dd, UART_PC);
			UWriteData('-', UART_PC);
			UWriteInt(e2p_date.mm, UART_PC);
			UWriteData('-', UART_PC);
			UWriteInt(e2p_date.yy, UART_PC);
			UWriteData(',', UART_PC);
			UWriteInt(e2p_date.QQ, UART_PC);		
#endif
        }
    }     
}

void eepromSdWriteRate()
{
    e2p_sd_wr_cfg.data_upld_rate=10; //for testing only
    e2p_sd_wr_cfg.server_sync_time=0; //for testing only
    e2p_sd_wr_cfg.sd_write_rate=1;    //for testing only
    e2p_sd_wr_cfg.reserved=0;        //for testing only
    //e2p_sd_wr_cfg.chksum=10;          //for testing only

    e2p_sd_wr_cfg.chksum = getChecksum((unsigned char*)(&e2p_sd_wr_cfg), (sizeof(e2p_sd_wr_cfg_t) - 1));
    
    nvs_handle_t nvs_handle;
    ret = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS open failed");
        return;
    }
    /* ret = */ nvs_set_blob(nvs_handle, E2P_SD_CFG, &e2p_sd_wr_cfg/* data_to_write */, DATA_SIZE);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS write failed");
        nvs_close(nvs_handle);
        return;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(E2P_TAG, "NVS commit failed");
        nvs_close(nvs_handle);
        return;
    }
}

char eepromSdReadRate()
{
    int i=0;
    char read_ok = 1;
    nvs_handle_t nvs_handle;
    ret = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    for (i=0; i<3;i++)
    {
        nvs_get_blob(nvs_handle, E2P_SD_CFG, &e2p_sd_wr_cfg, &read_data_size);
        if(e2p_sd_wr_cfg.chksum == getChecksum((unsigned char*)&e2p_sd_wr_cfg,sizeof(e2p_sd_wr_cfg_t)-1))
		{
			break;
		}
    }
    if(i>=3)
	{
		read_ok = 0;
	}

    nvs_close(nvs_handle);   // Close NVS handle
    printf("\ndata_upld_rate=%d",e2p_sd_wr_cfg.data_upld_rate);
    printf("\nsd_write_rate=%ld",e2p_sd_wr_cfg.sd_write_rate);
    return read_ok;
}

