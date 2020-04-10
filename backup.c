//void LCDFlash(void);              //屏幕闪动
//void DisplayOneChar(uchar X,uchar Y,uchar DData);   //显示一个字符


//屏幕闪动
//void LCDFlash(void)
//{
    //WriteCommandLCD(0x08,1);        //显示清屏
    //Delayms(200);
    //WriteCommandLCD(0x0c,1);        //显示开及光标设置
    //Delayms(200);
    //WriteCommandLCD(0x08,1);        //显示清屏
    //Delayms(200);
    //WriteCommandLCD(0x0c,1);        //显示开及光标设置
    //Delayms(200);
    //WriteCommandLCD(0x08,1);        //显示清屏
    //Delayms(200);
//}
//显示一个字符
//void DisplayOneChar(uchar X,uchar Y,uchar DData)
//{
    //if(Y<1)
        //Y = 1;

    //if(Y>4)
        //Y = 4;
    //X &= 0x0F;      //限制X不能大于16，Y不能大于1
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

    //WriteCommandLCD(X,0);       //这里不检测忙信号，发送地址码
    //WriteDataLCD(DData);
//}


//显示一个字符
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
    X &= 0x0F;      //限制X不能大于16，Y不能大于1
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

    WriteCommandLCD(X,0);       //这里不检测忙信号，发送地址码
    WriteDataLCD(DData);
}



//显示“当前状态” “下一状态” “水位选择” “当前水位”
void situation_display()
{
    LCDInit();
    DisplayListChar(0, 1, "当前：");
    DisplayListChar(0, 2, "之后：");
    DisplayListChar(0, 3, "设定水位：");
    DisplayListChar(0, 4, "当前水位：");


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