#include "Common.h"
#if (SETUPA_testing == 1) && (SETUPB_testing == 0)
#include <stdio.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "hal/timer_hal.h"
// #include "hal/timer_types.h"
// #include "hal/timer_ll.h"
// #include "esp_private/periph_ctrl.h"
// #include "soc/timer_group_struct.h"
// #include "soc/soc.h"
// #include "soc/timer_periph.h"
// #include "hal/clk_tree_hal.h"
// #include "esp_clk_tree.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "timer.h"
#include "gprs.h"

// timer_hal_context_t timer_hal0;
esp_timer_handle_t timer_handler;
schedular_flg_t schedular_flg;
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

void timer_callback(void *param)
{
 static unsigned char cnt_50ms = 0;
 static unsigned char cnt_100ms = 0;
 static unsigned char cnt_1sec = 0;
 cnt_50ms++;
 cnt_100ms++;
 cnt_1sec++;

if(gprs_rx_isr_handler.elapsed < 0xFF)
  gprs_rx_isr_handler.elapsed++;

 schedular_flg.flg_10ms  = true;
 if(cnt_50ms == SCHEDULE_50MS_CNT)
 {
    cnt_50ms = 0;
    schedular_flg.flg_50ms  = true;
  }
  /* 100msec flag check to perform 100msec task */
  if(cnt_100ms == SCHEDULE_100MS_CNT)
  {
    cnt_100ms = 0;
    schedular_flg.flg_100ms = true;
  }
  /* 1sec flag check to perform 1sec task */
  if(cnt_1sec == 100)
  {
    cnt_1sec = 0;
    schedular_flg.flg_1sec = true;
  }
}

void init_timer()
{
  const esp_timer_create_args_t my_timer_args = {
    .callback = &timer_callback,
    .name = "My Timer"};
  ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 10000));
}

void init_schedular( void )                           //VINAY 15/07/2023
{
  schedular_flg.flg_10ms = false;
  schedular_flg.flg_50ms = false;
  schedular_flg.flg_100ms = false;
  schedular_flg.flg_1sec = false;
}

#elif (SETUPB_testing == 1) 

#include <stdio.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/timer_hal.h"
#include "hal/timer_types.h"
#include "hal/timer_ll.h"
#include "hal/clk_tree_ll.h"
#include "esp_private/periph_ctrl.h"
#include "soc/timer_group_struct.h"
#include "soc/soc.h"
#include "soc/timer_periph.h"
#include "hal/clk_tree_hal.h"
#include "esp_clk_tree.h"
#include "esp_log.h"
//#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "driver/gptimer.h"


#include "timer.h"
#include "pins.h"
#include "Common.h"
#include "gprs.h"

timer_hal_context_t timer_hal0;
esp_timer_handle_t timer_handler;
schedular_flg_t schedular_flg;
static intr_handle_t handle_timer0;
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

void IRAM_ATTR timer_group0_isr(void *param)
{
    portENTER_CRITICAL_ISR(&isr_mutex_global);

    static unsigned char cnt_50ms = 0;
    static unsigned char cnt_100ms = 0;
    static unsigned char cnt_1sec = 0;
    cnt_50ms++;
    cnt_100ms++;
    cnt_1sec++;

    //GPIO.out ^= (1<<ESP_CAN_TX);

    //clear pending interrupt event
    timer_ll_clear_intr_status(&TIMERG0, TIMER_LL_EVENT_ALARM(0));
/* 
    //timer_ll_enable_alarm(&TIMERG0, 0, true);
    // TIMERG0.hw_timer[0].config.tx_alarm_en = 1;
    // TIMERG0.hw_timer[0].config.tx_edge_int_en = 1;

    //set timer counter reload value = 0
    timer_ll_set_reload_value(&TIMERG0, 0, 0);

    //setting this will instantly update the reload value & timer counter value from the 2 DWORD registers of reload value in one go. 
    timer_ll_trigger_soft_reload(&TIMERG0, 0);

    //set alarm trigger value = 2403846 (number of ticks in 10ms calculated on a frequency of 240 MHz, no prescaler as timer counter is 64 bit wide)
    timer_ll_set_alarm_value(&TIMERG0, 0, 2403846);
 */
    //set timer 0's alarm to be triggered when timer counter matches the alarm value
    timer_ll_enable_alarm(&TIMERG0, 0, true);

    if(gprs_rx_isr_handler.elapsed < 0xFF)
    gprs_rx_isr_handler.elapsed++;

    schedular_flg.flg_10ms  = true;
    if(cnt_50ms == SCHEDULE_50MS_CNT)
    {
        cnt_50ms = 0;
        schedular_flg.flg_50ms  = true;
    }
    /* 100msec flag check to perform 100msec task */
    if(cnt_100ms == SCHEDULE_100MS_CNT)
    {
        cnt_100ms = 0;
        schedular_flg.flg_100ms = true;
    }
    /* 1sec flag check to perform 1sec task */
    if(cnt_1sec == 100)
    {
        cnt_1sec = 0;
        schedular_flg.flg_1sec = true;
    }

    portEXIT_CRITICAL_ISR(&isr_mutex_global);
}

void init_timer()
{
    esp_err_t ret;
    uint32_t sclk_freq, intr_mask = 0;

    uint32_t soc_clk_sel = REG_GET_FIELD(RTC_CNTL_CLK_CONF_REG, RTC_CNTL_SOC_CLK_SEL);
    uint32_t cpu_period_sel = DPORT_REG_GET_FIELD(DPORT_CPU_PER_CONF_REG, DPORT_CPUPERIOD_SEL);

    printf("\n1_f1,f2=%lu,%lu",soc_clk_sel,cpu_period_sel);

    //enables timer 0 module
    periph_module_enable(PERIPH_TIMG0_MODULE);

    //updates timer group and timer id number ??
    timer_hal_init(&timer_hal0, 0, 0);

    //disable interrupt
    timer_ll_enable_intr(&TIMERG0, TIMER_LL_EVENT_ALARM(0), false); 

    //clear pending interrupt event
    timer_ll_clear_intr_status(&TIMERG0, TIMER_LL_EVENT_ALARM(0)); 

    //timer_ll_set_clock_source(&TIMERG0, 0, src_clk);

    // set counting direction
    timer_ll_set_count_direction(&TIMERG0, 0, GPTIMER_COUNT_UP);

    //enable timer counter
    timer_ll_enable_counter(&TIMERG0, 0, true);

    //set timer counter reload value = 0
    timer_ll_set_reload_value(&TIMERG0, 0, 0);

    //setting this will instantly update the reload value & timer counter value from the 2 DWORD registers of reload value in one go. 
    timer_ll_trigger_soft_reload(&TIMERG0, 0);

    timer_ll_set_clock_prescale(&TIMERG0, 0, 2);

    //set alarm trigger value = 2403846 (number of ticks in 10ms calculated on a frequency of 240 MHz, no prescaler as timer counter is 64 bit wide)
    //timer_ll_set_alarm_value(&TIMERG0, 0, 2403846);
    //timer_ll_set_alarm_value(&TIMERG0, 0, 1204819);
    timer_ll_set_alarm_value(&TIMERG0, 0, 400000);

    //set timer counter to be auto reloaded at alarm event
    timer_ll_enable_auto_reload(&TIMERG0, 0, true);

    //set timer 0's alarm to be triggered when timer counter matches the alarm value
    timer_ll_enable_alarm(&TIMERG0, 0, true);

    soc_clk_sel = REG_GET_FIELD(RTC_CNTL_CLK_CONF_REG, RTC_CNTL_SOC_CLK_SEL);
    cpu_period_sel = DPORT_REG_GET_FIELD(DPORT_CPU_PER_CONF_REG, DPORT_CPUPERIOD_SEL);

    printf("\n2_f1,f2=%lu,%lu",soc_clk_sel,cpu_period_sel);

    //enable interrupt
    timer_ll_enable_intr(&TIMERG0, TIMER_LL_EVENT_ALARM(0), true);     

    //allocate interrupt to an isr
    ret = esp_intr_alloc(timer_group_periph_signals.groups[0].timer_irq_id[0], ESP_INTR_FLAG_IRAM, timer_group0_isr, 0, &handle_timer0);

    printf("timer_intr=%d\n", ret);
}

void init_schedular( void )                           //VINAY 15/07/2023
{
  schedular_flg.flg_10ms = false;
  schedular_flg.flg_50ms = false;
  schedular_flg.flg_100ms = false;
  schedular_flg.flg_1sec = false;
}
#endif