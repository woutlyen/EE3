/**
  ******************************************************************************
  * @file    temp_sensor.h
  * @author  Lander Van Loock
  * @version V1.0.0
  * @date    10-April-2024
  * @brief   Temperature sensor library header
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

/* Private functions */
void TIM3_Configuration(void);
void ADC_Configuration(void);
void Temp_GPIO_Configuration(void);

/* Public functions */
void setTempThreshold(uint16_t newThreshold);
uint16_t getCurrentTemp(void);

/* Initialize the library */
void TempSens_Init(void);