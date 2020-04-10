#include <reg51.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define OP_READ  0xa1          // 器件地址以及读取操作
#define OP_WRITE 0xa0          // 器件地址以及写入操作
uchar code discode[]={ 0x28, 0x7E, 0xA2, 0x62, 0x74, 0x61, 0x21, 0x7A, 0x20, 0x60,0xff};//0,1,2,3,4,5,6,7,8,9,关显示，数码管码表                            
uchar data display[]={0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0};
sbit SDA = P3^7;    
sbit SCL = P3^6;
unsigned char shift ;
unsigned char count=0 ;
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
/**********************************************************
 延时子程序
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
 开始位
**********************************************************/
void start()
{
   SDA = 1;   // 初始化数据线
   SCL = 1;   // 初始化时钟线
   delayNOP();
   SDA = 0;   // 在SCL为1时由1置0 
   delayNOP();
   SCL = 0;   // 释放时钟线
}
/**********************************************************
  停止位
**********************************************************/
void stop()
{
   SDA = 0;  // 初始化数据线
   delayNOP();
   SCL = 1;  // 初始化时钟线 
   delayNOP();
   SDA = 1;  // 在SCL为1时由0置1
}
/**********************************************************
 从AT24C02移出数据到MCU
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
 从MCU移出数据到AT24C02
**********************************************************/
bit shout(uchar write_data) 
{
 uchar i;
 bit ack_bit;
 for(i = 0; i < 8; i++)  // 循环移入8个位
 {
  SDA = (bit)(write_data & 0x80);
  _nop_();
  SCL = 1;
  delayNOP();
  SCL = 0;
  write_data <<= 1;
 }
 SDA = 1;   // 读取应答
 delayNOP();
 SCL = 1;
 delayNOP();
 ack_bit = SDA;
 SCL = 0;
 return ack_bit; // 返回AT24C02应答位
}
/**********************************************************
 在指定地址addr处写入数据write_data
**********************************************************/
void write_byte(uchar addr, uchar write_data)
{
  start();
  shout(OP_WRITE);
  shout(addr);
  shout(write_data);
  stop();
  delayms(10);  // 写入周期
}
/**********************************************************
 在当前地址读取
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
 在指定地址读取
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
   for(k=0; k<8; k++)     //写入显示代码到AT24C02
   write_byte(k, discode[k]);   
   delayms(100);
   for(k=0; k<8; k++)      
   display[k]=read_random(k);  //从AT24C02读出显示代码
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

