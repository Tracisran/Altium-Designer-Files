#include"24C64.H"
#include"system.h"
/******************************************************************************
AT24C02存储与读取部分
******************************************************************************/
void delay()
{;;}
void start1()
{
	sda=1;
	delay();
	scl=1;
	delay();
	sda=0;
	delay();
}
void stop1()
{
	sda=0;
	delay();
	scl=1;
	delay();
	sda=1;
	delay();
}
void respons()
{
	uchar i;
	scl=1;
	delay();
	while((sda==1)&(i<250))i++;
	scl=0;
	delay();
}
void main_respons()
{
	scl=0;
	delay();
	sda=0;
	delay();
	scl=1;
	delay();
	scl=0;
	delay();
		
}
void AT24C64_init()
{
	scl=0;
	delay();
	sda=0;
	delay();
	scl=1;
	delay();

}
void write_byte1(uchar date)
{
	uchar i,temp;
	temp=date;
	for(i=0;i<8;i++)
	{
		temp=temp<<1;
		scl=0;
		delay();
		sda=CY;
		delay();
		scl=1;
		delay();
	}
	scl=0;
	delay();
	sda=1;
	delay();
}
uchar read_byte1()
{
	uchar i,k;
	scl=0;
	delay();
	sda=1;
	delay();
	for(i=0;i<8;i++)
	{
		scl=1;
		delay();
		k=(k<<1)|sda;
		scl=0;
		delay();
	}
	return k;
}
/*void write_add(uchar address,uchar date)
{
	start1();
	write_byte1(0xa0);
	respons();
	write_byte1(address/256);
	respons();
	write_byte1(address%256);
	respons();
	write_byte1(date);
	respons();
	stop1();
}  */
void page_write(uint address,uchar * num)
{
	uchar i=0;
	start1();
	write_byte1(0xa0);
	respons();
	write_byte1(address/256);
	respons();
	write_byte1(address%256);
	respons();
	for(i=0;i<32;i++)
	{	
		write_byte1(num[i]);
		respons();
	}
	stop1();
}
uchar read_add(uint address)
{
	uchar date;
	start1();
	write_byte1(0xa0);
	respons();
	write_byte1(address/256);
	respons();
	write_byte1(address%256);
	respons();
	start1();
	write_byte1(0xa1);
	respons();
	date=read_byte1();
	stop1();
	return date;	
}  	 
void sequential_read(uint address,uchar length, uchar * numread)
{
	uchar i;
	start1();
	write_byte1(0xa0);
	respons();
	write_byte1(address/256);
	respons();
	write_byte1(address%256);
	respons();
	start1();
	write_byte1(0xa1);
	respons();
	for(i=0;i<(length-1);i++)
	{
		numread[i]=read_byte1();
		main_respons();
	}
	numread[i]=read_byte1();
	stop1();
} 	   