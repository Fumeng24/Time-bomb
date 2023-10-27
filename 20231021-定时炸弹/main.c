#include <reg52.h>
sbit we = P2^7;
sbit du = P2^6;
sbit buzzer = P2^3;
bit isCountingDown = 0;	//	倒计时是否开始
unsigned char code leddata[]=	//	数码管段选表
{
	0x3F,	//	"0"0
	0x06,	//	"1"1
	0x5B,	//	"2"2
	0x4F,	//	"3"3
	0x66,	//	"4"4
	0x6D,   //	"5"5
	0x7D,   //	"6"6
	0x07,   //	"7"7
	0x7F,   //	"8"8
	0x6F,   //	"9"9
	0x77,   //	"A"10
	0x7C,   //	"B"11
	0x39,   //	"C"12
	0x5E,   //	"D"13
	0x79,   //	"E"14
	0x71,   //	"F"15
	0x76,   //	"H"16
	0x38,   //	"L"17
	0x37,   //	"n"18
	0x3E,   //	"u"19
	0x73,   //	"P"20
	0x5C,   //	"o"21
	0x40,	//	"-"22
	0x00,   //	熄灭23
	0x00 	//	自定义
};
unsigned char position[]=
{
	0xfe,	//	1111 1110
	0xfd,	//	1111 1101
	0xfb,	//	1111 1011
	0xf7,	//	1111 0111
	0xef,	//	1110 1111
	0xdf,	//	1101 1111
	0xbf,	//	1011 1111
	0x7f	//	0111 1111
};

unsigned int i=0;	//	数码管显示位选变量

unsigned int inputValue = 0;	//	用于存储输入的值

unsigned char numDigits = 0;	//	用于跟踪已输入的数字数量

unsigned char digitArray[4] = {0, 0, 0, 0};	//	用于存储每一位的值

unsigned int ticks = 0;	//	计时中断次数

void delay(unsigned int xms)	//	xms延迟函数
{
	unsigned char i, j;
	while(xms--)
	{
		i = 2;
		j = 199;
		do
		{
			while (--j);
		} while (--i);
	}
}

void init()	//	1ms定时器
{
	TMOD = 0x01;    //定时器16为计数工作模式
	TH0 = (65536-1000)/256; // 高8位赋初值
	TL0 = (65536-1000)%256; // 低8位赋初值
	ET0 = 1; //开定时器0中断
	TR0 = 1;//启动定时器0
	EA = 1;	//开总中断
}

void displayDigit(unsigned char digit, unsigned char pos)
{
	P0 = position[pos];	//	选择数码管位置
	we = 1;
	we = 0;
	P0 = leddata[digit];	//	传输要显示的数字
	du = 1;
	du = 0;
}

void updateDisplay() interrupt 1
{
	TH0 = (65536-1000)/256;	//	高8位赋初值
	TL0 = (65536-1000)%256;	//	低8位赋初值

	if(isCountingDown)
	{
		ticks++;
		if(ticks >= 1000)    // 1s
		{
			ticks = 0;

			if(digitArray[3] > 0)	//	个位大于0，个位-1
			digitArray[3]--;
			else 
			{
				digitArray[3] = 9;

				if(digitArray[2] > 0)	//	十位大于0，十位-1
				digitArray[2]--;
				else 
				{
					digitArray[2] = 9;

					if(digitArray[1] > 0)	//	百位大于0，百位-1
					digitArray[1]--;
					else 
					{
						digitArray[1] = 9;

						if(digitArray[0] > 0)	//	千位大于0，千位-1
						digitArray[0]--;
						else 
						{
							isCountingDown = 0;	//	倒计时终止
							for(i = 0; i < 4; i++)	//	显示屏清零
							{
								digitArray[i] = 0;
							}
							numDigits = 0;	//	数字长度清0，等待下一次定时
						}
					}
				}
			}
		}
	}

	if(i < 4)	//	继续数码管刷新显示
	{
		displayDigit(digitArray[i], i);
		i++;
	}
	else
	{
		i = 0;
	}
}


int KeyScan()	//	矩阵键盘扫描
{
	int KeyValue = -1;	//	初始化

	P3 = 0xf0;	//	P3设置输出
	delay(5);
	if(P3 != 0xf0)	//	检查是否有按键按下
	{
		delay(5);	//	软件消抖
		if(P3 != 0xf0)
		{
			switch(P3)
			{
				case 0xe0:KeyValue = 0;break;
				case 0xd0:KeyValue = 1;break;
				case 0xb0:KeyValue = 2;break;
				case 0x70:KeyValue = 3;break;
			}

			P3 = 0x0f;	//	检测哪个按键按下了
			delay(5);
			switch(P3)
			{
				case 0x0e:break;	// 不做修改
				case 0x0d:KeyValue += 4;break;
				case 0x0b:KeyValue += 8;break;
				case 0x07:KeyValue += 12;break;
			}

			while(P3 != 0x0f);	// 等待按键释放
		}
	}
	else
	{
		P3 = 0xff;	// 检测独立按键
		delay(5);
		if(P3 != 0xff)
		{
			delay(5);	// 软件消抖
			switch(P3)
			{
				case 0xfe:KeyValue = 16;break;
				case 0xfd:KeyValue = 17;break;
				case 0xfb:KeyValue = 18;break;
				case 0xf7:KeyValue = 19;break;
			}
			while(P3 != 0xff);	// 等待按键释放
		}
	}
	return KeyValue;
}


void main()
{
	int key;
	init();	//	启动定时器

	while(1)
	{
		key = KeyScan();

		if(key >= 0 && key <= 9 && numDigits < 4)	//当按下的按键为0123456789，且总输入长度小于4时
		{
			unsigned char i;
			for(i = 0; i < 3; i++)	//	将所有数值向左移一位
			{
				digitArray[i] = digitArray[i+1];
			}
			digitArray[3] = key;	//	设置个位为刚刚按下的key数值
			if(numDigits < 4)	//限定输入的总长度为4
			{
				numDigits++;
			}
		}
		else if(key == 10)	//	当按下的按键为S16时，清空已经输入的值，重新开始输入
		{
			unsigned char i;
			for(i = 0; i < 4; i++)
			{
				digitArray[i] = 0;
			}
			numDigits = 0;
		}
		else if(key == 16)	//	当按下的按键为独立按键第一个时，开始倒计时，再次按下停止倒计时
		{
			isCountingDown = !isCountingDown;    // 更改倒计时状态
		}
		if(key == 17)	//	当按下的按键为独立按键第二个时，提前引爆
		{
			digitArray[0] = 0;
			digitArray[1] = 0;
			digitArray[2] = 0;
			digitArray[3] = 3;
		}
		if(isCountingDown ==1 && digitArray[0] == 0 && digitArray[1] == 0 && digitArray[2] == 0)
		{
			if(digitArray[3] == 3 || digitArray[3] == 2 || digitArray[3] == 1)	//	作业要求，当他为3210时，蜂鸣器响20ms
			{
				buzzer=0;
				delay(20);
				buzzer=1;
				delay(980);
			}
			else if(digitArray[3] == 0)	//	作业要求，当他为0时，LED同时闪亮三次
			{
				P1=0x00;
				delay(500);
				P1=0xff;
				delay(500);
				P1=0x00;
				delay(500);
				P1=0xff;
				delay(500);
				P1=0x00;
				delay(500);
				P1=0xff;
				delay(500);
			}
		}
	}
}
