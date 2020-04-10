#ifndef _LCD12864_
#define _LCD12864_

typedef unsigned char uchar;
typedef unsigned long int uint32;
typedef unsigned short int uint16;

//LCD��ض˿ڶ���
sbit LCD_RS=P2^0;
sbit LCD_RW=P2^1;
sbit LCD_E=P2^2;
sbit PSB=P2^3;          //PSB��Ϊ12864-12ϵ�еĴ�����ͨѶ�����л�

#define LCD_Data P0     //LCD���ݿ�
#define Busy 0x80

//��������

void LCDInit(void);     //LCD��ʼ��
void WriteCommandLCD(uchar WCLCD, uchar BuysC);    //дָ��
uchar ReadStatusLCD(void);          //��״̬�����æ
void WriteDataLCD(uchar WDLCD);     //д����
uchar ReadDataLCD(void);            //������
void LCDClear(void);                //����
void DisplayListChar(uchar X, uchar Y, uchar *DData);   //��ʾһ���ַ�
void Delayms(uint16 time);  //��ʱ

#endif