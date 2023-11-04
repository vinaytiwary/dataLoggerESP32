#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_comm.h"
#include "UART.h"
#include "gprs.h"
#include "config.h"
#include "Common.h"
#include "gps.h"
#include "web_comm.h"
#include "_debug.h"
#include "error.h"

extern uart_hal_context_t uart_hal0;
extern uart_hal_context_t uart_hal1;
extern uart_hal_context_t uart_hal2;

extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3];

extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;

extern volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;   //16-2-19 VC: for dynamic allocation
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

//extern gps_state_t gps_handler_state;
extern gps_statuses_t gps_statuses;
extern conn_state_t conn_state;

/* extern volatile Rx_Buff_t Rx_Buff[3];
extern volatile Tx_Buff_t Tx_Buff[3]; */

unsigned char FRAME_TIMEOUT[3] = {DEFAULT_FRAME_TIMEOUT, GPRS_FRAME_TIMEOUT, DEFAULT_FRAME_TIMEOUT};

void UWriteBytes(unsigned char *str, int len, int no)
{	
	while(len)
	{
		UWriteData(*str, no);
		str++;
		len--;
	}
}
void UWriteInt(unsigned long num, char no)
{
	char temp[30];
	my_ltoa(num, temp, 10);
	UWriteString((char*)temp, no);
}

char UWriteString(char *str, int no)		// char for GPRS
{
	char retVal = 0;
    if(no == GPRS_UART)
	{
		//char tempstr[BUFFER_MAX];		//Anand 07-04-16 why??
		if (str[0] == '\0') retVal = IS_SUCCESS;
		else
		{
			uint32_t intr_mask = 0;

            /* uint32_t tx_fifo_cnt = UART0.status.txfifo_cnt;
            if(!tx_fifo_cnt) */
            {
                if(strlen(str) < GPRS_TX_BUFFER_MAX)
				{
					strcpy((char*)gprs_tx_buff.buffer, (const char*)str);
				}
				else
				{
					strcpy((char*)gprs_tx_buff.buffer, (const char*)"BUFFER SIZE ERROR");
				}
				/* enable_tx_intr(GPRS_UART);
				GPRS_UDR = gprs_tx_buff.buffer[0]; */
                /* gprs_tx_buff.index = 0;
				WRITE_PERI_REG(UART_FIFO_AHB_REG(0), gprs_tx_buff.buffer[0]);
                WRITE_PERI_REG(UART_FIFO_AHB_REG(1), gprs_tx_buff.buffer[0]); */

				/* while((*str)!='\0')
				{
					UWriteData(*str,no);
					UWriteData(*str,0);
					str++;
				}
				UWriteData('\0',no); */

				//printf("\ngi=%d",gprs_tx_buff.index);

				flushUartFifo(&uart_hal1, TX);

				intr_mask |= (/* (UART_INTR_RXFIFO_FULL)| */(UART_INTR_TX_DONE));

				//uart_hal_ena_intr_mask(&uart_hal1, intr_mask);

				gprs_tx_buff.index = 0;
				//WRITE_PERI_REG(UART_FIFO_AHB_REG(0), gprs_tx_buff.buffer[0]);
                WRITE_PERI_REG(UART_FIFO_AHB_REG(1), gprs_tx_buff.buffer[0]);

				uart_hal_ena_intr_mask(&uart_hal1, intr_mask);

				retVal = IS_SUCCESS;
            }
        }
    }
	else
	{
		while((*str)!='\0')
		{
			UWriteData(*str,no);
			str++;
		}
		UWriteData('\0',no);
	}
	
	return retVal;		//Anand 07-04-16
}

void checkforUARTFrameTimeout(int uart_no)
{
	if(uart_no == GPRS_UART)	//PP 29-06-23: added else if instead of if
	{		
		if(gprs_rx_isr_handler.elapsed >= FRAME_TIMEOUT[uart_no])
		{
			gprs_rx_isr_handler.elapsed = 0;

			if(strstr((char*)gprs_rx_buff.buffer, "+CME ERROR: SIM not inserted"))
			{
				//setSIMSts(NOT_AVBL);
				set_system_state(SYS_GSM_SIM, HIGH);
			}

			/* if((getFdmState() == FDM_GPRS_CONFIG) || (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG))
			{
				if((gprs_temp_rx_buff.index > 0))
				{
					gprs_temp_rx_buff.locked = LOCKED;
					gprs_rx_isr_handler.state = GPRS_RX_IDLE;
				}
			}
			else */
#ifdef COMBINE_GGA_GNSS_CMD
			if(((gps_statuses.gps_handler_state == GPS_CMD_GGA_GNSS) || (gps_statuses.gps_handler_state == GPS_RSP_GGA_GNSS) || (gps_statuses.gps_handler_state == GPS_CMD_GPGGA_STOP)) 
				&& gps_statuses.gps_handler_state != GPS_RSP_GPGGA_STOP && conn_state == CONNECT_LOCATION)
			{
				if((gprs_rx_buff.index > 0) && (count_comma((char*)gprs_rx_buff.buffer) == 29))
				{
					gprs_rx_buff.locked = LOCKED;
					gprs_rx_isr_handler.state = GPRS_RX_IDLE;
#ifdef DEBUG_GGA_GNSS_COMB
					UWriteString((char*)"\nFT2:", UART_PC);
					UWriteString((char*)"\nRx_i:", UART_PC);
					//UWriteInt(strlen((char*)gprs_rx_buff.buffer), UART_PC);
					UWriteInt(gprs_rx_buff.index, UART_PC);
					UWriteString((char *)"\nRx_L", UART_PC);
					UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
					/* UWriteString((char*)"\nRx_i:", UART_PC);
					//UWriteInt(strlen((char*)gprs_rx_buff.buffer), UART_PC);
					UWriteInt(gprs_rx_buff.index, UART_PC); */
#endif
				}	
			}
			else
#endif
			{
				if(gprs_rx_buff.index > 0)
				{
					gprs_rx_buff.locked = LOCKED;
					gprs_rx_isr_handler.state = GPRS_RX_IDLE;
#ifdef DEBUG_GGA_GNSS_COMB
					UWriteString((char*)"\nFT1:", UART_PC);
					UWriteString((char*)"\nRx_i:", UART_PC);
					//UWriteInt(strlen((char*)gprs_rx_buff.buffer), UART_PC);
					UWriteInt(gprs_rx_buff.index, UART_PC);
					UWriteString((char *)"\nRx_L", UART_PC);
					UWriteString((char*)gprs_rx_buff.buffer, UART_PC);
#endif
				}
			}
		}
	}
}


void flushTxBuffer(int no)
{
	if(no == GPRS_UART)
	{
		flushUartFifo(&uart_hal1, TX);
		memset((void*)&gprs_tx_buff, 0, sizeof(gprs_tx_data_buff_t));
	}
	else
	{
		flushUartFifo(&uart_hal0, TX);
		memset((void*)&Tx_Buff[no], 0, sizeof(Tx_Buff[no]));
	}

}
void flushRxBuffer(int no)
{
	if(no == GPRS_UART)
	{
		flushUartFifo(&uart_hal1, RX);
		memset((void*)&gprs_rx_isr_handler, 0, (sizeof(gprs_rx_isr_handler_t)));
		memset((void*)&gprs_rx_buff, 0, (sizeof(gprs_rx_data_buff_t)));
		if (gprs_temp_rx_buff.buff != NULL)
		{
			//memset((char *)gprs_temp_rx_buff.buff, 0 , GPRS_TEMP_RX_BUFFER_MAX);
			memset((char *)gprs_temp_rx_buff.buff, 0 , GPRS_RX_BUFFER_MAX);
		}
	}
	else
	{
		flushUartFifo(&uart_hal0, RX);
		memset((void*)&Rx_Buff[no], 0, (sizeof(Rx_Buff[no])));
	}
}