/**
  ******************************************************************************
  * @file    temp_sensor.c
  * @author  Lander Van Loock
  * @version V1.0.0
  * @date    10-April-2024
  * @brief   Temperature sensor library body
   ******************************************************************************
  * @attention
  * 
  * Enable interrupts in the main program body to get the libary to work
  * 
  * Call TempSens_Init() to initialize the library
  * 
  * Call setTempThreshold() to set a new temperature threshold
  * 
  * Call getCurrentTemp() to get the current temperature
  * 
  * This library uses TIM3 to generate interrupts each 5 seconds
  * 
  * This library linearizes a 2k2 Ohm temperature sensor
  * 
  * Change the defines to where the heating resistor and temperature sensor
  * are connected on the used PCB
  * 
  ******************************************************************************
   */

/* Includes ------------------------------------------------------------------*/
#include "stm8s_gpio.h"
#include "stm8s_adc1.h"
#include "stm8s_tim3.h"
#include "temp_sensor.h"

/* Private defines -----------------------------------------------------------*/
#define ADC_PORT GPIOB, GPIO_PIN_0
#define ADC_CHANNEL ADC1_CHANNEL_0

#define HEATING_PORT GPIOB, GPIO_PIN_4

/* Global variables ----------------------------------------------------------*/
uint16_t adcValue = 0;
uint16_t tempThreshold = 30;
uint16_t currentTemp = 20;
uint16_t decimalTemps[28] = { 315, 304, 294, 284, 274, 264, 255, 246, 238, 229, 221, 213, 206, 199, 191, 185, 178, 172, 165, 159, 154, 148, 143, 138, 133, 128, 124, 120 };

/* Private functions ---------------------------------------------------------*/
void TIM3_Configuration() {

    /* DeInit TIM3 */
    TIM3_DeInit();

     /* Configure TIM3 */
    TIM3_TimeBaseInit(TIM3_PRESCALER_32768, 2440);

    // Enable TIM3 update interrupt
	TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
}

void ADC_Configuration() {
    
    ADC1_DeInit();
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,        // Single conversion mode
              ADC_CHANNEL,                       // ADC Channel (see defines)
              ADC1_PRESSEL_FCPU_D18,             // ADC prescaler
              ADC1_EXTTRIG_TIM,                  // External trigger (optional)
              DISABLE,                           // Disable the buffer (optional)
              ADC1_ALIGN_RIGHT,                  // Right alignment
              ADC1_SCHMITTTRIG_CHANNEL0,         // Schmitt trigger disable on ADC channel (optional)
              DISABLE);                          // Disable fast mode (optional)

    ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);
}

void Temp_GPIO_Configuration() {

    /* Initialize ADC input pin */
    GPIO_Init(ADC_PORT, GPIO_MODE_IN_FL_NO_IT);

    /* Initialize heating resistor pin */
    GPIO_Init(HEATING_PORT, GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(LED_PORT, GPIO_MODE_OUT_PP_LOW_SLOW);
}

/* Public functions ---------------------------------------------------------*/
void setTempThreshold(uint16_t newThreshold) {
    tempThreshold = newThreshold;
}

uint16_t getCurrentTemp() {
    return currentTemp;
}

/* Interrupt routines -------------------------------------------------------*/
/* Interrupt routine of TIM3 to start ADC1 every 5s */
INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, 15)
 {
    ADC1_StartConversion();
	TIM3_ClearFlag(TIM3_FLAG_UPDATE);
 }

/* Interrupt routine for EOC of ADC1 */
INTERRUPT_HANDLER(ADC1_IRQHandler, 22)
{		
    int i = 0;   

    /* Read the ADC value */
	adcValue = ADC1_GetConversionValue();

    /* Convert ADC value to Temp */
    for (i = 0; i < (sizeof(decimalTemps)/sizeof(decimalTemps[0])); i++) {
      if((i == (sizeof(decimalTemps)/2)-1) || (adcValue >= (decimalTemps[i]-(decimalTemps[i]-decimalTemps[i+1])/2))) {
        currentTemp = i + 10;
        break;
      }
    }
    
    /* Determine heating action based on the Temp */
	if(currentTemp >= tempThreshold) {
		GPIO_WriteLow(HEATING_PORT);
    } else if(currentTemp < (tempThreshold - 1)) {
		GPIO_WriteHigh(HEATING_PORT);
    }

    ADC1_ClearITPendingBit(ADC1_IT_EOC);
}

/* Sensor init function ---------------------------------------------------------*/
void TempSens_Init() {

    /* Init all components */
    TIM3_Configuration();
    ADC_Configuration();
    Temp_GPIO_Configuration();

	/* Start all functions */
    TIM3_Cmd(ENABLE);
}