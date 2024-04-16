/**
  * Code by: Aswinth Raj
  * Website: https://circuitdigest.com
  * GitHub: https://github.com/CircuitDigest/STM8S103F3_SPL/
  */

/* Function prototypes -----------------------------------------------*/
void Lcd_SetBit(char data_bit);
void Lcd_Cmd(char a);
void Lcd_Begin(void);
void Lcd_Address(uint8_t row, uint8_t column);
void Lcd_Print_Char(char data);
void Lcd_Clear(void);
void Lcd_Set_Cursor(char a, char b);
void Lcd_Print_String(char *a);
