/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    16-June-2017
  * @brief   Main program body
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s_tim3.h"
#include "driver_as608_fingerprint_test.h"
#include "driver_as608_interface.h"
#include "stm8s105_LCD_16x2.h"
#include "stm8_nrf24.h"
#include "stdio.h"

/* Private function prototypes -----------------------------------------------*/
void GPIO_Configuration(void);
void TIM3_Configuration(void);

/* Private functions ---------------------------------------------------------*/
INTERRUPT_HANDLER(EXTI_PORTC_IRQHandler, 5)
{
  /**
  * Author: Robbe Decapmaker <debber@dcpm.be>
  * Co-author: Kobe Michiels <kobe.michiels2@student.kuleuven.be>
  * Team: HOME2
  */
	volatile uint8_t status_contents = 0;	
	volatile uint8_t i = 0;
	volatile datagram datagram_buffer;
	
	status_contents = nrf24_update_status();
	
	if(status_contents != 0x42){
		// status register -- resetting the IRQ of the NRF
		nrf24_write_register(0x07, 0x40);
		nrf24_write_register(0x07, 0x30);
		return;
	}
	
	datagram_buffer = nrf24_read_payload();

	nrf24_flush_rx();
  // status register -- resetting the IRQ of the NRF
	nrf24_write_register(0x07, 0x40);

  if((datagram_buffer.dev == 0x08) && (voice_assistent_started == 1)) {
		TIM3_Cmd(DISABLE);
		TIM3_DeInit();
		TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 183);
		TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
    GPIOC->ODR |= (1 << 2);
		Lcd_Clear();
		Lcd_Set_Cursor(1,1);
		Lcd_Print_String("Door unlocked!");
		voice_assistent_started = 2;
		TIM3_Cmd(ENABLE);
  }

	return;
}

void GPIO_Configuration() {
  /* Init the solenoid port */
  GPIO_DeInit(GPIOC);
	GPIO_Init(GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_SLOW);
}

void TIM3_Configuration() {
  /* DeInit TIM3 */
  TIM3_DeInit();

  /* Configure TIM3 */
  TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 61);

  // Enable TIM3 update interrupt
	TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
}

void main(void) {

  /* Init GPIO */
  GPIO_Configuration();

  /* Init TIM3 */
  TIM3_Configuration();

  /* Init LCD */
	Lcd_Begin();
  Lcd_Clear();
	Lcd_Set_Cursor(1,1);
	Lcd_Print_String("Put a finger on ");
  Lcd_Set_Cursor(2,1);
  Lcd_Print_String("the sensor..");
	
	as608_interface_uart_init();
	as608_fingerprint_test(0xFFFFFFFF);
	
  /* NRF24 library setup */
  nrf24_setup_bus();
	setup_interupt();
	config_nrf();

  GPIOC->CR2 = 0x00;
	GPIOC->ODR = 0xFB;

  GPIOC->CR2 = ((1 << 4)| GPIOC->CR2);
	GPIOC->DDR &= ~(1<<4);
	GPIOC->CR1 &= ~(1<<4);
	
	GPIOC->DDR |= (1 << 2);
	GPIOC->CR1 |= (1 << 2);
	GPIOC->CR2 &= ~(1<<2);

	/* enable interrupts */
	enableInterrupts();
	
	TIM3_Cmd(ENABLE);
	
  while (1) {
  }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
