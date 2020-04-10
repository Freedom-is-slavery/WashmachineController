#ifndef _LCD12864_
#define _LCD12864_

typedef unsigned char uchar;
typedef unsigned long int uint32;
typedef unsigned short int uint16;

//LCD相关端口定义
sbit LCD_RS=P2^0;
sbit LCD_RW=P2^1;
sbit LCD_E=P2^2;
sbit PSB=P2^3;          //PSB脚为12864-12系列的串、并通讯功能切换

#define LCD_Data P0     //LCD数据口
#define Busy 0x80

//函数声明

void LCDInit(void);     //LCD初始化
void WriteCommandLCD(uchar WCLCD, uchar BuysC);    //写指令
uchar ReadStatusLCD(void);          //读状态即检测忙
void WriteDataLCD(uchar WDLCD);     //写数据
uchar ReadDataLCD(void);            //读数据
void LCDClear(void);                //清屏
void DisplayListChar(uchar X, uchar Y, uchar *DData);   //显示一串字符
void Delayms(uint16 time);  //延时

#endif
