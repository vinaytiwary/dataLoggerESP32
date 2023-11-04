#ifndef UART_H_
#define UART_H_

#include "hal/uart_hal.h"
#include "hal/uart_ll.h"

#define UART_PC (0)
#define GPRS_UART (1)
#define UART_2 (2)

#define BUF_SIZE (128)
#define MDM_TX    (13)
#define MDM_RX     (4)

#define TX_BUFFER_MAX (64)
#define RX_BUFFER_MAX (64)

static const char *UART_TAG = "uart_events";

// static uart_hal_context_t uart_hal0 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_0),};
// static uart_hal_context_t uart_hal1 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_1),};
// static uart_hal_context_t uart_hal2 = {.dev = (uart_dev_t *)UART_LL_GET_HW(UART_NUM_2),};

typedef enum
{
    RX,
    TX,
    RXTX
}rxtx_t;

typedef enum
{
	START,
	PROCESS,
	READY,
	READ_LEN,
	DATA_READ,
	EOP,
	LRC_READ,
}ISR_rx_state_t;

typedef struct  
{
	unsigned char rx_buffer[RX_BUFFER_MAX];
	int rx_indx;
	//rx_msg_state_t rx_state;
	unsigned int elapsed;
	ISR_rx_state_t rx_state;
}Rx_Buff_t;

typedef struct
{
	char tx_buffer[TX_BUFFER_MAX];
	int tx_indx;
	char tx_ready;
	char pending_command;
	unsigned long curr_pck_num;
	char wait_for_ack;
}Tx_Buff_t;

/* 
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
 */


void initUart(int);
void flushUartFifo(uart_hal_context_t* , rxtx_t);
void print_uart_cfg_Regs(int no);
void print_uart_fifos(void);
void uart1_rx_intr(void);
void uart1_tx_intr(void);
void UWriteData(char data,int no);

#endif