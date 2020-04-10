//LCD��ʼ��
void LCDInit(void)
{
    WriteCommandLCD(0x30,1);        //��ʾģʽ���ã���ʼҪ��ÿ�μ��æ�ź�
    WriteCommandLCD(0x01,1);        //��ʾ����
    WriteCommandLCD(0x06,1);        //��ʾ����ƶ�����
    WriteCommandLCD(0x0C,1);        //��ʾ�����������

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

//�����Ǿ�����װ��Ĳ������������ַ���ʾ

//����
void LCDClear(void)
{
    WriteCommandLCD(0x01,1);        //��ʾ����
    WriteCommandLCD(0x34,1);        //��ʾ����ƶ�����
    WriteCommandLCD(0x30,1);        //��ʾ�����������
    return;
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
            Delayms(5);
        }
    }
    return;
}