#include <REGX52.H>
sbit we = P2 ^ 7;
sbit du = P2 ^ 6;
sbit feng = P2 ^ 3;

unsigned char position[]=
	{
	0xff,	//1111 1111
	0xfe,	//1111 1110
	0xfd,	//1111 1101
	0xfb,	//1111 1011
	0xf7,	//1111 0111
	0xef,	//1110 1111
	0xdf,	//1101 1111
	0xbf,	//1011 1111
	0x7f	//0111 1111
	};

unsigned char number[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x40};
unsigned char __2to16[]={0xff,0xfe};

void delay(unsigned int xms)
{
	unsigned char i, j;
	while(xms)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
		xms--;
	}
}

void beep()
{
feng =1;
delay(50);
feng =0;
delay(50);
}

void shumaguan(unsigned char digit) {
	unsigned char q = 0, e = 0;
	if (digit > 10) return;
	while(1)
	{
		P1 = 0x00;
		feng = 1;
		for(q = 0; q < 9; q++)
		{
			P0 = position[q];
			we = 1;
			we = 0;
			P0 = number[digit];
			du = 1;
			du = 0;
			delay(1);
		}

		e++;
		if(e >= 100)
		{
			du = 1;
			P0 = 0x00;
			du = 0;
			P1 = 0Xff;
			feng = 0;
			break;
		}
	}
}


void main()
{
	bit exitflag = 0;
	while(1)
	{
		unsigned int count = 10, i;
		while(exitflag == 0)
		{
			for (i = 9; i >= 0; i--)
			{
				we = 1;
				P0 = __2to16[1];
				we = 0;
				du = 1;
				P0 = number[i];
				du = 0;
				
				switch(count)
				{
					case 10:
					case 9:
					case 8:
					case 7:
					case 6:
					case 5:
						delay(1000);
						count--;
						break;
					
					case 4:
					case 3:
					case 2:
						beep();
						delay(900);
						count--;
						break;
					
					case 1:
						exitflag = 1;
					  count=1;
						beep();
						break;
					
					default:
						break;
				}
					if (count == 0) 
					{
						exitflag = 1;
						beep();
						break;
					}
				
				if(exitflag == 1)
				{
					break;
				}
			}
			if(exitflag == 1)
			{
				break;
			}
		}
		while (exitflag == 1)
		{
			delay(1000);
			shumaguan(10);
			delay(400);
			shumaguan(10);
			delay(400);
			shumaguan(10);
			delay(400);
			break;
		}
		if(exitflag == 1)
		{
			exitflag = 3;
			break;
		}
	}
}
