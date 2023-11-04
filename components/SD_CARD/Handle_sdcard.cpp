#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "driver/sdmmc_host.h"
#include "rom/gpio.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "esp_timer.h"
#include "Common.h"
#include "EEPROM.h"
#include "gps.h"
#include "gprs.h"
#include "web_comm.h"
#include "adc.h"
#include "pins.h"
#include "Handle_sdcard.h"
#include "UART.h"
#include "app_comm.h"
#include "error.h"
#include "_debug.h"
#include <errno.h>

#include "esp_err.h"

#define LOG_COLUMN_COUNT 11
static const char *SD_TAG = "example";
#define MOUNT_POINT "/sdcard"

extern e2p_date_t e2p_date;
extern e2p_sd_wr_cfg_t e2p_sd_wr_cfg; 
extern conn_state_t conn_state;

extern gprs_t gprs;
extern gps_statuses_t gps_statuses;

#if 0
char * log_col_names[LOG_COLUMN_COUNT] = 
{
   "log no.","Date", "Time", "latitude", "longitude", "speed","MSL_altitude", "Geoid_separation", "ellipsoid_altitude", "quantity_of_gps", "PDOP"
}; 

#endif
const char data_logger_header[] = "Log no.,Date,Time,Latitude,Longitude,Speed(Km/hr),MSL Alt(mtr),Geoid Separation(mtr), Ellipsoid Alt(mtr),Sat,PDOP,B,M,STS,W/R,,\n";

ram_data_t ram_data;
static uint32_t log_no=0;
static uint8_t de_init=0;

char sd_buffer[160];
char Fname[30];
char Fpath[30];

void init_sdcard()
{
    esp_err_t ret;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = 
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(SD_TAG, "Initializing SD card");

    ESP_LOGI(SD_TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = 
    {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(SPI2_HOST/* host.slot */, &bus_cfg, SPI_DMA_CHAN);
    printf("\nError=%d",ret);
    if (ret != ESP_OK) 
    {
        set_system_state(SYS_SD_CARD, HIGH);
        ESP_LOGE(SD_TAG, "Failed to initialize bus.");
        //return;
    }
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = SPI2_HOST;//host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    printf("\nret=%d",ret);
    if (ret != ESP_OK) 
    {
      if (ret == ESP_FAIL) 
      {
          set_system_state(SYS_SD_CARD_INIT, HIGH);
          ESP_LOGE(SD_TAG, "Failed to mount filesystem. "
              "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
      } 
      else 
      {
          set_system_state(SYS_SD_CARD, HIGH);
          ESP_LOGE(SD_TAG, "Failed to initialize the card (%s). "
              "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
      }
      return;
    }

    // Card has been initialized, print its properties

    set_system_state(SYS_SD_CARD, LOW);
    set_system_state(SYS_SD_CARD_INIT, LOW);
    
    sdmmc_card_print_info(stdout, card);

    FILE* f = fopen(MOUNT_POINT"/Vinay.csv", "w");
    if (f == NULL) 
    {
        ESP_LOGE(SD_TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Kaise ho ji , Presani me koi kami?");
    fprintf(f,"\n");
    fclose(f);
}

void deinitialize_sdcard() 
{
    printf("\ndeinitializing sdcard");
    esp_err_t ret;
    sdmmc_card_t* card;
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(SD_TAG, "\nSD card unmounted.");
    ret = spi_bus_free(HSPI_HOST);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(SD_TAG, "\nFailed to free the SPI bus (%s).", esp_err_to_name(ret));
    } 
    else 
    {
        de_init=1;
        ESP_LOGI(SD_TAG, "\nSPI bus freed.");
    }
}

void reinitialize_sdcard() 
{
    deinitialize_sdcard();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    init_sdcard();
}

void printHeader()              //Vinay 21-07-2023
{
  {
    FILE* f = fopen(Fpath, "w"); 
    if (f) 
    {
      fprintf(f, "%s",data_logger_header);
      fclose(f); 
    }
    else
    {
      printf("\nHeader_Err=%d",errno );
    }
  }
}

void SD_file_Save()               //Vinay 21-07-2023
{
    sprintf(Fpath, "/sdcard/%02d%02d%02d_%1d.csv",  e2p_date.yy, e2p_date.mm, e2p_date.dd, e2p_date.QQ);	//e2p date
    ram_data.log_num++;

    if(!(ram_data.Status &(1<<13)))
    {
      sprintf(sd_buffer, "%08lu,%02d-%02d-20%02d,%02d:%02d:%02d,%+03ld.%06lu,%+04ld.%06lu,%03d.%02d,%+05ld.%02ld,%+05ld.%02ld,%+05ld.%02ld,%02d,%03lu.%02lu,%u.%03u,%02u.%03u,0x%08lX,WW,%d,%d,\n",
      ram_data.log_num, ram_data.time.dd,ram_data.time.mm,ram_data.time.yy,ram_data.time.hr,ram_data.time.min,ram_data.time.sec,
      ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L),ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L),(ram_data.speed/100),(ram_data.speed%100),
      ram_data.msl_altitude/100, abs(ram_data.msl_altitude%100),ram_data.geoid_separation/100, abs(ram_data.geoid_separation%100),
      ram_data.ellipsoid_altitude/100, abs(ram_data.ellipsoid_altitude%100),ram_data.quantity_of_gps,ram_data.PDOP/100, ram_data.PDOP%100,
      ram_data.v_batt/1000,ram_data.v_batt%1000,ram_data.main_supply/1000,ram_data.main_supply%1000,ram_data.Status,gprs.state,conn_state);
    }
    else
    {
      sprintf(sd_buffer, "%08lu,%02d-%02d-20%02d,%02d:%02d:%02d,%+03ld.%06lu,%+04ld.%06lu,%03d.%02d,%+05ld.%02ld,%+05ld.%02ld,%+05ld.%02ld,%02d,%03lu.%02lu,%u.%03u,%02u.%03u,0x%08lX,WR,%d,%d,\n",
      ram_data.log_num, ram_data.time.dd,ram_data.time.mm,ram_data.time.yy,ram_data.time.hr,ram_data.time.min,ram_data.time.sec,
      ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L),ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L),(ram_data.speed/100),(ram_data.speed%100),
      ram_data.msl_altitude/100, abs(ram_data.msl_altitude%100),ram_data.geoid_separation/100, abs(ram_data.geoid_separation%100),
      ram_data.ellipsoid_altitude/100, abs(ram_data.ellipsoid_altitude%100),ram_data.quantity_of_gps,ram_data.PDOP/100, ram_data.PDOP%100,
      ram_data.v_batt/1000,ram_data.v_batt%1000,ram_data.main_supply/1000,ram_data.main_supply%1000,ram_data.Status,gprs.state,conn_state);

    }
    FILE* f = fopen(Fpath, "r+");
    if(!f)
    {
      printf("\nFileSave_Err=%d",errno );
      printHeader();
      ram_data.log_num=0;
    }
    else
    {
      set_system_state(SYS_SD_DETECT, LOW);
      fclose(f);
      f = fopen(Fpath, "a");
      fprintf(f,sd_buffer);
      fclose(f); 
    }
}

void get_random_data()
{
  /* ram_data.time_stamp.yy = rand()%30;
  ram_data.time_stamp.mm = rand()%12;
  ram_data.time_stamp.dd = rand()%30;
  ram_data.time_stamp.hr = rand()%23;
  ram_data.time_stamp.min = rand()%59;
  ram_data.time_stamp.sec = rand()%59;
  ram_data.lat = rand()%100000;
  ram_data.lon = rand()%100000;
  ram_data.alt = rand()%100000;
  ram_data.speed = rand()%5;
  ram_data.accuracy = rand()%2;
  memset(buffer1,0,sizeof(buffer1)); */
}

void update_ram_data(void)
{
    static int sd_wr_rate;
    static char sd_retry_cnt;
    sd_wr_rate++;
    ram_data.main_supply = ReadVoltage(ADC1_CHANNEL_0);

#ifdef DEBUG_ADC
  //printf("\nin_12v = %u", ram_data.main_supply);
#endif
    get_location();
    sync_time_modem();
    if(sd_wr_rate ==e2p_sd_wr_cfg.sd_write_rate)
    {
      if(!(ram_data.Status &(1<<SYS_SD_TIMEOUT)) && !(ram_data.Status &(1<<SYS_SD_DETECT)))
      {
          printf("\nSYS ok");
          SD_file_Save();
      }
      else
      {
        printf("\nSYS not ok");
        if(sd_retry_cnt++ >=30)
        {
          sd_retry_cnt=0;
          set_system_state(SYS_SD_TIMEOUT, LOW);
        }
      }
      sd_wr_rate=0;
    }
}

void Get_Curr_sdData(const char* filename, int min)
{
  static uint16_t log_num = 0;
  if(timer_check(20))
  {
    if((ram_data.log_num %(min*60)==0)&& log_num==0 &&ram_data.log_num>0)
    {
      log_num=(ram_data.log_num -(min*60));
      printf("\nfilename=%s",filename);
      read_sdcard(filename,log_num);
      log_num++;
      set_system_state(SYS_SD_CARD_STATE, HIGH);
    }
    else if(log_num>=1 && ((log_num % (min*60)) !=0))
    {
      read_sdcard(filename,log_num);
      log_num++;
      set_system_state(SYS_SD_CARD_STATE, HIGH);
    }
    else
    {

#ifdef DEBUG_READ_SDCARD

      if(ram_data.log_num > (min*60))
      {
        printf("\nPlease wait :%02ld sec",(600-(ram_data.log_num % (min*60))));
      }
      else
      {
        printf("\nPlease wait :%02ld sec",((min*60)-ram_data.log_num));
      }
#endif
      set_system_state(SYS_SD_CARD_STATE, LOW);
      log_num=0;
    }
  }
}

char read_sdcard(const char *filename, long row_number)
{
    if(!(ram_data.Status &(1<<SYS_SD_TIMEOUT)) && !(ram_data.Status &(1<<SYS_SD_DETECT)))
    {
        char buffer[128];
        FILE *fp = fopen(filename, "rb");
        if (fp == NULL) 
        {
            printf("\nFailed to open file: ");
            return 0;
        }
        fseek(fp, (row_number * 128), SEEK_SET);
        {
            memset(buffer,0,sizeof(buffer));
            fread(buffer,1,128,fp);
            if(!strlen(buffer))
            {
              return 0;
            }
            printf("\n");
            UWriteString(buffer, UART_PC);
        }
        fclose(fp);
    }  
    return 1;
}

char timer_check(int ms)
{
  static const unsigned long REFRESH_INTERVAL = ms; // ms
  static unsigned long lastRefreshTime = 0;
  if((esp_timer_get_time()/1000) - lastRefreshTime >= REFRESH_INTERVAL)
  {
    lastRefreshTime += REFRESH_INTERVAL;
    return 1;
  }
  return 0;
}

void check_switch_pressed(void)
{
	static int c=1,ls=1;
	static int count=0;
	c=(gpio_get_level(GPIO_NUM_0)&1);
	if(c==0&&ls!=0)
	{ 
		if (count++>=5)
		{
			count=0;
			if(c==0)
			{
        set_system_state(SYS_SD_EJECT, HIGH);
        set_system_state(SYS_SD_TIMEOUT, HIGH);
        //set_system_state(SYS_SD_CARD_INIT, HIGH);
        MCP23017_pin_write(MCP_LED_GREEN_PORT,(MCP_LED_GREEN|MCP_LED_RED |MCP_LED_BLUE ), 0);
        printf("\nSwitch is pressed");
#ifdef DEBUG_DG_STATE
				printf("\nSwitch is pressed");
#endif			
			}
		}
	}
	else
	{
		ls=c;
	}
  
}

void check_card_detect()
{
  static char retry_cnt=0;
  if(gpio_get_level(GPIO_NUM_32))
  {
    if(retry_cnt++ >=2)
    {
      retry_cnt=0;
      set_system_state(SYS_SD_DETECT, HIGH);
      set_system_state(SYS_SD_TIMEOUT, HIGH);
      set_system_state(SYS_SD_CARD_INIT,HIGH);
      printf("\nCard not Detected");
    }
  }
  else
  {
    if((ram_data.Status &(1<<SYS_SD_CARD_INIT)) && !(ram_data.Status &(1<<SYS_SD_TIMEOUT)))
    {
      reinitialize_sdcard();
    }
    set_system_state(SYS_SD_DETECT, LOW);
  }
}
