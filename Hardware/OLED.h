#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);//清屏
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);//字符
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);//字符串
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);//无符号数，字符少了就去大的位数
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);//正数
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);//十六进制
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);//二进制


#endif
