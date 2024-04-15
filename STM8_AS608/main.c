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
#include "driver_as608_fingerprint_test.h"
#include "driver_as608_interface.h"
#include "stdio.h"

void Timer_init(void);
void SPI_init(void);
void IO_init(void);
void init_interrupts(void);
void sendDataSPI(uint8_t data);
void init(void);

//#define STM8S105


// Init TIM2
void Timer_init(){		
	TIM2_DeInit();
	TIM2_TimeBaseInit(TIM2_PRESCALER_128, 0x3D08);
  TIM2_Cmd(ENABLE);
	TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
}

// Init SPI
void SPI_init(){
	GPIOC->DDR = 0x60;
	GPIOC->CR1 = 0xE0;
	SPI_DeInit();
	SPI_Init(SPI_FIRSTBIT_LSB, 
			SPI_BAUDRATEPRESCALER_16 ,
			SPI_MODE_MASTER, 
			SPI_CLOCKPOLARITY_LOW, 
			SPI_CLOCKPHASE_1EDGE, 
			SPI_DATADIRECTION_2LINES_FULLDUPLEX, 
			SPI_NSS_SOFT, 
			SPI_CRC_TX);
	SPI_Cmd(ENABLE);
}

// Init Port D
void IO_init(){
	GPIOD->DDR = 0xff;
	GPIOD->CR1 = 0xff;
}

// Init interrupt
void init_interrupts(){
  SPI->ICR = 0x40;
	enableInterrupts();
}

// Init all the different components
void init(){
	IO_init();
	SPI_init();
	Timer_init();
	init_interrupts();
}



 
void main(void){
  //init();
  while (1){
		
		//nop();
		as608_interface_uart_init();
		as608_fingerprint_test(0xFFFFFFFF);
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
