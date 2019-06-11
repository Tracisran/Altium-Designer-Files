#include<reg52.h>
#include"24L01.h"
#include"ziku.h"
#include"24c64.h"
extern uchar TxBuf[5];	  
extern uchar RxBuf[TX_PLOAD_WIDTH];
extern uchar sta;
extern uchar MAX_RT;//重发次数溢出中断，写1清除中断，如果MAX_RT产生中断，必须清楚后系统才能进行通
extern uchar RX_DR;//接收数据中断，当收到有效数据后置1，写1清除中断
extern uchar  TX_DS;//数据发送完成中断，写1清除中断 
uchar a;
uchar xdata xiancun1[96];
uchar xdata xiancun2[96];
uint num2;
uchar b;		//接收中断标志（中断1）
uchar stop=1;	//表示移动或者暂停移动标志位
uchar wide=4;	//字体宽度标志
uchar speed=2;	//字体移动速度
uchar up=50;	//动态扫描上限
uchar down=0;	//动态扫描下限
void yanshi(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)
		for(j=3;j>0;j--);
}  
void main()
{
	uchar i,j=0,k=0,num=6;
	uchar cycle;	//所转的圈数标志
	uchar c;		//接收到的命令类型标志
	uchar d=0;		//不滚动显示时，开始显示汉字的位置
	init_24L01();		//24L01init
	AT24C64_init();

	EA=1;
	EX0=0;
	EX1=1;
	IT0=1;
	IT1=1;
	P0=0xff;				 
	P1=0xff;
/*	for(i=0;i<32;i++)
	{
		RxBuf[i]=0xaa;
	}
	for(i=0;i<255;i++)
	{
		page_write(i*32,RxBuf);
		delayms(5);
		P0=~P0;
	}  	
	P0=0xff;  */
	for(i=0;i<32;i++)
	{
		RxBuf[i]=dian[i];
	} 
	for(i=0;i<5;i++)
	{
		TxBuf[i]=0x55;
	}
//	RX_Mode(); 
	TX_Mode(TxBuf);		
	Check_ACK(1);	
	delayms(1);
//	sta = SPI_Read(STATUS);	  // 读状态寄存器
//	SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70);	   	//清空状态寄存器
	for(i=0;i<6;i++)
	{
		sequential_read(32*i,16,xiancun1+16*i);
		sequential_read(32*i+16,16,xiancun2+16*i);
	}	   
	while(1)
	{ 
		RX_Mode();
		if(b)
		{
			EX0=0;
			SPI_Read_Buf(RD_RX_PLOAD, RxBuf, TX_PLOAD_WIDTH);
			if(RxBuf[0]==0xaa&&RxBuf[1]==0x55)	//如果收到的是0xaa,0x55，表示收到的是命令，
			{
				EA=0;
				c=RxBuf[2];
				switch(c)
				{
					case 1:	   		//清除显示内容
						for(i=0;i<32;i++)
						RxBuf[i]=0;
						for(i=0;i<255;i++)
						{
							page_write(i*32,RxBuf);
							delayms(5);
							P0=~P0;
						}	
						break;
					case 2:stop=!stop;break;
					case 3:					 //字体宽度增加
						wide++;
						if(wide==10)wide=4;
						break;
					case 4:					  //字体宽度减小
						wide--;
						if(wide==0)wide=4;
						break;
					case 5:				  		//字体移动速度增加
						speed--;
						if(speed==-1)speed=2;
						break;				  //字体移动速度变慢
					case 6:
						speed++;
						if(speed==10)speed=2;
						break;	
					case 7:				   	//不滚动显示时显示开始位置确定
						d+=6;
						if(d>250)d=0;
						num=d+7;
						for(i=0;i<6;i++)
						{
							sequential_read(32*(i+d),16,xiancun1+16*i);
							sequential_read(32*(i+d)+16,16,xiancun2+16*i);
						}
						break;		
					case 8:					  //不滚动显示时显示开始位置确定
						d-=6;
						if(d<0)d=250;
						num=d+7;
						for(i=0;i<6;i++)
						{
							sequential_read(32*(i+d),16,xiancun1+16*i);
							sequential_read(32*(i+d)+16,16,xiancun2+16*i);
						}
						break;	
					case 9:				 //接收到的数据在24C64中存储的开始位置
						num2=RxBuf[3];
						break;
					case 10:			 //动态扫描范围上限
						up=RxBuf[3];
						break;
					case 11:			 //动态扫描范围下限
						down=RxBuf[3];
						break;						
				}
				EA=1;
			}									//如果收到的不是命令，则表示收到的是数据
			else 
			{								//则存入24c64
				page_write(num2,RxBuf);
				num2+=32;
				TxBuf[0]=num2/32;
				TxBuf[1]=num2/32;
				TxBuf[2]=0x55;		  	//发送0x55表示接收到了，并通知主机要求发下一帧数据
				SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70);
				TX_Mode(TxBuf);
				Check_ACK(1);
			}	
		}	
		EX0=1;
		a=1;	
		if(b)a=0;
		b=0;
		while(a);
		for(i=0;i<96;i++)
		{
			P0=~xiancun1[i];
			P1=~xiancun2[i];
			yanshi(wide);
		}
		P0=0xff;
		P1=0xff; 	
		cycle++;
		if(cycle>=speed&&stop)
		{
			cycle=0;
			for(i=0;i<95;i++)
			{
				xiancun1[i]=xiancun1[i+1];
				xiancun2[i]=xiancun2[i+1];
			}
			xiancun1[95]=read_add(32*num+k);				//更新第六个汉字的第16列
			xiancun2[95]=read_add(32*num+k+16);
			k++;
			if(k==16)
			{
				k=0;
				num++;
				if(num>=up)
				num=down;
			}
		}
				  
	}
}
void int0() interrupt 0
{
	a=0;
	EX0=0;
}
void int1() interrupt 2
{
	sta = SPI_Read(STATUS);	  // 读状态寄存器
	RX_DR=sta>>6&0x01;
	if(RX_DR)
	{
		EX0=0;
		a=0;				//防止接收到信号中断后，a=1；使程序停止
		b=1;				//表示接收到数据
	}
	SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70);		
}