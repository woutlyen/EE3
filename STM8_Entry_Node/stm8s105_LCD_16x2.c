/**
  * Code by: Aswinth Raj
  * Website: https://circuitdigest.com
  * GitHub: https://github.com/CircuitDigest/STM8S103F3_SPL/
  */
  

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s105_LCD_16x2.h"

/* Defines ------------------------------------------------------------------*/
#define LCD_RS     GPIOB, GPIO_PIN_5
#define LCD_EN     GPIOB, GPIO_PIN_4
#define LCD_DB4    GPIOB, GPIO_PIN_3
#define LCD_DB5    GPIOB, GPIO_PIN_2
#define LCD_DB6    GPIOB, GPIO_PIN_1
#define LCD_DB7    GPIOB, GPIO_PIN_0

/* Functions ---------------------------------------------------------*/

void delay_ms (int ms) 
{
    int i =0 ;
    int j=0;
    for (i=0; i<=ms; i++)
    {
    for (j=0; j<180; j++) 
    _asm("nop"); 
    }
}

void Lcd_SetBit(char data_bit) 
{
    if(data_bit& 1) 
        GPIO_WriteHigh(LCD_DB4); //D4 = 1
    else
        GPIO_WriteLow(LCD_DB4); //D4=0

    if(data_bit& 2)
        GPIO_WriteHigh(LCD_DB5); //D5 = 1
    else
        GPIO_WriteLow(LCD_DB5); //D5=0

    if(data_bit& 4)
        GPIO_WriteHigh(LCD_DB6); //D6 = 1
    else
        GPIO_WriteLow(LCD_DB6); //D6=0

    if(data_bit& 8) 
        GPIO_WriteHigh(LCD_DB7); //D7 = 1
    else
        GPIO_WriteLow(LCD_DB7); //D7=0
}

void Lcd_Cmd(char a)
{
    GPIO_WriteLow(LCD_RS); //RS = 0, enable command mode      
    Lcd_SetBit(a); 
    GPIO_WriteHigh(LCD_EN); //EN  = 1, start sending command
		delay_ms(2);
		GPIO_WriteLow(LCD_EN); //EN  = 0 , end sending command  
}

 void Lcd_Begin()
 {
 //Initialize all GPIO pins as Output
    GPIO_DeInit(GPIOB);
	GPIO_Init(LCD_RS , GPIO_MODE_OUT_PP_HIGH_FAST);  
    GPIO_Init(LCD_EN, GPIO_MODE_OUT_PP_HIGH_FAST);  
    GPIO_Init(LCD_DB4, GPIO_MODE_OUT_PP_HIGH_FAST);  
    GPIO_Init(LCD_DB5, GPIO_MODE_OUT_PP_HIGH_FAST);  
    GPIO_Init(LCD_DB6 , GPIO_MODE_OUT_PP_HIGH_FAST);  
    GPIO_Init(LCD_DB7, GPIO_MODE_OUT_PP_HIGH_FAST);  
	 
	 
	 
	Lcd_SetBit(0x00); //initialize inputs
	delay_ms(1000); //waiting after power is applied


//initialization of Lcd Driver Chip 
  Lcd_Cmd(0x03);
  delay_ms(5);
  Lcd_Cmd(0x03);
  delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); // enable 4-bit mode
	
	
	//initialization of  display 
  Lcd_Cmd(0x02); //return cursor to its original position
  Lcd_Cmd(0x08); //display off, cursor off, blinking off 
  Lcd_Cmd(0x00); //nop
  Lcd_Cmd(0x0C); //display on, cursor off 
  Lcd_Cmd(0x00); //nop
  Lcd_Cmd(0x06); //  cursor shift to right, disable the shift of entire display
	
 }

void Lcd_Clear()
{
		Lcd_Cmd(0x00);//nop
		Lcd_Cmd(0x01);//Clear the LCD
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Print_Char(char data)   // 4 bit mode 
{
   char lowerNibble,upperNibble;
	 
   lowerNibble = data&0x0F;
   upperNibble = data&0xF0;
	 
   GPIO_WriteHigh(LCD_RS);             // => RS = 1, input mode
	 
   Lcd_SetBit(upperNibble>>4);             
   GPIO_WriteHigh(LCD_EN); 
   delay_ms(5); 
   GPIO_WriteLow(LCD_EN);
	 
   Lcd_SetBit(lowerNibble); 
   GPIO_WriteHigh(LCD_EN); 
   delay_ms(5); 
   GPIO_WriteLow(LCD_EN); 
}

void Lcd_Print_String(char *str)
{
    while (*str) {
        Lcd_Print_Char(*str);
        str++;
    }
}
