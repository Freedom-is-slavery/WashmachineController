//void LCDFlash(void);              //��Ļ����
//void DisplayOneChar(uchar X,uchar Y,uchar DData);   //��ʾһ���ַ�


//��Ļ����
//void LCDFlash(void)
//{
    //WriteCommandLCD(0x08,1);        //��ʾ����
    //Delayms(200);
    //WriteCommandLCD(0x0c,1);        //��ʾ�����������
    //Delayms(200);
    //WriteCommandLCD(0x08,1);        //��ʾ����
    //Delayms(200);
    //WriteCommandLCD(0x0c,1);        //��ʾ�����������
    //Delayms(200);
    //WriteCommandLCD(0x08,1);        //��ʾ����
    //Delayms(200);
//}
//��ʾһ���ַ�
//void DisplayOneChar(uchar X,uchar Y,uchar DData)
//{
    //if(Y<1)
        //Y = 1;

    //if(Y>4)
        //Y = 4;
    //X &= 0x0F;      //����X���ܴ���16��Y���ܴ���1
    //switch(Y)
    //{
        //case 1:
            //X |= 0x80;
            //break;
        //case 2:
            //X |=0x90;
            //break;
        //case 3:
            //X |=0x88;
            //break;
        //case 4:
            //X |=0x98;
            //break;
    //}

    //WriteCommandLCD(X,0);       //���ﲻ���æ�źţ����͵�ַ��
    //WriteDataLCD(DData);
//}


//��ʾһ���ַ�
void DisplayOneChar(uchar X,uchar Y,uchar DData)
{
    if(Y<1)
    {
        Y = 1;
    }
    if(Y>4)
    {
        Y = 4;
    }
    X &= 0x0F;      //����X���ܴ���16��Y���ܴ���1
    switch(Y)
    {
        case 1:
            X |= 0x80;
            break;
        case 2:
            X |=0x90;
            break;
        case 3:
            X |=0x88;
            break;
        case 4:
            X |=0x98;
            break;
    }

    WriteCommandLCD(X,0);       //���ﲻ���æ�źţ����͵�ַ��
    WriteDataLCD(DData);
}



//��ʾ����ǰ״̬�� ����һ״̬�� ��ˮλѡ�� ����ǰˮλ��
void situation_display()
{
    LCDInit();
    DisplayListChar(0, 1, "��ǰ��");
    DisplayListChar(0, 2, "֮��");
    DisplayListChar(0, 3, "�趨ˮλ��");
    DisplayListChar(0, 4, "��ǰˮλ��");


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