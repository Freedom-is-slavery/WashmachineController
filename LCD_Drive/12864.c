//LCD初始化
void LCDInit(void)
{
    WriteCommandLCD(0x30,1);        //显示模式设置，开始要求每次检测忙信号
    WriteCommandLCD(0x01,1);        //显示清屏
    WriteCommandLCD(0x06,1);        //显示光标移动设置
    WriteCommandLCD(0x0C,1);        //显示开及光标设置

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

//以下是经过封装后的操作：清屏、字符显示

//清屏
void LCDClear(void)
{
    WriteCommandLCD(0x01,1);        //显示清屏
    WriteCommandLCD(0x34,1);        //显示光标移动设置
    WriteCommandLCD(0x30,1);        //显示开及光标设置
    return;
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
            Delayms(5);
        }
    }
    return;
}