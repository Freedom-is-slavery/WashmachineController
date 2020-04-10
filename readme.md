# 自动洗衣机控制系统设计

>本项目是浙江大学电气工程学院爱迪生实验班微机原理与应用期末项目设计，建个Repository留档保存于此

**注：主程序见Repository中wash_v2.0.c文件**

本项目以MCS51为基础，设计了一个自动化的洗衣机控制系统，涵盖了日常洗衣机使用的所有功能，包括定时洗衣、定时脱水、水位设定、启动、暂停的基本功能，以及制动、重启、漏电漏水状态的上传、断电继续洗衣等进阶功能。整个系统中，矩阵键盘读取用户操作；液晶屏作为人机交互接口，显示洗衣机的基本信息与状态；AD转换读取水位信息；EEPROM配合I2C总线存储洗衣机状态信息；步进电机作为被控对象，可实现高低速、正反转多种状态；串口实现与上位PC机的通信。

![ ](RunningDiagram.png)

## 该项目实现功能

1. 洗衣机工作方式：有洗衣和脱水两种模式，两种方式能够分别通过按键设定一个小于9秒的时间；
2. 水位设置：能够通过按键设定两档水位（高和低）；
3. 启停操作：提供给用户启动、暂停、紧急制动的按键操作，按下启动键后启动洗衣，按下暂停键后暂停洗衣，按下制动键后紧急停机，只能通过重启键从头开始洗衣过程；
4. 液晶显示：实现人机交互，显示开始界面、状态信息、结束界面、紧急停机界面等，其中状态信息的界面能够展示当前状态和下一状态（进水、待启动、洗衣、脱水、暂停等）、水位选择信息、当前的实时水位信息（进水时可显示0~4共五个等级，洗衣时可显示是否正常），洗衣和脱水能够实时显示剩余时间，以秒为单位；
5. 水位传感器及其读取判断：外部输入电压作为模拟水位信号，通过AD转换芯片读回当前电压采样值，转换成相应的水位等级，并能够判断是否符合水位设定要求（本项目中低水位下限设置为2V，高水位下限设置为4V）；
6. 机械系统控制：系统能够控制步进电机运行，在洗衣模式下，电机以低速正反转交替运行；在脱水模式下，电机以高速正转运行，暂停和紧急制动状态下，电机立刻停止运转；
7. 紧急状态的通信：三种紧急状态：预留的漏电状态按键被按下，视为洗衣机漏电；当水位降低到设定值以下后，视为漏水；用户按下制动键。以上三种均采取紧急停机操作，并将相应信息通过串口上传至上位PC；
8. 断电后的继续运行：在洗衣机正常运转时，将当前状态（水位设定、洗衣时间、脱水时间、当前剩余时间）通过I2C总线上传至EEPROM芯片保存，若突然断电后再上电运转，系统能够通过I2C总线读回存储的状态继续运行；
9. 重复运行：当洗衣机完整运行一轮后，可以通过重新开始的按键启动新一轮的运行；
