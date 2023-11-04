#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "hal/uart_hal.h"
#include "hal/uart_ll.h"
#include "soc/uart_struct.h"
#include "esp_clk_tree.h"
#include "esp_private/periph_ctrl.h"
#include "soc/periph_defs.h"
#include "soc/uart_periph.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_intr_alloc.h"
#include "UART.h"
#include "gprs.h"
#include "app_comm.h"

volatile uint8_t Rx_buff[128] = "";
//volatile uint8_t Rx_char[1];
volatile uint8_t Rx_char;
volatile uint8_t test_uart0_intr_flg = false;
volatile uint8_t test_uart1_intr_rx_flg = false;
volatile uint8_t test_uart1_intr_tx_flg = false;

static portMUX_TYPE my_mutex = portMUX_INITIALIZER_UNLOCKED;
//static intr_handle_t handle_console;  
static intr_handle_t handle_console0;
static intr_handle_t handle_console1;  
volatile uint32_t status_intr_uart1 = 0; 
volatile uint32_t status_intr_uart0 = 0; 

uart_hal_context_t uart_hal0 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_0),};
uart_hal_context_t uart_hal1 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_1),};
uart_hal_context_t uart_hal2 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_2),};

volatile Rx_Buff_t Rx_Buff[3];
volatile Tx_Buff_t Tx_Buff[3];

extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;   //16-2-19 VC: for dynamic allocation
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

static void IRAM_ATTR uart1_intr_handle(void *arg)
{
#if (SETUPB_testing == 1) && (SETUPA_testing == 0)
    portENTER_CRITICAL_ISR(&isr_mutex_global);

    //uint32_t status;
    status_intr_uart1 = UART1.int_st.val; // read UART interrupt Status

    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), status);
    //UWriteInt(status,UART_PC);

    if(status_intr_uart1 & UART_INTR_RXFIFO_FULL)
    {
        //UWriteData('^',UART_PC);
        uart1_rx_intr();
    }
    /* else */ if(status_intr_uart1 & UART_INTR_TX_DONE)
    {
        //UWriteData('@',UART_PC);
        uart1_tx_intr();
    }

    portEXIT_CRITICAL_ISR(&isr_mutex_global);

#elif (SETUPA_testing == 1)
    //uint32_t status;
    status_intr_uart1 = UART1.int_st.val; // read UART interrupt Status

    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), status);
    //UWriteInt(status,UART_PC);

    if(status_intr_uart1 & UART_INTR_RXFIFO_FULL)
    {
        //UWriteData('^',UART_PC);
        uart1_rx_intr();
    }
    /* else */ if(status_intr_uart1 & UART_INTR_TX_DONE)
    {
        //UWriteData('@',UART_PC);
        uart1_tx_intr();
    }

#endif
}

/*
 * Define UART interrupt subroutine to ackowledge interrupt
 */
static void IRAM_ATTR uart0_intr_handle(void *arg)
{
/* 
    uint16_t rx_fifo_len, status;
    uint16_t i;

    status = UART0.int_st.val; // read UART interrupt Status
    rx_fifo_len = UART0.status.rxfifo_cnt; // read number of bytes in UART buffer

    while(rx_fifo_len){
    rxbuf[i++] = UART0.fifo.rw_byte; // read all bytes
    rx_fifo_len--;
    }
 */
    status_intr_uart0 = UART0.int_st.val; // read UART interrupt Status
    //Rx_char[0] = UART0.fifo.rw_byte;
    Rx_char = UART0.fifo.rw_byte;

    // after reading bytes from buffer clear UART interrupt status
    uart_clear_intr_status(UART_NUM_0, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
/* 
    // a test code or debug code to indicate UART receives successfully,
    // you can redirect received byte as echo also
    //uart_write_bytes(UART_NUM_0, (char*)&Rx_char , 1);
    printf("%X", Rx_char[0]);
    Rx_char[0] = 0;
 */
    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), Rx_char[0]);
    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), Rx_char);
    //UART0.fifo.rw_byte = Rx_char;

    test_uart0_intr_flg = true;

}
#if 0
void app_main(void)
{
    esp_log_level_set(UART_TAG, ESP_LOG_INFO);
    initUart(UART_NUM_0);
    initUart(UART_NUM_1);
    print_uart_cfg_Regs();
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
    flushUartFifo(&uart_hal0, RXTX);
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
        //printf("\nRx_char=0x%02X", Rx_char);
        if(test_uart0_intr_flg && UART0.status.txfifo_cnt == 0)
        {
            test_uart0_intr_flg = false;
            /* printf("\nRx_char=0x%02X", Rx_char);
            Rx_char = 0; */

            //printf("\nP");
            print_uart_fifos();
            

            /* printf("%X", Rx_char[0]);
            Rx_char[0] = 0; */
        }
    }
} 
#endif

void initUart(int uart_num)
{
    /* uint32_t sclk_freq, intr_mask = 0; */
    uart_sclk_t src_clk = UART_SCLK_APB;  //for version 5.0.1
    esp_err_t ret;

    switch(uart_num)
    {
        case UART_2:
        {
            //
        }
        break;

        case GPRS_UART:
        {
            uint32_t sclk_freq, intr_mask = 0;
            periph_module_enable(PERIPH_UART1_MODULE);

            //ret = uart_set_pin(UART_NUM_1, MDM_TX, MDM_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            ret = uart_set_pin(UART_NUM_1, MDM_RX, MDM_TX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            printf("set_pin1=%d", ret);

            uart_hal_init((uart_hal_context_t*)&uart_hal1, UART_NUM_1);
            
            //UART1.conf0.tick_ref_always_on = 0x01;
            uart_hal_set_sclk(&uart_hal1, src_clk);

            esp_clk_tree_src_get_freq_hz((soc_module_clk_t)src_clk, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &sclk_freq);

            uart_hal_set_baudrate(&uart_hal1, 115200UL, sclk_freq);
            
            uart_hal_rxfifo_rst(&uart_hal1);
            uart_hal_txfifo_rst(&uart_hal1);

            uart_hal_disable_intr_mask(&uart_hal1, UART_LL_INTR_MASK);
            uart_hal_clr_intsts_mask(&uart_hal1, UART_LL_INTR_MASK);

            uart_hal_set_rxfifo_full_thr(&uart_hal1, 1);

            UART1.mem_conf.tx_size = 1;
            UART1.mem_conf.rx_size = 1;

            /* intr_mask |= ((UART_INTR_RXFIFO_FULL)|(UART_INTR_TX_DONE));

            uart_hal_ena_intr_mask(&uart_hal1, intr_mask); */

            ret = esp_intr_alloc(uart_periph_signal[UART_NUM_1].irq, ESP_INTR_FLAG_IRAM,
                       uart1_intr_handle, 0,
                        &handle_console1);

            intr_mask |= ((UART_INTR_RXFIFO_FULL)/* |(UART_INTR_TX_DONE) */);

            uart_hal_ena_intr_mask(&uart_hal1, intr_mask);

            printf("intr_alloc1=%d\n", ret);

            print_uart_cfg_Regs(GPRS_UART); 
            vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
            flushUartFifo(&uart_hal1, RXTX);

        }
        break;

        case UART_PC:
        default:
        {
            uint32_t sclk_freq, intr_mask = 0;
            periph_module_enable(PERIPH_UART0_MODULE);

            ret = uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            printf("set_pin0=%d", ret);

            uart_hal_init((uart_hal_context_t*)&uart_hal0, UART_NUM_0);
            
            //UART0.conf0.tick_ref_always_on = 0x01;
            uart_hal_set_sclk(&uart_hal0, src_clk);

            esp_clk_tree_src_get_freq_hz((soc_module_clk_t)src_clk, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &sclk_freq);

            uart_hal_set_baudrate(&uart_hal0, 115200UL, sclk_freq);
            
            uart_hal_rxfifo_rst(&uart_hal0);
            uart_hal_txfifo_rst(&uart_hal0);

            uart_hal_disable_intr_mask(&uart_hal0, UART_LL_INTR_MASK);
            uart_hal_clr_intsts_mask(&uart_hal0, UART_LL_INTR_MASK);

            uart_hal_set_rxfifo_full_thr(&uart_hal0, 1);

            intr_mask |= (UART_INTR_RXFIFO_FULL);

            uart_hal_ena_intr_mask(&uart_hal0, intr_mask);

            ret = esp_intr_alloc(uart_periph_signal[UART_NUM_0].irq, ESP_INTR_FLAG_IRAM,
                       uart0_intr_handle, 0,
                        &handle_console0);

            printf("intr_alloc0=%d\n", ret);
            print_uart_cfg_Regs(UART_PC); 
            vTaskDelay(pdMS_TO_TICKS(10)); // Delay for 10 milliseconds
            flushUartFifo(&uart_hal0, RXTX);

        }
        break;
    }
}

void flushUartFifo(uart_hal_context_t* hal, rxtx_t rxtx)
{
    switch(rxtx)
    {
        case RX:
        {
            uart_hal_rxfifo_rst(hal);
        }
        break;

        case TX:
        {
            uart_hal_txfifo_rst(hal);
        }
        break;

        case RXTX:
        {
            uart_hal_rxfifo_rst(hal);
            uart_hal_txfifo_rst(hal);
        }
        break;

        default:
        break;
    }
}

void print_uart_cfg_Regs(int no) 
{
    if (!no)
    {
        int * ptr;

        ptr = (int*)UART_CONF0_REG(0);
        ESP_LOGI(UART_TAG, "UART_CONF0_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int *)UART_CONF1_REG(0);
        ESP_LOGI(UART_TAG, "UART_CONF1_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_MEM_CONF_REG(0);
        ESP_LOGI(UART_TAG, "UART_MEM_CONF_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_INT_RAW_REG(0);
        ESP_LOGI(UART_TAG, "UART_INT_RAW_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_INT_ENA_REG(0);
        ESP_LOGI(UART_TAG, "UART_INT_ENA_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_STATUS_REG(0);
        ESP_LOGI(UART_TAG, "UART_STATUS_REG(0) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_FIFO_REG(0);
        ESP_LOGI(UART_TAG, "UART_FIFO_REG(0) value: 0x%X\n", (unsigned int)*ptr);
    }
    else
    {
        int * ptr;
        /////////////////////////////////////////////////////////////////////
        ptr = (int*)UART_CONF0_REG(1);
        ESP_LOGI(UART_TAG, "UART_CONF0_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int *)UART_CONF1_REG(1);
        ESP_LOGI(UART_TAG, "UART_CONF1_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_MEM_CONF_REG(1);
        ESP_LOGI(UART_TAG, "UART_MEM_CONF_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_INT_RAW_REG(1);
        ESP_LOGI(UART_TAG, "UART_INT_RAW_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_INT_ENA_REG(1);
        ESP_LOGI(UART_TAG, "UART_INT_ENA_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_STATUS_REG(1);
        ESP_LOGI(UART_TAG, "UART_STATUS_REG(1) value: 0x%X", (unsigned int)*ptr);

        ptr = (int*)UART_FIFO_REG(1);
        ESP_LOGI(UART_TAG, "UART_FIFO_REG(1) value: 0x%X\n", (unsigned int)*ptr);
    }
}

void print_uart_fifos(void)
{
/*   int * ptr;

  ptr = (int*)UART_FIFO_AHB_REG(0);
  ESP_LOGI(UART_TAG, "UART_FIFO_AHB_REG(0) value: 0x%X", (unsigned int)*ptr);

  ptr = (int*)UART_FIFO_REG(0);
  ESP_LOGI(UART_TAG, "UART_FIFO_REG(0) value: 0x%X", (unsigned int)*ptr);

  ptr = (int*)UART_STATUS_REG(0);
  ESP_LOGI(UART_TAG, "UART_STATUS_REG(0) value: 0x%X", (unsigned int)*ptr);
 */
  printf("\nUART0.status.rxfifo_cnt: 0x%X", UART0.status.rxfifo_cnt);
  printf("\nUART0.status.txfifo_cnt: 0x%X\n", UART0.status.txfifo_cnt);

  printf("\nUART1.status.rxfifo_cnt: 0x%X", UART1.status.rxfifo_cnt);
  printf("\nUART1.status.txfifo_cnt: 0x%X\n", UART1.status.txfifo_cnt);

  /* ptr = (int*)UART_MEM_CNT_STATUS_REG(0);
  ESP_LOGI(UART_TAG, "UART_MEM_CNT_STATUS_REG(0) value: 0x%X", (unsigned int)*ptr);
 */
/* 
  printf("\nUART0.mem_cnt_status.rx_cnt: 0x%X", UART0.mem_cnt_status.rx_cnt);
  printf("\nUART0.mem_cnt_status.tx_cnt: 0x%X\n", UART0.mem_cnt_status.tx_cnt);
 */ 
}

#if (SETUPA_testing == 1) && (SETUPB_testing == 0)  //PP commented on 11-10-23, now rewriting these fubctions so that they are safe for calling in an interrupt handler with IRAM_ATTR
void uart1_rx_intr(void)
{
    char tmpudr;
	tmpudr = READ_PERI_REG(UART_FIFO_AHB_REG(1));
    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), tmpudr);

    gprs_rx_isr_handler.state = GPRS_RX_INPROG;
	
	gprs_rx_isr_handler.elapsed = 0;
    if (gprs_rx_buff.index < GPRS_RX_BUFFER_MAX)
    {
        gprs_rx_buff.buffer[gprs_rx_buff.index++] = tmpudr;
    }
    //uart_clear_intr_status(UART_NUM_1, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
    UART1.int_clr.val |= (UART_RXFIFO_FULL_INT_CLR);
    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), tmpudr);
    //printf("P");
    flushUartFifo(&uart_hal1, RX);
    test_uart1_intr_rx_flg = true;
}

void uart1_tx_intr(void)
{
    uint32_t intr_mask = UART_INTR_TX_DONE;
    gprs_tx_buff.index++;
    if ((gprs_tx_buff.index >= GPRS_TX_BUFFER_MAX) || 
    (gprs_tx_buff.buffer[gprs_tx_buff.index] == '\0')) 
    {
		unlock(gprs_tx_buff.locked);
        //clear intr mask
        //uart_clear_intr_status(UART_NUM_1, UART_INTR_TX_DONE);
        UART1.int_clr.val |= (UART_INTR_TX_DONE);
        uart_hal_disable_intr_mask(&uart_hal1,UART_INTR_TX_DONE);
        flushUartFifo(&uart_hal1, TX);
        //UART1.int_clr.tx_done = 1;
    } 
    else 
    {
        flushUartFifo(&uart_hal1, TX);
        WRITE_PERI_REG(UART_FIFO_AHB_REG(1), gprs_tx_buff.buffer[gprs_tx_buff.index]);
        //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), gprs_tx_buff.buffer[gprs_tx_buff.index]);
    }
    test_uart1_intr_tx_flg = true;
}

#elif (SETUPB_testing == 1)

void IRAM_ATTR uart1_rx_intr(void)
{
    char tmpudr;
	tmpudr = READ_PERI_REG(UART_FIFO_AHB_REG(1));   //PP(11-10-23): W/R to a memory mapped var is IRAM_ATTR safe.

    gprs_rx_isr_handler.state = GPRS_RX_INPROG; //PP(11-10-23): global variables are in data memory, not flash so they're IRAM_ATTR safe.
	
	gprs_rx_isr_handler.elapsed = 0;

    if (gprs_rx_buff.index < GPRS_RX_BUFFER_MAX)
    {
        gprs_rx_buff.buffer[gprs_rx_buff.index++] = tmpudr;
    }

    UART1.int_clr.val |= (UART_RXFIFO_FULL_INT_CLR);    //PP(11-10-23): this variable is memory mapped, so it is IRAM_ATTR safe.

    //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), tmpudr);
    //printf("P");

    //flushUartFifo(&uart_hal1, RX);
    //uart_hal_rxfifo_rst(&uart_hal1);  //PP(11-10-23): this function is defined uart_hal_iram.c, according to the docs, it is IRAM_ATTR safe.
    uart_ll_rxfifo_rst(uart_hal1.dev);

    test_uart1_intr_rx_flg = true;
}

void IRAM_ATTR uart1_tx_intr(void)
{
    //uint32_t intr_mask = UART_INTR_TX_DONE;   //PP(11-10-23): declaring this and not using it may cause compiler to treat it as a const and store it in flash, causing it to be IRAM_ATTR unsafe.
    gprs_tx_buff.index++;

    if ((gprs_tx_buff.index >= GPRS_TX_BUFFER_MAX) || 
    (gprs_tx_buff.buffer[gprs_tx_buff.index] == '\0')) 
    {
		unlock(gprs_tx_buff.locked);    //PP(11-10-23): Macros are'nt stored in flash so they're IRAM_ATTR safe.

        //clear intr mask
        UART1.int_clr.val |= (UART_INTR_TX_DONE); //PP(11-10-23): this variable is memory mapped, so it is IRAM_ATTR safe.

        //disable tx intr in order to be safe from recursion inducing conditions.
        uart_hal_disable_intr_mask(&uart_hal1,UART_INTR_TX_DONE); //PP(11-10-23): this macro calls a func with the attribute "static inline __attribute__((always_inline))" which is IRAM_ATTR safe according to the docs.
        
        //flushUartFifo(&uart_hal1, TX);
        //uart_hal_txfifo_rst(&uart_hal1); //PP(11-10-23): this function is defined uart_hal_iram.c, according to the docs, it is IRAM_ATTR safe.
        uart_ll_txfifo_rst(uart_hal1.dev);
    } 
    else 
    {
        //flush hw tx fifo in order to prevent fifo overflow.
        //flushUartFifo(&uart_hal1, TX);
        //uart_hal_txfifo_rst(&uart_hal1); //PP(11-10-23): this function is defined uart_hal_iram.c, according to the docs, it is IRAM_ATTR safe.
        uart_ll_txfifo_rst(uart_hal1.dev);
        
        WRITE_PERI_REG(UART_FIFO_AHB_REG(1), gprs_tx_buff.buffer[gprs_tx_buff.index]); //PP(11-10-23): W/R to a memory mapped var is IRAM_ATTR safe.
        //WRITE_PERI_REG(UART_FIFO_AHB_REG(0), gprs_tx_buff.buffer[gprs_tx_buff.index]);
    }
    test_uart1_intr_tx_flg = true;
}
#endif

void UWriteData(char data, int no)
{
	switch(no)
	{
#ifdef UART_PC	
		case UART_PC :
        {
            /* uint32_t tx_fifo_cnt = UART0.status.txfifo_cnt;
            while(tx_fifo_cnt); */
            WRITE_PERI_REG(UART_FIFO_AHB_REG(0), data);

        }			
		break;
#endif
#ifdef UART_PC	
		case GPRS_UART :
        {
            /* uint32_t tx_fifo_cnt = UART1.status.txfifo_cnt;
            while(tx_fifo_cnt); */
            WRITE_PERI_REG(UART_FIFO_AHB_REG(1), data);
        }			
		break;
#endif
#ifdef UART_PC
		case UART_2 :
			//
		break;
#endif
        default:
        break;
	}
}