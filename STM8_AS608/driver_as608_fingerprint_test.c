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
#include "driver_as608.h"

static as608_handle_t gs_handle;        /**< as608 handle */
//static uint8_t gs_buffer[49152];    /**< inner buffer */
static uint8_t gs_buffer[384];    /**< inner buffer */

/**
 * @brief     fingerprint test
 * @param[in] addr is the chip address
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
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
    else
    {
        /* print chip information */
				/*
        as608_interface_debug_print("as608: chip is %s.\n", info.chip_name);
        as608_interface_debug_print("as608: manufacturer is %s.\n", info.manufacturer_name);
        as608_interface_debug_print("as608: interface is %s.\n", info.interface);
        as608_interface_debug_print("as608: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        as608_interface_debug_print("as608: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        as608_interface_debug_print("as608: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        as608_interface_debug_print("as608: max current is %0.2fmA.\n", info.max_current_ma);
        as608_interface_debug_print("as608: max temperature is %0.1fC.\n", info.temperature_max);
        as608_interface_debug_print("as608: min temperature is %0.1fC.\n", info.temperature_min);
				*/
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
    
    /* delay 2000ms */
    as608_interface_delay_ms(2000);
    
    /* max 10s */
    timeout = 10;
    
    /* wait your finger */
    while (timeout != 0)
    {
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
                    break;
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
        
        /* delay 1000ms */
        as608_interface_delay_ms(1000);
        
        /* timeout-- */
        timeout--;
    }
}
