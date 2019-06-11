#include"1602.h"
void yjwrite_com(uchar com)
{
	lcdrs=0;
	P0=com;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}
void yjwrite_date(uchar date)
{
	lcdrs=1;
	P0=date;
	delayms(5);
	lcden=1;
	delayms(5);
	lcden=0;
}
void yjinit()
{
	lcden=0;
	yjwrite_com(0x38);		
	yjwrite_com(0x0c);
	yjwrite_com(0x06);
	yjwrite_com(0x01);		//显示清0，指针清0
}