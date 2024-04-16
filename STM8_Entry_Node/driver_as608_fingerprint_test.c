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
 * @file      driver_as608_fingerprint_test.c
 * @brief     driver as608 fingerprint test source file
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

#include "driver_as608_fingerprint_test.h"
#include "stm8s105_lcd_16x2.h"
#include "driver_as608.h"
#include "stm8_nrf24.h"

static as608_handle_t gs_handle;        /**< as608 handle */
//static uint8_t gs_buffer[49152];    /**< inner buffer */
static uint8_t gs_buffer[384];    /**< inner buffer */
int voice_assistent_started = 0;

/**
 * @brief     fingerprint test
 * @param[in] addr is the chip address
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
 
 @svlreg
 INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, 15)
		{
				TIM3_ClearFlag(TIM3_FLAG_UPDATE);
        TIM3_Cmd(DISABLE);
				if(voice_assistent_started == 1) {
					TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 61);
					voice_assistent_started = 0;
					Lcd_Clear();
					Lcd_Set_Cursor(1,1);
					Lcd_Print_String("Time-out");
					Lcd_Set_Cursor(2,1);
					Lcd_Print_String("Try again!");
			  } else if(voice_assistent_started == 2) {
					GPIOC->ODR &= ~(1<<2);
					TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 61);
					voice_assistent_started = 0;
					Lcd_Clear();
					Lcd_Set_Cursor(1,1);
					Lcd_Print_String("Put a finger on ");
					Lcd_Set_Cursor(2,1);
					Lcd_Print_String("the sensor..");
				} else {
					as608_interface_uart_init();
					as608_fingerprint_test(0xFFFFFFFF);
				}
        TIM3_Cmd(ENABLE);
		}
		
		
uint8_t as608_fingerprint_test(uint32_t addr)
{
	
		uint8_t res;
    uint8_t j;
    uint8_t table[32];
    uint16_t number;
    uint16_t score;
    uint16_t page_number;
    uint16_t found_page;
    uint16_t output_len;
    uint16_t i;
    uint32_t timeout;
    as608_info_t info;
    as608_status_t status;
		volatile datagram finger_datagram;
		
    /* link interface function */
    DRIVER_AS608_LINK_INIT(&gs_handle, as608_handle_t);
    DRIVER_AS608_LINK_UART_INIT(&gs_handle, as608_interface_uart_init);
    DRIVER_AS608_LINK_UART_DEINIT(&gs_handle, as608_interface_uart_deinit);
    DRIVER_AS608_LINK_UART_READ(&gs_handle, as608_interface_uart_read);
    DRIVER_AS608_LINK_UART_WRITE(&gs_handle, as608_interface_uart_write);
    DRIVER_AS608_LINK_UART_FLUSH(&gs_handle, as608_interface_uart_flush);
    DRIVER_AS608_LINK_DELAY_MS(&gs_handle, as608_interface_delay_ms);
    DRIVER_AS608_LINK_DEBUG_PRINT(&gs_handle, as608_interface_debug_print);
		
    /* get as608 information */
    res = as608_info(&info);
    if (res != 0)
    {
        as608_interface_debug_print("as608: get info failed.\n");
       
        return 1;
    }
		
		/* start fingerprint test */
		as608_interface_debug_print("\n\n\n");
    as608_interface_debug_print("as608: start fingerprint test.\n");
		as608_interface_debug_print("\n\n\n");
		
    /* as608 init */
    res = as608_init(&gs_handle, addr);
    if (res != 0)
    {
        as608_interface_debug_print("as608: init failed.\n");
        return 1;
    }

		/* output */
		as608_interface_debug_print("\n\n\n");
    as608_interface_debug_print("as608: please put your finger on the sensor.\n");
		as608_interface_debug_print("\n\n\n");
	
    /* Check image */

				/* get image */
        res = as608_get_image(&gs_handle, addr, &status);
        if (res != 0)
        {
            as608_interface_debug_print("as608: get image failed.\n");
            (void)as608_deinit(&gs_handle);
            
            return 1;
        }
        if (status == AS608_STATUS_OK)
        {
            /* generate feature */
						
            res = as608_generate_feature(&gs_handle, addr, AS608_BUFFER_NUMBER_1, &status);
            if (res != 0)
            {
                as608_interface_debug_print("as608: generate feature failed.\n");
                (void)as608_deinit(&gs_handle);
                
                return 1;
            }
            if (status == AS608_STATUS_OK)
            {
                /* search */
                res = as608_search_feature(&gs_handle, addr, AS608_BUFFER_NUMBER_1, 
                                           0, 300, &found_page, 
                                           &score, &status);
                if (res != 0)
                {
                    as608_interface_debug_print("as608: search feature failed.\n");
                    (void)as608_deinit(&gs_handle);
                    
                    return 1;
                }
                if (status == AS608_STATUS_OK)
                {
										char page[16];
										char score[16];
                    //as608_interface_debug_print("as608: found in the page %d and score is %d.\n", found_page, score);
										as608_interface_debug_print("\n\n\n\n\n\n");
										as608_interface_debug_print("as608: found in the page ");
										sprintf(&page[0], "%d", found_page);
										as608_interface_debug_print(&page[0]);
										as608_interface_debug_print(" and score is ");
										sprintf(&score[0], "%d", score);
										as608_interface_debug_print(&score[0]);
										as608_interface_debug_print(".\n");
										as608_interface_debug_print("\n\n\n\n\n\n");
										Lcd_Clear();
										Lcd_Set_Cursor(1,1);
										Lcd_Print_String("Finger matched");
										
										finger_datagram.dev = 0x08;
										finger_datagram.command = 0x01;
										finger_datagram.data_byte0 = 0x00;
										finger_datagram.data_byte1 = 0x00;
										finger_datagram.status = 0x00;
										
										send_data_nrf(finger_datagram, 0);
										
										TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 915);
										voice_assistent_started = 1;
										
                    return 0;
                }
                else
                {
										as608_interface_debug_print("\n\n\n\n\n\n");
                    (void)as608_print_status(&gs_handle, status);
										as608_interface_debug_print("\n\n\n\n\n\n");
                    (void)as608_deinit(&gs_handle);
									
                    return 1;
                }
            }
            else
            {
                (void)as608_print_status(&gs_handle, status);
                (void)as608_deinit(&gs_handle);
                
                return 1;
            }
        }
}
