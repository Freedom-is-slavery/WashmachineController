#include <REG52.H>
#include <INTRINS.H>

typedef unsigned char uchar;
typedef unsigned short int uint16;

//LCD相关端口定义
sbit LCD_RS=P2^0;
sbit LCD_RW=P2^1;
sbit LCD_E=P2^2;
sbit PSB=P2^3;          //PSB脚为12864-12系列的串、并通讯功能切换


#define LCD_Data P0     //LCD数据口
#define Busy 0x80
//I2C总线和EEPROM操作相关定义
#define OP_READ  0xa1          // 器件地址以及读取操作
#define OP_WRITE 0xa0          // 器件地址以及写入操作
#define current_save_addr 0x00
#define washmode_save_addr 0x01
#define drytime_save_addr 0x02
#define waterlevel_save_addr 0x03
sbit SDA = P2^7;
sbit SCL = P2^6;
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
//AD转换相关定义
sbit ad_wr = P3^6;
sbit ad_rd = P3^7;
#define ad_input P1
//uchar code low_level = 0X64;   //低水位电压值，设定为2V
//uchar code high_level = 0XC8;  //高水位电压值，设定为4V

//步进电机相关定义
uchar code FFW[4] = {0X20, 0X10, 0X08, 0X04};       //正转
uchar code REV[4] = {0X20, 0X04, 0X08, 0X10};       //反转
uchar round_count1;     //为步进电机拍的周期计数
bit round_count2;     //为正反转周期进行计数
uchar round_count3;     //为秒计数
//uchar direction_flag = 0;   //正转反转的指示标志位，0代表正转，1代表反转

//按键扫描相关全局变量
uchar n_disp;       //按键检测确认次数，用于去抖动
uchar keycode;


//状态标识符
uchar state = 1;

//洗衣、脱水时间、水位设定相关
uchar set_num;
uchar washtime, drytime, current_time;
uchar waterlevel_set;
//uchar waterlevel = 0;
uchar waterlevel_current[1];
uchar waterlevel_last[1];
bit waterjudge_flag;      //在状态重新启动时需要改变
//bit read_back_flag = 0;
bit mode_change_flag;
bit wash_mode;            //0表示洗衣，1表示脱水
uchar display_char[2];

//串口上传相关定义
#define electricity_leakage 0X44;
#define water_leakage 0X66;
#define restart 0X55;

void delayms(uint16 ms);    //非精确延时

void start(void);       //I2C开始位
void stop(void);        //I2C停止位
uchar shin(void);       //从AT24C02移出数据到MCU
bit shout(uchar write_data);
void write_byte(uchar addr, uchar write_data);      //在指定地址addr处写入数据write_data
uchar read_current(void);
uchar read_random(uchar random_addr);       //在指定地址random_addr读取

uchar ReadStatusLCD(void);      //读状态
void WriteCommandLCD(uchar WCLCD, uchar BuysC);     //写指令
void WriteDataLCD(uchar WDLCD);     //写数据
uchar ReadDataLCD(void);            //读数据
void LCDInit(void);                 //LCD初始化
void LCDClear(void);                //LCD清屏
void DisplayListChar(uchar X, uchar Y, uchar *DData);   //显示一串字符
uchar waterlevel_read(void);        //AD转换水位读取
uchar waterlevel_convert(uchar value);      //水位等级转换
void key_scan(void);                //按键扫描
void situation_display(uchar *now_words, uchar *next_words);    //固定的状态显示
void motor_set(void);               //电机运转中断初始化
void motor_one_round(uchar *period);        //电机正转或反转一个周期

//延时
void delayms(uint16 ms)
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
void start(void)
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
void stop(void)
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
uchar shin(void)
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
  delayms(20);  // 写入周期
}


uchar read_current(void)
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

//以下是LCD的基本操作
//读状态
uchar ReadStatusLCD(void)
{
    LCD_Data = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_E = 1;
    while(LCD_Data & Busy)  //检测忙信号
    ;
    return LCD_Data;
}

//写指令
void WriteCommandLCD(uchar WCLCD, uchar BuysC)
{
    if(BuysC)
    ReadStatusLCD();
      LCD_RS = 0;
      LCD_RW = 0;
      LCD_Data = WCLCD;
      LCD_E = 1;
      LCD_E = 1;
      LCD_E = 1;
      LCD_E = 0;
}

//写数据
void WriteDataLCD(uchar WDLCD)
{
    ReadStatusLCD();        //检测忙
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_Data = WDLCD;
    LCD_E = 1;
    LCD_E = 1;
    LCD_E = 1;
    LCD_E = 0;
    return;
}
//读数据
uchar ReadDataLCD(void)
{
    LCD_RS = 1;
    LCD_RW = 1;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
    return (LCD_Data);
}

//以下是经过封装后的操作：初始化、清屏、字符显示

//LCD初始化
void LCDInit(void)
{
    WriteCommandLCD(0x30,1);        //显示模式设置，开始要求每次检测忙信号
    WriteCommandLCD(0x01,1);        //显示清屏
    WriteCommandLCD(0x06,1);        //显示光标移动设置
    WriteCommandLCD(0x0C,1);        //显示开及光标设置
}

//清屏
void LCDClear(void)
{
    WriteCommandLCD(0x01,1);        //显示清屏
    WriteCommandLCD(0x34,1);        //显示光标移动设置
    WriteCommandLCD(0x30,1);        //显示开及光标设置
}

//显示一串字符
void DisplayListChar(uchar X, uchar Y, uchar *DData)
{
    uchar ListLength, X2;
    ListLength = 0;
    X2 = X;
    if(Y < 1) Y = 1;
    if(Y > 4) Y = 4;
    X &= 0x0F;      //限制X不能大于16，Y在1-4之内
    switch(Y)
    {
        case 1:
            X2 |=0x80;
            break;
        case 2:
            X2 |=0x90;
            break;
        case 3:
            X2 |=0x88;
            break;
        case 4:
            X2 |=0x98;
            break;
    }
    WriteCommandLCD(X2, 1);      //发送地址码
    while(DData[ListLength] >= 0x20)      //若到达字串尾则退出
    {
        if(X <= 0x0F)   //X坐标应小于0x0F
        {
            WriteDataLCD(DData[ListLength]);
            ListLength++;
            X++;
            delayms(10);
        }
    }
}

//AD转换,返回AD转换值
uchar waterlevel_read()
{
    uchar tmp;
    ad_wr = 0;
    _nop_();
    ad_wr = 1;
    delayms(1);
    ad_input = 0XFF;
    _nop_();
    ad_rd = 0;
    _nop_();
    tmp = ad_input;
    ad_rd = 1;
    _nop_();
    return tmp;
}

//水位等级转换
uchar waterlevel_convert(uchar value)
{
    uchar level;
    if (value >= 0XC8)
        level = 4;
    else if (value >= 0X96)
        level = 3;
    else if (value >= 0X64)
        level = 2;
    else if (value >= 0X32)
        level = 1;
    else
        level = 0;
    return level;
}


//键盘扫描，返回按键值，无按键按下则为0XFF
void key_scan(void)
{
    uchar key1, key2, key;
    P1 = 0XF0;
    _nop_();
    key1 = P1;
    _nop_();
    P1 = 0X0F;
    _nop_();
    key2 = P1;
    key = key1 | key2;

    if (key != 0XFF)
    {
        n_disp ++;
        delayms(5);
        if (n_disp >= 10)
        {
            n_disp = 0;
            keycode = key;
            return;
        }
    }
    keycode = 0XFF;
    P1 = 0XFF;
}

//固定的状态显示
void situation_display(uchar *now_words, uchar *next_words)
{
    LCDClear();
    DisplayListChar(0, 1, "当前：");
    DisplayListChar(0, 2, "之后：");
    DisplayListChar(0, 3, "设定水位：");
    DisplayListChar(0, 4, "当前水位：");
    DisplayListChar(3, 1, now_words);
    DisplayListChar(3, 2, next_words);
    switch (waterlevel_set)
    {
        case 1: DisplayListChar(5, 3, "低"); break;
        case 2: DisplayListChar(5, 3, "高"); break;
        default: break;
    }
}

//电机运转中断初始化
void motor_set(void)    
{
    TMOD |= 0X01;
    TH0 = 0XB1;
    TL0 = 0XE0;     //定时设为20ms
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void motor_one_round(uchar *period)
{
    P3 &= 0XC3;
    P3 |= period[round_count1];
    round_count1 ++;
    if (round_count1 == 4)
        round_count1 = 0;
}

//中断服务程序，用于电机运转、计秒数、读水位
void timer0_int(void) interrupt 1
{
    TH0 = 0XB1;
    TL0 = 0XE0;
    if (waterlevel_convert(waterlevel_read()) < waterlevel_set * 2)
    {
        SBUF = 0X66;
        state = 9;      //制动
    }
    if (!wash_mode)  //washmode为0，表示洗衣状态
    {
        if (round_count2)
        {
            if (round_count3 >= 25)
            {
                motor_one_round(FFW);
            }
            else
            {
                motor_one_round(REV);
            }
        }
        round_count2 = ~round_count2;

    }
    else            //washmode为1，表示脱水状态
    {
        motor_one_round(FFW);
    }

    //秒计数
    if (round_count3 >= 50)
    {
        round_count3 = 0;
        display_char[0] = current_time + 48;
        DisplayListChar(6, 1, display_char);
        current_time --;
        write_byte(current_save_addr, current_time);
        write_byte(washmode_save_addr, wash_mode);
        display_char[0] --;
        DisplayListChar(6, 2, display_char);
        if (!wash_mode && !current_time)
        {
            wash_mode = 1;
        }
    }
    else
        round_count3 ++;

}

//定时器1用于波特率发生，波特率为2400
void baud_set(void)
{
    TMOD |= 0X20;
    TH1 = 0XF3;
    TL1 = 0XF3;
    SCON = 0X50;
    PCON &= 0XEF;
    TR1 = 1;
}

//初始化
void machine_init(void)
{
    LCDInit();
    EA = 0;
    set_num = 0;
    washtime = 0;
    drytime = 0;
    current_time = 0;
    waterlevel_set = 0;
    waterlevel_current[0] = 0;
    waterlevel_last[0] = 5;
    waterjudge_flag = 1;
    mode_change_flag = 0;
    wash_mode = 0;
    round_count1 = 0;
    round_count2 = 0;
    round_count3 = 0;
    keycode = 0XFF;
    n_disp = 0;
    display_char[1] = ' ';
}

//启动程序，进行一系列初始化操作，state=1
void machine_start(void)
{
    LCDInit();
    LCDClear();
    DisplayListChar(1, 2, "登哥牌洗衣机");
    DisplayListChar(2, 4, "欢迎使用");
    delayms(2000);

    baud_set();

    state = 2;
}

//掉电前状态的读回，state=2

//设定界面，设置水位、washtime、drytime，state=3
void settings(void)
{
    uchar key_number, send_char[1];
    key_scan();
    switch (keycode)
    {
        case 0X77: key_number = 1; set_num++; break;
        case 0XB7: key_number = 2; set_num++; break;
        case 0XD7: key_number = 3; set_num++; break;
        case 0XE7: key_number = 4; set_num++; break;
        case 0X7B: key_number = 5; set_num++; break;
        case 0XBB: key_number = 6; set_num++; break;
        case 0XDB: key_number = 7; set_num++; break;
        case 0XEB: key_number = 8; set_num++; break;
        case 0X7D: key_number = 9; set_num++; break;
        default: break;
    }
    send_char[0] = key_number + 48;
    switch (set_num)
    {
        case 1: 
            washtime = key_number; 
            DisplayListChar(5, 2, send_char); 
            break;
        case 2: 
            drytime = key_number; 
            DisplayListChar(5, 3, send_char); 
            break;
        case 3:
            if (key_number > 2)
                set_num = 2;
            else 
			{
                waterlevel_set = key_number; 
            	DisplayListChar(5, 4, send_char); 
            }
			break;
    }
    if (set_num >= 3 && keycode == 0X7E)
    {
        state = 5;
        set_num = 0;
        write_byte(drytime_save_addr, drytime);
        write_byte(waterlevel_save_addr, waterlevel_set);
		current_time = washtime;
    }
}

//读回之前洗衣的水位、washtime、drytime、current_time，state=4
void read_back(void)
{
    current_time = read_random(current_save_addr);
    wash_mode = read_random(washmode_save_addr);
    drytime = read_random(drytime_save_addr);
    waterlevel_set = read_random(waterlevel_save_addr);
    //read_back_flag = 1;
    state = 5;
}

//进水过程和水位判断，水位达标前驻留，水位达标后按确定键开始运行
//调用AD转换，state=5
void water_judge(void)
{
    uchar waterlevel_char[1];
    waterlevel_current[0] =  waterlevel_convert(waterlevel_read());
    key_scan();
    if (waterlevel_current[0] != waterlevel_last[0])
    {
        waterlevel_char[0] = waterlevel_current[0] + 48;
        DisplayListChar(5, 4, waterlevel_char);
        waterlevel_last[0] = waterlevel_current[0];
    }
    if ((waterlevel_current[0] >= 2 && waterlevel_set == 1) || \
    (waterlevel_current[0] >=4 && waterlevel_set == 2))
    {
        DisplayListChar(3, 1, "可启动");
        if (keycode == 0XBD)
           state = 6;
    }
    else DisplayListChar(3, 1, "进水中");
}

//电机运行，显示秒数改变，I2C将当前状态保存，漏电和漏水状态通过串口上传PC
//洗衣机工作状态，包含洗衣和脱水，state=6
void run(void)
{
    key_scan();
    switch (keycode)
    {
        case 0XDD:
            state = 8;
            break;
        case 0XED:
            SBUF = restart;
            state = 9;
            break;
        case 0XDE:
            SBUF = electricity_leakage;
            state = 9;
            break;
        default:
            break;
    }
    if (current_time == 0)
    {
        if (mode_change_flag)
        {
            state = 7;
            EA = 0;
        }
        else
        {
            current_time = drytime;
            situation_display("脱水剩  秒", "脱水剩  秒");
            DisplayListChar(6, 4, "OK");
        }
        mode_change_flag = 1;
    }
}
//结束程序，state=7
void machine_over(void)
{
    
    key_scan();
    if (keycode == 0XEE)
    {
        state = 1;
    }
}

//暂停状态，电机停转，state=8
void machine_pause(void)
{
    key_scan();
    if (keycode != 0XBD)
    {
        TR0 = 0;     //关闭中断，防止电机运转
    }
    else
    {
        TR0 = 1;
        state = 6;
        if (!wash_mode)
            situation_display("洗衣剩  秒", "洗衣剩  秒");
        else
            situation_display("脱水剩  秒", "脱水剩  秒");
    }
}

//紧急制动状态，电机停转，只能按重启键重启，state=9
void machine_stop(void)
{
    key_scan();
    EA = 0;
    if (keycode == 0XEE)
        state = 1;

}

void main()
{
    while(1)
    {
        switch (state)  //状态标识
        {
        case 1:     //启动程序
            machine_start();
            if (state == 2)
            {
                LCDClear();
                DisplayListChar(2, 2, "继续洗衣");
                DisplayListChar(0, 4, "确定        取消");
            }
            break;
        case 2:     //掉电前状态读回的确认
            key_scan();
            if (keycode == 0XBE)
            {
                state = 3;
                LCDClear();
                DisplayListChar(0, 1, "请输入：");
                DisplayListChar(0, 2, "洗衣时长：  s");
                DisplayListChar(0, 3, "脱水时长：  s");
                DisplayListChar(0, 4, "水位：");
            }
            else if (keycode == 0X7E)
                state = 4;
            break;
        case 3:     //水位、washtime、drytime设定
            settings();
            if (state == 5)
                situation_display("进水", "洗衣");
            break;
        case 4:     //读掉电前状态
            read_back();
            situation_display("读取", "洗衣");
            delayms(1000);
            break;
        case 5:     //进水过程和水位判定
            water_judge();
            if (state == 6)
            {
                motor_set();
                situation_display("洗衣剩  秒", "洗衣剩  秒");
                DisplayListChar(6, 4, "OK");         
            }
            break;
        case 6:     //洗衣和脱水
            run();
            if (state == 8)
            {
                situation_display("暂停", "运行");
				DisplayListChar(6, 4,"OK");
            }
            else if (state == 9)
            {
                LCDClear();
                DisplayListChar(2, 2, "紧急停机");
                DisplayListChar(1, 3, "需按键重启");
            }
            else if (state == 7)
            {
                delayms(2000);
                LCDClear();
				DisplayListChar(2, 1, "机你太美");
    			DisplayListChar(1, 3, "感谢您的使用");
            }
            break;
        case 7:     //结束程序
            machine_over();
            if (state == 1)
            {
                machine_init();
            }
            break;
        //以下是暂停和制动状态
        case 8:     //暂停
            machine_pause();
            break;
        case 9:     //制动
            machine_stop();
            if (state == 1)
            {
                machine_init();
            }
            break;
        }
    }
}

