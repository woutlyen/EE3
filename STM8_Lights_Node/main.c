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

 /*
    author: Ricky Ao, Sebastiaan Schoeters, Lander Van Loock, Jaskaran
    Team: Home 3 
    Description: this file is the main.c of light node
    It has non-dimmable led, dimmable led and rgb led (PWM)
    It uses 5-byte message for nrf24 wireless communication
    It has motion sensor and temperature sensor

 */
 
	
	
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8_nrf24.h"
#include "temp_sensor.h"

uint8_t r, g, b;
uint16_t red, green, blue;

void gpioSetup(void);
void color_init(void);
void LED_on(void);
void LED_off(void);
void init_dim_LED(void);
void dim_LED(uint8_t);
void RGB_init(void);
void change_color(uint8_t, uint8_t, uint8_t);
void change_brightness(uint16_t);
void motionSetup(void);
void sendAck(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t status_contents);



void gpioSetup(void)
{
		GPIO_DeInit(GPIOB);
		GPIO_DeInit(GPIOC);
		GPIO_DeInit(GPIOD);
		GPIO_Init(GPIOB, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST);
		GPIO_Init(GPIOB, GPIO_PIN_1, GPIO_MODE_IN_PU_IT);
		GPIO_Init(GPIOB, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
		GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_FAST);
	  GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
	  GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST);
		
}

void color_init()
{
	red = 0;
	green = 0;
	blue = 0;
}

void LED_on() 
{
	GPIO_WriteHigh(GPIOB, GPIO_PIN_2);
}

void LED_off() 
{
	GPIO_WriteLow(GPIOB, GPIO_PIN_2);
}


void init_dim_LED()
{
	//GPIO_DeInit(GPIOC);
	TIM1_DeInit();
	
	TIM1_TimeBaseInit(0, TIM1_COUNTERMODE_UP, 100, 0);
	TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE,
	TIM1_OUTPUTNSTATE_ENABLE, 0,
	TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_HIGH,
	TIM1_OCIDLESTATE_SET,
  TIM1_OCNIDLESTATE_RESET);
	
	TIM1_Cmd(ENABLE);
	TIM1_CtrlPWMOutputs(ENABLE);
}

void dim_LED(uint8_t brightness)
{
	TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE,
	TIM1_OUTPUTNSTATE_ENABLE, brightness,
	TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH,
	TIM1_OCIDLESTATE_SET,
  TIM1_OCNIDLESTATE_RESET);
}

void RGB_init()
{

	color_init();
	
	//Initialization for Timer 2
	TIM2_DeInit();
	TIM2_TimeBaseInit(TIM2_PRESCALER_1, 255);
	
	//Initialization of Timer channels
	//Channel 1 (RED), Output PWM signal on pin D4
	TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 
	red, TIM2_OCPOLARITY_LOW);
	
	//Channel  3 (GREEN), Output PWM signal on pin D2
	TIM3_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,
	green, TIM2_OCPOLARITY_LOW);
	
	//Channel 2 (BLUE), Output PWM signal on pin D3
	TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 
	blue, TIM2_OCPOLARITY_LOW);
	
	TIM2_Cmd(ENABLE);
}

void change_color(uint8_t newRed, uint8_t newGreen, uint8_t newBlue)
{
	red = newRed;
	green = newGreen;
	blue = newBlue;
	
	//Changing color by re-initializing Timer Channels with new input values
	TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 
	newRed, TIM2_OCPOLARITY_LOW);
	TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,
	newBlue, TIM2_OCPOLARITY_LOW);
	TIM2_OC2Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 
	newGreen, TIM2_OCPOLARITY_LOW);
}

void change_brightness(uint16_t brightness)
{
	change_color(red*brightness/100, green*brightness/100, blue*brightness/100);
}

void motionSetup()
{
  EXTI_DeInit();
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_ONLY);
}


void sendAck(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5,uint8_t status_contents)
{
   volatile uint8_t  i=0;
   volatile datagram msg;
   msg.dev = b1;
	 msg.command = b2;
	 msg.data_byte0 = b3;
	 msg.data_byte1 = b4;
	 msg.status = b5;
	
	 for( status_contents = 0; status_contents < 250; status_contents++){
        for( i = 0; i < 20; i++){
            nop();
        }
    }
		
	send_data_nrf(msg, 0);
}

/* Receiving data from nrf */
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
	
	/* Check the data of the received message here */
  if(datagram_buffer.dev == 0x01 && datagram_buffer.command == 0x01)
	{
		LED_on();
		sendAck(0x01,0x11, 0x00, 0x00, 0x00,status_contents);
  }
	else if(datagram_buffer.dev == 0x01 && datagram_buffer.command == 0x02)
	{
		LED_off();
		sendAck(0x01,0x12, 0x00, 0x00, 0x00,status_contents);
  }
	else if(datagram_buffer.dev == 0x03){
			 if(datagram_buffer.command == 0x01){
					 //turn on rgb led 
					  change_color(red,green,blue);
					 	sendAck(0x03,0x11, 0x00, 0x00, 0x00,status_contents);
			 }
			 else if(datagram_buffer.command == 0x02){
					 //turn off rgb led 
					 change_color(0,0,0);
					 	sendAck(0x03,0x12, 0x00, 0x00, 0x00,status_contents);
			 }
			 else if(datagram_buffer.command == 0x03){
					 //set brightness for rgb led
					 change_brightness(datagram_buffer.data_byte0);
					 	sendAck(0x03,0x13, datagram_buffer.data_byte0, 0x00, 0x00,status_contents); //change data[2]
			 }
			 else if(datagram_buffer.command == 0x04){
					 //set color for rgb led
					  change_color(datagram_buffer.data_byte0,datagram_buffer.data_byte1,datagram_buffer.status);
					 	sendAck(0x03,0x14,datagram_buffer.data_byte0,datagram_buffer.data_byte1,datagram_buffer.status,status_contents); //change data[2]
			 }
	}
	else if(datagram_buffer.dev == 0x04){
			 if(datagram_buffer.command == 0x01){
					 //turn on rgb led 
					 GPIO_WriteHigh( GPIOB, GPIO_PIN_4);
					 	sendAck(0x04,0x11, 0x00, 0x00, 0x00,status_contents);
			 }
			 else if(datagram_buffer.command == 0x02){
					 //turn off heating systems
					 GPIO_WriteLow( GPIOB, GPIO_PIN_4);
					 	sendAck(0x04,0x12, 0x00, 0x00, 0x00,status_contents);
			 }
			 else if(datagram_buffer.command == 0x03){
					 //set temperature for heating system
					  setTempThreshold(datagram_buffer.data_byte0);
					 	sendAck(0x04,0x13, datagram_buffer.data_byte0, 0x00, 0x00,status_contents); //change data[2]
			 }
	}
	return;
}


void main()
{
		gpioSetup();
		TIM1_DeInit();
		motionSetup();
		RGB_init();
		init_dim_LED();
		TempSens_Init();
		
		//reset leds
		LED_on();
		change_color(0,0,0);
		dim_LED(30);
		change_color(10,0,10);
		
		
		/* NRF24 library setup */
		nrf24_setup_bus();
		setup_interupt();
		config_nrf();
		
		GPIOC->CR2 = 0x00;
		GPIOC->ODR = 0xFF;
	
		GPIOC->CR2 = ((1 << 4)| GPIOC->CR2);
		GPIOC->DDR &= ~(1<<4);
		GPIOC->CR1 &= ~(1<<4);
	
		/* enable interrupts */
		enableInterrupts();
		
    while(1){
				 /*volatile datagram updateMsg;
				 updateMsg.dev = 0x05;
				 updateMsg.command = 0x02;
				 updateMsg.data_byte0 = 0x00;
				 updateMsg.data_byte1 = 0x00;
				 updateMsg.status = 0x00;
				 send_data_nrf(updateMsg, 0);*/
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
