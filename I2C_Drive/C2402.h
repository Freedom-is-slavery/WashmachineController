#include <reg51.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define OP_READ  0xa1          // ������ַ�Լ���ȡ����
#define OP_WRITE 0xa0          // ������ַ�Լ�д�����
uchar code discode[]={ 0x28, 0x7E, 0xA2, 0x62, 0x74, 0x61, 0x21, 0x7A, 0x20, 0x60,0xff};//0,1,2,3,4,5,6,7,8,9,����ʾ����������                            
uchar data display[]={0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0};
sbit SDA = P3^7;    
sbit SCL = P3^6;
unsigned char shift ;
unsigned char count=0 ;
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
/**********************************************************
 ��ʱ�ӳ���
**********************************************************/
char code SST516[3] _at_ 0x003b;
void delayms(uint ms) 
{
 uchar k;
 while(ms--)
 {
  for(k = 0; k < 120; k++);
 }
}
/**********************************************************
 ��ʼλ
**********************************************************/
void start()
{
   SDA = 1;   // ��ʼ��������
   SCL = 1;   // ��ʼ��ʱ����
   delayNOP();
   SDA = 0;   // ��SCLΪ1ʱ��1��0 
   delayNOP();
   SCL = 0;   // �ͷ�ʱ����
}
/**********************************************************
  ֹͣλ
**********************************************************/
void stop()
{
   SDA = 0;  // ��ʼ��������
   delayNOP();
   SCL = 1;  // ��ʼ��ʱ���� 
   delayNOP();
   SDA = 1;  // ��SCLΪ1ʱ��0��1
}
/**********************************************************
 ��AT24C02�Ƴ����ݵ�MCU
**********************************************************/
uchar shin()
{
   uchar i,read_data;
   for(i = 0; i < 8; i++)
   {
     SCL = 1;
     read_data <<= 1;
     read_data |= SDA;
     SCL = 0;
   }
   return(read_data);
}
/**********************************************************
 ��MCU�Ƴ����ݵ�AT24C02
**********************************************************/
bit shout(uchar write_data) 
{
 uchar i;
 bit ack_bit;
 for(i = 0; i < 8; i++)  // ѭ������8��λ
 {
  SDA = (bit)(write_data & 0x80);
  _nop_();
  SCL = 1;
  delayNOP();
  SCL = 0;
  write_data <<= 1;
 }
 SDA = 1;   // ��ȡӦ��
 delayNOP();
 SCL = 1;
 delayNOP();
 ack_bit = SDA;
 SCL = 0;
 return ack_bit; // ����AT24C02Ӧ��λ
}
/**********************************************************
 ��ָ����ַaddr��д������write_data
**********************************************************/
void write_byte(uchar addr, uchar write_data)
{
  start();
  shout(OP_WRITE);
  shout(addr);
  shout(write_data);
  stop();
  delayms(10);  // д������
}
/**********************************************************
 �ڵ�ǰ��ַ��ȡ
**********************************************************/
uchar read_current()
{
  uchar read_data;
  start();
  shout(OP_READ);
  read_data = shin();
  stop();
  return read_data;
}
/**********************************************************
 ��ָ����ַ��ȡ
**********************************************************/
uchar read_random(uchar random_addr)
{
   start();
   shout(OP_WRITE);
   shout(random_addr);
   return(read_current());
}
/**********************************************************
**********************************************************/
main(void)
{
   uchar k;
   SDA = 1;
   SCL = 1;  
   for(k=0; k<8; k++)     //д����ʾ���뵽AT24C02
   write_byte(k, discode[k]);   
   delayms(100);
   for(k=0; k<8; k++)      
   display[k]=read_random(k);  //��AT24C02������ʾ����
   while(1)
   {
     shift=0xfe;
     P2=0xff ;
     for(k=0; k<8; k++)
     {  
       P0=display[k];
       P2=shift;
       shift=_crol_(shift,1);
	   delayms(1);
     } 
   }
}
/**********************************************************/

