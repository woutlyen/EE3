/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_as608_interface_template.c
 * @brief     driver as608 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-09-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/09/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_as608_interface.h"

#include "stdio.h"
#include "stm8s.h"

uint8_t rx_buffer[384];
uint8_t rx_used = 0;
uint8_t rx_cur = 0;

INTERRUPT_HANDLER(UART2_RX_IRQHandler, 21){
	/*
uint8_t rb;
	if(UART2_SR & UART2_SR_RXNE_OR){ // data received
		rb = UART2_DR; // read received byte & clear RXNE flag
		while(!(UART2_SR & UART_SR_TXE));
		rx_buffer[rx_cur++] = rb; // put received byte into cycled buffer
		if(rx_cur == 500){ // Oops: buffer overflow! Just forget old data
			rx_cur = 0;
		}
	}*/
	rx_buffer[rx_cur] = UART2_ReceiveData8();
	UART2_ClearITPendingBit(UART2_IT_RXNE);
	UART2_ClearFlag(UART2_FLAG_RXNE);
	
	if(rx_buffer[rx_cur] == 0x55){
		rx_buffer[rx_cur] = 0;
	}
	else{
		rx_cur++;
		if(rx_cur == 384){
			rx_cur = 0;
		}
		//UART2_SendData8(rx_buffer[rx_cur-1]);
	}
}


//#include <stdio.h> // For sprintf function

#define DEBUG_BUFFER_SIZE 128

/**
 * @brief  interface uart init
 * @return status code
 *         - 0 success
 *         - 1 uart init failed
 * @note   none
 */
uint8_t as608_interface_uart_init(void)
{
	UART2_DeInit(); //Deinitialize UART peripherals 
                
	UART2_Init(57600, 
                UART2_WORDLENGTH_8D, 
                UART2_STOPBITS_1, 
                UART2_PARITY_NO, 
                UART2_SYNCMODE_CLOCK_DISABLE, 
                UART2_MODE_TXRX_ENABLE);
								
	UART2_ITConfig(UART2_IT_RXNE_OR, ENABLE);
	enableInterrupts();
	UART2_Cmd(ENABLE);
	
	as608_interface_debug_print("as608: get info failed.\n");
    return 0;
}

/**
 * @brief  interface uart deinit
 * @return status code
 *         - 0 success
 *         - 1 uart deinit failed
 * @note   none
 */
uint8_t as608_interface_uart_deinit(void)
{
		UART2_DeInit();
    return 0;
}

/**
 * @brief      interface uart read
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failedte
 * @note       none
 */
uint16_t as608_interface_uart_read(uint8_t *buf, uint16_t len)
{
	/*
		uint16_t i;
    for (i = 0; i < len; i++)
    {
        //while (UART2_GetFlagStatus(UART2_FLAG_RXNE) == RESET)
        //{}
				
				while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
				UART2_ClearFlag(UART2_FLAG_RXNE);
        buf[i] = UART2_ReceiveData8();
    }*/
		
		uint16_t i;
		if(len >= 384){
			for(i = 0; i < 384; i++){
				if(rx_used == rx_cur){
					return i;
				}
				if(rx_used < 384){
					buf[i]=rx_buffer[rx_used];
					rx_buffer[rx_used] = 0;
					rx_used++;
				}
				else{
					buf[0]=rx_buffer[0];
					rx_buffer[rx_used] = 0;
					rx_used = 0;
				}
				//as608_interface_uart_write(&buf[i], 1);
			}
		}
		else{
			for(i = 0; i < len; i++){
				if(rx_used == rx_cur){
					return i;
				}
				if(rx_used < len){
					buf[i]=rx_buffer[rx_used];
					rx_buffer[rx_used] = 0;
					rx_used++;
				}
				else{
					buf[0]=rx_buffer[0];
					rx_buffer[rx_used] = 0;
					rx_used = 0;
				}
			}
		}
		
    return len;
}

/**
 * @brief  interface uart flush
 * @return status code
 *         - 0 success
 *         - 1 uart flush failed
 * @note   none
 */
uint8_t as608_interface_uart_flush(void)
{
		while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET)
    {}
    return 0;
}

/**
 * @brief     interface uart write
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t as608_interface_uart_write(uint8_t *buf, uint16_t len)
{
		uint16_t i;
    for (i = 0; i < len; i++)
    {
        UART2_SendData8(buf[i]);
        while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET)
        {}
    }
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void as608_interface_delay_ms(uint32_t ms)
{
	uint32_t i;
	for (i = 0; i < ms; i++)
	{
		// Delay approximately 1ms at 16MHz CPU clock
		//__asm__("NOP");
		//__asm__("NOP");
		uint16_t j;
		for (j = 0; j < 285; j++)
		{
			nop();
		}
		//as608_interface_debug_print("delay\n");
	}
}

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
void as608_interface_debug_print(const char *const string, ...)
{
    char i=0;
    while (string[i] != 0x00)
    {
      UART2_SendData8(string[i]);
      while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
        i++;
		}

}

void Serial_print_string (char string[])
 {
		char i=0;
    while (string[i] != 0x00)
    {
      UART2_SendData8(string[i]);
      while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
        i++;
		}
 }