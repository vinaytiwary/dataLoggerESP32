#ifndef __GPIO_H__
#define __GPIO_H__

typedef enum
{
    NOT_CHARGING,
    CHARGING
}charger_sts_t;

void set_charger_sts(charger_sts_t);
charger_sts_t get_charger_sts(void);

void charger_status_atStartup(void);

void check_charger_status(void);

void control_RGB(void);

void init_gpio(void);


#endif