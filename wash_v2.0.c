#include <REG52.H>
#include <INTRINS.H>

typedef unsigned char uchar;
typedef unsigned short int uint16;

//LCD��ض˿ڶ���
sbit LCD_RS=P2^0;
sbit LCD_RW=P2^1;
sbit LCD_E=P2^2;
sbit PSB=P2^3;          //PSB��Ϊ12864-12ϵ�еĴ�����ͨѶ�����л�


#define LCD_Data P0     //LCD���ݿ�
#define Busy 0x80
//I2C���ߺ�EEPROM������ض���
#define OP_READ  0xa1          // ������ַ�Լ���ȡ����
#define OP_WRITE 0xa0          // ������ַ�Լ�д�����
#define current_save_addr 0x00
#define washmode_save_addr 0x01
#define drytime_save_addr 0x02
#define waterlevel_save_addr 0x03
sbit SDA = P2^7;
sbit SCL = P2^6;
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
//ADת����ض���
sbit ad_wr = P3^6;
sbit ad_rd = P3^7;
#define ad_input P1
//uchar code low_level = 0X64;   //��ˮλ��ѹֵ���趨Ϊ2V
//uchar code high_level = 0XC8;  //��ˮλ��ѹֵ���趨Ϊ4V

//���������ض���
uchar code FFW[4] = {0X20, 0X10, 0X08, 0X04};       //��ת
uchar code REV[4] = {0X20, 0X04, 0X08, 0X10};       //��ת
uchar round_count1;     //Ϊ��������ĵ����ڼ���
bit round_count2;     //Ϊ����ת���ڽ��м���
uchar round_count3;     //Ϊ�����
//uchar direction_flag = 0;   //��ת��ת��ָʾ��־λ��0������ת��1����ת

//����ɨ�����ȫ�ֱ���
uchar n_disp;       //�������ȷ�ϴ���������ȥ����
uchar keycode;


//״̬��ʶ��
uchar state = 1;

//ϴ�¡���ˮʱ�䡢ˮλ�趨���
uchar set_num;
uchar washtime, drytime, current_time;
uchar waterlevel_set;
//uchar waterlevel = 0;
uchar waterlevel_current[1];
uchar waterlevel_last[1];
bit waterjudge_flag;      //��״̬��������ʱ��Ҫ�ı�
//bit read_back_flag = 0;
bit mode_change_flag;
bit wash_mode;            //0��ʾϴ�£�1��ʾ��ˮ
uchar display_char[2];

//�����ϴ���ض���
#define electricity_leakage 0X44;
#define water_leakage 0X66;
#define restart 0X55;

void delayms(uint16 ms);    //�Ǿ�ȷ��ʱ

void start(void);       //I2C��ʼλ
void stop(void);        //I2Cֹͣλ
uchar shin(void);       //��AT24C02�Ƴ����ݵ�MCU
bit shout(uchar write_data);
void write_byte(uchar addr, uchar write_data);      //��ָ����ַaddr��д������write_data
uchar read_current(void);
uchar read_random(uchar random_addr);       //��ָ����ַrandom_addr��ȡ

uchar ReadStatusLCD(void);      //��״̬
void WriteCommandLCD(uchar WCLCD, uchar BuysC);     //дָ��
void WriteDataLCD(uchar WDLCD);     //д����
uchar ReadDataLCD(void);            //������
void LCDInit(void);                 //LCD��ʼ��
void LCDClear(void);                //LCD����
void DisplayListChar(uchar X, uchar Y, uchar *DData);   //��ʾһ���ַ�
uchar waterlevel_read(void);        //ADת��ˮλ��ȡ
uchar waterlevel_convert(uchar value);      //ˮλ�ȼ�ת��
void key_scan(void);                //����ɨ��
void situation_display(uchar *now_words, uchar *next_words);    //�̶���״̬��ʾ
void motor_set(void);               //�����ת�жϳ�ʼ��
void motor_one_round(uchar *period);        //�����ת��תһ������

//��ʱ
void delayms(uint16 ms)
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
void start(void)
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
void stop(void)
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
  delayms(20);  // д������
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
 ��ָ����ַ��ȡ
**********************************************************/
uchar read_random(uchar random_addr)
{
   start();
   shout(OP_WRITE);
   shout(random_addr);
   return(read_current());
}

//������LCD�Ļ�������
//��״̬
uchar ReadStatusLCD(void)
{
    LCD_Data = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_E = 1;
    while(LCD_Data & Busy)  //���æ�ź�
    ;
    return LCD_Data;
}

//дָ��
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

//д����
void WriteDataLCD(uchar WDLCD)
{
    ReadStatusLCD();        //���æ
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_Data = WDLCD;
    LCD_E = 1;
    LCD_E = 1;
    LCD_E = 1;
    LCD_E = 0;
    return;
}
//������
uchar ReadDataLCD(void)
{
    LCD_RS = 1;
    LCD_RW = 1;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
    return (LCD_Data);
}

//�����Ǿ�����װ��Ĳ�������ʼ�����������ַ���ʾ

//LCD��ʼ��
void LCDInit(void)
{
    WriteCommandLCD(0x30,1);        //��ʾģʽ���ã���ʼҪ��ÿ�μ��æ�ź�
    WriteCommandLCD(0x01,1);        //��ʾ����
    WriteCommandLCD(0x06,1);        //��ʾ����ƶ�����
    WriteCommandLCD(0x0C,1);        //��ʾ�����������
}

//����
void LCDClear(void)
{
    WriteCommandLCD(0x01,1);        //��ʾ����
    WriteCommandLCD(0x34,1);        //��ʾ����ƶ�����
    WriteCommandLCD(0x30,1);        //��ʾ�����������
}

//��ʾһ���ַ�
void DisplayListChar(uchar X, uchar Y, uchar *DData)
{
    uchar ListLength, X2;
    ListLength = 0;
    X2 = X;
    if(Y < 1) Y = 1;
    if(Y > 4) Y = 4;
    X &= 0x0F;      //����X���ܴ���16��Y��1-4֮��
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
    WriteCommandLCD(X2, 1);      //���͵�ַ��
    while(DData[ListLength] >= 0x20)      //�������ִ�β���˳�
    {
        if(X <= 0x0F)   //X����ӦС��0x0F
        {
            WriteDataLCD(DData[ListLength]);
            ListLength++;
            X++;
            delayms(10);
        }
    }
}

//ADת��,����ADת��ֵ
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

//ˮλ�ȼ�ת��
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


//����ɨ�裬���ذ���ֵ���ް���������Ϊ0XFF
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

//�̶���״̬��ʾ
void situation_display(uchar *now_words, uchar *next_words)
{
    LCDClear();
    DisplayListChar(0, 1, "��ǰ��");
    DisplayListChar(0, 2, "֮��");
    DisplayListChar(0, 3, "�趨ˮλ��");
    DisplayListChar(0, 4, "��ǰˮλ��");
    DisplayListChar(3, 1, now_words);
    DisplayListChar(3, 2, next_words);
    switch (waterlevel_set)
    {
        case 1: DisplayListChar(5, 3, "��"); break;
        case 2: DisplayListChar(5, 3, "��"); break;
        default: break;
    }
}

//�����ת�жϳ�ʼ��
void motor_set(void)    
{
    TMOD |= 0X01;
    TH0 = 0XB1;
    TL0 = 0XE0;     //��ʱ��Ϊ20ms
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

//�жϷ���������ڵ����ת������������ˮλ
void timer0_int(void) interrupt 1
{
    TH0 = 0XB1;
    TL0 = 0XE0;
    if (waterlevel_convert(waterlevel_read()) < waterlevel_set * 2)
    {
        SBUF = 0X66;
        state = 9;      //�ƶ�
    }
    if (!wash_mode)  //washmodeΪ0����ʾϴ��״̬
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
    else            //washmodeΪ1����ʾ��ˮ״̬
    {
        motor_one_round(FFW);
    }

    //�����
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

//��ʱ��1���ڲ����ʷ�����������Ϊ2400
void baud_set(void)
{
    TMOD |= 0X20;
    TH1 = 0XF3;
    TL1 = 0XF3;
    SCON = 0X50;
    PCON &= 0XEF;
    TR1 = 1;
}

//��ʼ��
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

//�������򣬽���һϵ�г�ʼ��������state=1
void machine_start(void)
{
    LCDInit();
    LCDClear();
    DisplayListChar(1, 2, "�Ǹ���ϴ�»�");
    DisplayListChar(2, 4, "��ӭʹ��");
    delayms(2000);

    baud_set();

    state = 2;
}

//����ǰ״̬�Ķ��أ�state=2

//�趨���棬����ˮλ��washtime��drytime��state=3
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

//����֮ǰϴ�µ�ˮλ��washtime��drytime��current_time��state=4
void read_back(void)
{
    current_time = read_random(current_save_addr);
    wash_mode = read_random(washmode_save_addr);
    drytime = read_random(drytime_save_addr);
    waterlevel_set = read_random(waterlevel_save_addr);
    //read_back_flag = 1;
    state = 5;
}

//��ˮ���̺�ˮλ�жϣ�ˮλ���ǰפ����ˮλ����ȷ������ʼ����
//����ADת����state=5
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
        DisplayListChar(3, 1, "������");
        if (keycode == 0XBD)
           state = 6;
    }
    else DisplayListChar(3, 1, "��ˮ��");
}

//������У���ʾ�����ı䣬I2C����ǰ״̬���棬©���©ˮ״̬ͨ�������ϴ�PC
//ϴ�»�����״̬������ϴ�º���ˮ��state=6
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
            situation_display("��ˮʣ  ��", "��ˮʣ  ��");
            DisplayListChar(6, 4, "OK");
        }
        mode_change_flag = 1;
    }
}
//��������state=7
void machine_over(void)
{
    
    key_scan();
    if (keycode == 0XEE)
    {
        state = 1;
    }
}

//��ͣ״̬�����ͣת��state=8
void machine_pause(void)
{
    key_scan();
    if (keycode != 0XBD)
    {
        TR0 = 0;     //�ر��жϣ���ֹ�����ת
    }
    else
    {
        TR0 = 1;
        state = 6;
        if (!wash_mode)
            situation_display("ϴ��ʣ  ��", "ϴ��ʣ  ��");
        else
            situation_display("��ˮʣ  ��", "��ˮʣ  ��");
    }
}

//�����ƶ�״̬�����ͣת��ֻ�ܰ�������������state=9
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
        switch (state)  //״̬��ʶ
        {
        case 1:     //��������
            machine_start();
            if (state == 2)
            {
                LCDClear();
                DisplayListChar(2, 2, "����ϴ��");
                DisplayListChar(0, 4, "ȷ��        ȡ��");
            }
            break;
        case 2:     //����ǰ״̬���ص�ȷ��
            key_scan();
            if (keycode == 0XBE)
            {
                state = 3;
                LCDClear();
                DisplayListChar(0, 1, "�����룺");
                DisplayListChar(0, 2, "ϴ��ʱ����  s");
                DisplayListChar(0, 3, "��ˮʱ����  s");
                DisplayListChar(0, 4, "ˮλ��");
            }
            else if (keycode == 0X7E)
                state = 4;
            break;
        case 3:     //ˮλ��washtime��drytime�趨
            settings();
            if (state == 5)
                situation_display("��ˮ", "ϴ��");
            break;
        case 4:     //������ǰ״̬
            read_back();
            situation_display("��ȡ", "ϴ��");
            delayms(1000);
            break;
        case 5:     //��ˮ���̺�ˮλ�ж�
            water_judge();
            if (state == 6)
            {
                motor_set();
                situation_display("ϴ��ʣ  ��", "ϴ��ʣ  ��");
                DisplayListChar(6, 4, "OK");         
            }
            break;
        case 6:     //ϴ�º���ˮ
            run();
            if (state == 8)
            {
                situation_display("��ͣ", "����");
				DisplayListChar(6, 4,"OK");
            }
            else if (state == 9)
            {
                LCDClear();
                DisplayListChar(2, 2, "����ͣ��");
                DisplayListChar(1, 3, "�谴������");
            }
            else if (state == 7)
            {
                delayms(2000);
                LCDClear();
				DisplayListChar(2, 1, "����̫��");
    			DisplayListChar(1, 3, "��л����ʹ��");
            }
            break;
        case 7:     //��������
            machine_over();
            if (state == 1)
            {
                machine_init();
            }
            break;
        //��������ͣ���ƶ�״̬
        case 8:     //��ͣ
            machine_pause();
            break;
        case 9:     //�ƶ�
            machine_stop();
            if (state == 1)
            {
                machine_init();
            }
            break;
        }
    }
}

