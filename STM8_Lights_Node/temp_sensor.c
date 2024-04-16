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

    //  /* Configure TIM3 */
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

}

/* Public functions ---------------------------------------------------------*/
void setTempThreshold(uint16_t newThreshold) {
    tempThreshold = newThreshold;
}

uint16_t getCurrentTemp() {
    return currentTemp;
}

/* Sensor init function ---------------------------------------------------------*/
void TempSens_Init() {

    /* Init all components */
    TIM3_Configuration();
   // ADC_Configuration();
    Temp_GPIO_Configuration();

	/* Start all functions */
    TIM3_Cmd(ENABLE);
}
