#ifndef _EEPROM_H_
#define _EEPROM_H_

#define EEPROM_NAMESPACE "my_eeprom"
#define EEPROM_START_ADDRESS "data"  // Starting address in flash memory
#define E2P_SD_CFG  "sdrate"
#define DATA_SIZE 8

typedef struct
{
  char dd;
  char mm;
  char yy;
  char QQ;
  unsigned char reserved[3];
  unsigned char chksum;
}__attribute__((packed)) e2p_date_t;

typedef struct
{
  uint8_t data_upld_rate;
  uint8_t server_sync_time;
  uint32_t sd_write_rate;
  uint8_t reserved;
  uint8_t chksum;
}e2p_sd_wr_cfg_t;

void init_eeprom();
void eepromWriteDate();
char eepromReadDate();
void date_change_e2p();
void eepromSdWriteRate();
// void eeproSdmWriteRate();
//void eepromSdWriteRate();
char eepromSdReadRate();
#endif