#include<STC15F2K.h>
#include <intrins.H>
#include"ziku.h"
#define uint unsigned int
#define uchar unsigned char
uchar code table[]="0123456789abcdef";
uchar ok;
uchar abc=0;			//需要发送汉子的个数
uchar x;				//收到返回信号标志
uchar flag_end=0;		//一次串口接收结束
uint receive_length=6;	//串口一次接收数据长度定义，默认为6个字 
uchar menu=1; 
uchar xdata ckou[256];	//最多可装8个汉字
sbit lcden=P5^4;
sbit lcdrw=P5^5;
sbit lcdrs=P4^5;


void delayms(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)
		for(j=110;j>0;j--);
}




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
	lcdrw=0;
	yjwrite_com(0x38);		
	yjwrite_com(0x0c);
	yjwrite_com(0x06);
	yjwrite_com(0x01);		//显示清0，指针清0
}








#define TX_ADR_WIDTH   5  // 5字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH 32  // 数据通道有效数据宽度
#define RX_PLOAD_WIDTH 32
sbit CE   = P2^2;  //芯片的模式控制线，在CSN为低时，CE与CONFIG寄存器共同决定状态 Chip Enable pin signal (output)
sbit CSN  = P2^3;  //芯片的片选线，CSN为低电平芯片工作 Slave Select pin, (output to CSN, nRF24L01)  
sbit SCK  = P2^1;  //芯片控制的时钟线（SPI时钟） Master Out, Slave In pin (output)	
sbit MOSI = P2^4;  //芯片控制数据线，串行输入 Serial Clock pin, (output)  
sbit MISO = P2^0;  //芯片控制数据线，串行输出 Master In, Slave Out pin (input)
sbit IRQ  = P3^2;  // 采用中断写时必须接到P32或P33上Interrupt signal, from nRF24L01 (input)

// SPI(nRF24L01) commands		24L01操作命令
#define READ_REG    0x00  //读寄存器指令 Define read command to register
#define WRITE_REG   0x20  //写寄存器指令 Define write command to register
#define RD_RX_PLOAD 0x61  //读取接收数据指令 Define RX payload register address
#define WR_TX_PLOAD 0xA0  //写待发数据指令 Define TX payload register address
#define FLUSH_TX    0xE1  //冲洗发送TX FIFO指令 Define flush TX register command
#define FLUSH_RX    0xE2  //冲洗接收 FIFO指令 Define flush RX register command
#define REUSE_TX_PL 0xE3  //定义重复装载数据指令 Define reuse TX payload register command
						  //重新使用上一包发射的有效数据，当CE=1时，数据不断的重新发射
#define NOP         0xFF  //空操作，可以用来读状态寄存器 Define No Operation, might be used to read status register

// SPI(nRF24L01) registers(addresses)	  24L01寄存器地址
#define CONFIG      0x00  //配置收发状态，CRC校验模式以及收发状态响应方式 'Config' register address
#define EN_AA       0x01  //自动应答功能设置 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  //可用信道设置 'Enabled RX addresses' register address
#define SETUP_AW    0x03  //收发地址宽度设置 'Setup address width' register address
#define SETUP_RETR  0x04  //自动重发功能设置 'Setup Auto. Retrans' register address
#define RF_CH       0x05  //工作频率设置 'RF channel' register address
#define RF_SETUP    0x06  // 发射速率、功耗功能设置'RF setup' register address
#define STATUS      0x07  //状态寄存器 'Status' register address
#define OBSERVE_TX  0x08  //发送监测功能 'Observe TX' register address
#define CD          0x09  //地址检测 'Carrier Detect' register address
#define RX_ADDR_P0  0x0A  //频道0接收数据地址，最大长度为五字节 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F  //频道5接收数据地址 'RX address pipe5' register address
#define TX_ADDR     0x10  //发送地址寄存器 'TX address' register address
#define RX_PW_P0    0x11  //接收频道0接收数据长度，有效数据长度1~32 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16  // 接收频道5接收数据长度 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17  //FIFO栈入栈出状态寄存器设置 'FIFO Status Register' register address

void init_io(void);
uchar SPI_RW(uchar byte);
uchar SPI_RW_Reg(uchar reg, uchar value);
uchar SPI_Read(uchar reg);
uchar SPI_Read_Buf(uchar reg, uchar * pBuf, uchar bytes);
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes);
void RX_Mode(void);
void TX_Mode(uchar * BUF);
uchar Check_ACK(bit clear);
void CheckButtons();	 

sbit KEY1=P3^4;
sbit KEY2=P3^5;
sbit KEY3=P3^6;
sbit KEY4=P3^7;
sbit key_lock=P3^3;
uchar code table1[]="0123456789abcdef";
uchar length=150;


uchar code TX_ADDRESS[5]={0x34,0x43,0x10,0x10,0x01};//定义一个静态发送地址
uchar TxBuf[TX_PLOAD_WIDTH]={0};	 // 
uchar RxBuf[RX_PLOAD_WIDTH]={0};
uchar sta;
uchar MAX_RT;//重发次数溢出中断，写1清除中断，如果MAX_RT产生中断，必须清楚后系统才能进行通
uchar RX_DR;//接收数据中断，当收到有效数据后置1，写1清除中断
uchar  TX_DS;//数据发送完成中断，写1清除中断 


/*MAX_RT=sta>>4&0x01;//重发次数溢出中断，写1清除中断，如果MAX_RT产生中断，必须清楚后系统才能进行通
RX_DR=sta>>6&0x01;//接收数据中断，当收到有效数据后置1，写1清除中断
TX_DS=sta>>5&0x01;//数据发送完成中断，写1清除中断 		   




/**************************************************
函数: init_io()

描述:
    初始化IO
/**************************************************/
void init_io(void)
{
	CE  = 0;        // 待机
	CSN = 1;        // SPI禁止不工作
	SCK = 0;        // SPI时钟置低
	IRQ = 1;        // 中断复位
}
/**************************************************/

/**************************************************/

/**************************************************
函数：SPI_RW()
此处为spi的核心，是spi协议的编程
描述：
    根据SPI协议，写一字节数据到nRF24L01，同时从nRF24L01
	读出一字节
/**************************************************/
uchar SPI_RW(uchar byte)
{
	uchar i;
   	for(i=0; i<8; i++)          // 循环8次
   	{
   		MOSI = (byte & 0x80);   // byte最高位输出到MOSI
   		byte <<= 1;             // 低一位移位到最高位
   		SCK = 1;                // 拉高SCK，nRF24L01从MOSI读入1位数据，同时从MISO输出1位数据
   		byte |= MISO;       	// 读MISO到byte最低位
   		SCK = 0;            	// SCK置低
   	}
    return(byte);           	// 返回读出的一字节
}
/**************************************************/

/**************************************************
函数：SPI_RW_Reg()

描述：
    写数据value到reg寄存器
/**************************************************/
uchar SPI_RW_Reg(uchar reg, uchar value)
{
	uchar status;
  	CSN = 0;                   // CSN置低，开始传输数据
  	status = SPI_RW(reg);      // 选择寄存器，同时返回状态字
  	SPI_RW(value);             // 然后写数据到该寄存器
  	CSN = 1;                   // CSN拉高，结束数据传输
  	return(status);            // 返回状态寄存器
}
/**************************************************/

/**************************************************
函数：SPI_Read()

描述：
    从reg寄存器读一字节
/**************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN = 0;                    // CSN置低，开始传输数据
  	SPI_RW(reg);                // 选择寄存器
  	reg_val = SPI_RW(0);        // 然后从该寄存器读数据READ_REG    0x00  //读寄存器指令
  	CSN = 1;                    // CSN拉高，结束数据传输
  	return(reg_val);            // 返回寄存器数据
}
/**************************************************/

/**************************************************
函数：SPI_Read_Buf()

描述：
    从reg寄存器读出bytes个字节，通常用来读取接收通道
	数据或接收/发送地址
功能: 用于读数据，reg：为寄存器地址，pBuf：为待读出数据地址，uchars：读出数据的个数
/**************************************************/
uchar SPI_Read_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN置低，开始传输数据
  	status = SPI_RW(reg);       // 选择寄存器，同时返回状态字
  	for(i=0; i<bytes; i++)
    	pBuf[i] = SPI_RW(0);    // 逐个字节从nRF24L01读出
  	CSN = 1;                    // CSN拉高，结束数据传输
  	return(status);             // 返回状态寄存器
}
/**************************************************/

/**************************************************
函数：SPI_Write_Buf()

描述：
    把pBuf缓存中的数据写入到nRF24L01，通常用来写入发
	射通道数据或接收/发送地址
功能: 用于写数据：为寄存器地址，pBuf：为待写入数据地址，uchars：写入数据的个数
/**************************************************/
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN置低，开始传输数据
  	status = SPI_RW(reg);       // 选择寄存器，同时返回状态字
  	for(i=0; i<bytes; i++)
    	SPI_RW(pBuf[i]);        // 逐个字节写入nRF24L01
  	CSN = 1;                    // CSN拉高，结束数据传输
  	return(status);             // 返回状态寄存器
}
/**************************************************/

/**************************************************
函数：RX_Mode()

描述：
    这个函数设置nRF24L01为接收模式，等待接收发送设备的数据包
/**************************************************/
void RX_Mode(void)
{
	CE = 0;
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); 
	 // 接收设备接收通道0使用和发送设备相同的发送地址
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               
	// 使能接收通道0自动应答ACK应答允许
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           
	// 使能接收通道0
  	SPI_RW_Reg(WRITE_REG + RF_CH, 40);                 
	// 选择射频通道0x40
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); 
	 // 接收通道0选择和发送通道相同有效数据宽度
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);            
	// 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);              
	// CRC使能，16位CRC校验，上电，接收模式
  	CE = 1;        // 拉高CE启动接收设备
}
/**************************************************/

/**************************************************
函数：TX_Mode()

描述：
    这个函数设置nRF24L01为发送模式，（CE=1持续至少10us），
	130us后启动发射，数据发送结束后，发送模块自动转入接收
	模式等待应答信号。
/**************************************************/
void TX_Mode(uchar * BUF)
{
	CE = 0;
  	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     
	// 写入发送地址
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  
	// 为了应答接收设备，接收通道0地址和发送地址相同
  	SPI_Write_Buf(WR_TX_PLOAD, BUF, TX_PLOAD_WIDTH);                  
	// 写数据包到TX FIFO
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);       // 使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);   // 使能接收通道0
  	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x0a);  // 自动重发延时等待250us+86us，自动重发10次
  	SPI_RW_Reg(WRITE_REG + RF_CH, 40);         // 选择射频通道0x40
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);    // 数据传输率1Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      // CRC使能，16位CRC校验，上电
	CE = 1;
}
/**************************************************/

/**************************************************
函数：Check_ACK()

描述：
    检查接收设备有无接收到数据包，设定没有收到应答信
	号是否重发
/**************************************************/
uchar Check_ACK(bit clear)
{	
	uchar i=0;
	while(IRQ&&i<50)i++;

	                    // 返回状态寄存器
	MAX_RT=sta>>4&0x01;
	RX_DR=sta>>6&0x01;
	TX_DS=sta>>5&0x01;
	if(MAX_RT)//重发次数溢出中断，写1清除中断
		if(clear)// 是否清除TX FIFO，没有清除在复位MAX_RT中断标志后重发
			SPI_RW(FLUSH_TX);//TX FIFO寄存器满标志1是满，0不满


	SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70); 
	// 清除TX_DS或MAX_RT中断标志，接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清楚中断标志
	IRQ = 1;
	
	if(TX_DS)//数据发送完成中断，写1清除中断
		return(0x00);
	else
		return(0xff);
}
uchar key_scan()
{
	uchar key=0;
	static uchar key_up=1;
	key_lock=0;
	if(KEY1==0&&key_up)
	{
		delayms(10);
		if(KEY1==0)
		{
			key_up=0;
			key=1;
		}
	}
	if(KEY2==0&&key_up)
	{
		delayms(10);
		if(KEY2==0)
		{
			key_up=0;
			key=2;
		}
	}
	if(KEY3==0&&key_up)
	{
		delayms(10);
		if(KEY3==0)
		{
			key_up=0;
			key=3;
		}
	}
	if(KEY4==0&&key_up)
	{
		delayms(10);
		if(KEY4==0)
		{
			key_up=0;
			key=4;
		}
	}
	if(KEY1&&KEY2&&KEY3&&KEY4)key_up=1;
	return key;
}
void CheckButtons()
{
	uchar k;
	static uchar add=0;		//24c64中存储位置标志
	static uchar up=20,down=0;
	k=key_scan();	
	if(k==4)
	{
		menu++;
		if(menu==12)menu=0;
	}
	switch(menu)
	{
		case 0:				  //清除
			yjwrite_com(0x80+0x40);
			yjwrite_date('C');yjwrite_date('l');yjwrite_date('e');yjwrite_date('a');
			yjwrite_date('r');yjwrite_date('.');yjwrite_date('.');yjwrite_date('.');
			yjwrite_date(' ');yjwrite_date(' ');
			if(k==1)
			{
				TxBuf[0]=0xAA;
				TxBuf[1]=0x55;
				TxBuf[2]=1;
				TxBuf[3]=0;
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			break;
		case 1:									  //暂停移动
			yjwrite_com(0x80+0x40);
			yjwrite_date('S');yjwrite_date('t');yjwrite_date('o');yjwrite_date('p');
			yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			if(k==2||k==3||k==1)
			{
				TxBuf[0]=0xAA;
				TxBuf[1]=0x55;
				TxBuf[2]=2;
				TxBuf[3]=0;
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);	
			}
			break;
		case 2:							   	//字体宽度设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('W');yjwrite_date('i');yjwrite_date('d');yjwrite_date('e');
			yjwrite_date(':');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;TxBuf[3]=0;
			if(k==2)
			{
				TxBuf[2]=3;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('+');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			if(k==3)
			{
				TxBuf[2]=4;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('-');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			break;						  
		case 3:								   //移动速度设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('S');yjwrite_date('p');yjwrite_date('e');yjwrite_date('e');
			yjwrite_date('d');yjwrite_date(':');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;TxBuf[3]=0;
			if(k==2)
			{
				TxBuf[2]=5;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('+');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			if(k==3)
			{
				TxBuf[2]=6;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('-');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			break;
		case 4:		//静止显示时显示内容的开始位置设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('S');yjwrite_date('p');yjwrite_date('o');yjwrite_date('t');
			yjwrite_date('a');yjwrite_date('d');yjwrite_date('d');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;TxBuf[3]=0;
			if(k==2)
			{
				TxBuf[2]=7;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('+');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			if(k==3)
			{
				TxBuf[2]=8;
				yjwrite_com(0x80+0x40+7);
				yjwrite_date('-');
				TX_Mode(TxBuf);
				delayms(200);
				Check_ACK(1);
			}
			break;
		case 5:			//接收到的数据在24C64中存储位置设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('C');yjwrite_date('6');yjwrite_date('4');yjwrite_date('a');
			yjwrite_date('d');yjwrite_date('d');yjwrite_date(':');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;
			if(k==2)
			{
				add++;
			}
			if(k==3)
			{
				add--;
				if(add==-1)add=0;
			}
			if(k==1)
			{
				TxBuf[2]=9;
				TxBuf[3]=add;
				TX_Mode(TxBuf);delayms(200);Check_ACK(1);	
			}
			yjwrite_com(0x80+0x40+7);
			yjwrite_date('0'+add/100);yjwrite_date('0'+add/10%10);yjwrite_date('0'+add%10);
			break;
		case 7:					//动态扫描范围上限确定
			yjwrite_com(0x80+0x40);
			yjwrite_date('U');yjwrite_date('p');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;
			if(k==2)
			{
				up++;
				if(up==255)up=255;	
			}
			if(k==3)
			{
				up--;
				if(up<=(down+6))up=down+7;
			}
			if(k==1)
			{
				TxBuf[2]=10;
				TxBuf[3]=up;
				TX_Mode(TxBuf);delayms(200);Check_ACK(1);	
			}
			yjwrite_com(0x80+0x40+7);
			yjwrite_date('0'+up/100);yjwrite_date('0'+up/10%10);yjwrite_date('0'+up%10);
			break;
		case 6:					//动态扫描范围下限确定
			yjwrite_com(0x80+0x40);
			yjwrite_date('D');yjwrite_date('o');yjwrite_date('w');yjwrite_date('n');
			yjwrite_date(':');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;delayms(2);TxBuf[1]=0x55;
			if(k==2)
			{
				down++;
				if(down>=(up-6))down=up-7;	
			}
			if(k==3)
			{
				down--;
				if(down==-1)down=0;
			}
			if(k==1)
			{
				TxBuf[2]=11;
				TxBuf[3]=down;
				TX_Mode(TxBuf);delayms(200);Check_ACK(1);	
			}
			yjwrite_com(0x80+0x40+7);
			yjwrite_date('0'+down/100);yjwrite_date('0'+down/10%10);yjwrite_date('0'+down%10);
			break;
		case 8:		//发送的开始位置设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('S');yjwrite_date('t');yjwrite_date('a');yjwrite_date('r');
			yjwrite_date('t');yjwrite_date(':');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			TxBuf[0]=0xAA;TxBuf[1]=0x55;
			if(k==2)
			{
				abc++;
			}
			if(k==3)
			{
				abc--;
				if(abc==-1)abc=255;
			}
			break;
		case 9:  		//发送汉子的长度设置
			yjwrite_com(0x80+0x40);
			yjwrite_date('L');yjwrite_date('e');yjwrite_date('n');yjwrite_date('g');
			yjwrite_date('t');yjwrite_date('h');yjwrite_date(':');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			if(k==2)
			{
				length++;
			}
			if(k==3)
			{
				length--;
				if(length==0)length=1;
			}
			yjwrite_com(0x80+0x40+7);
			yjwrite_date('0'+length/100);yjwrite_date('0'+length/10%10);yjwrite_date('0'+length%10);
			break;
		case 10:
			yjwrite_com(0x80+0x40);
			yjwrite_date('S');yjwrite_date('e');yjwrite_date('n');yjwrite_date('d');
			yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			yjwrite_date(' ');yjwrite_date(' ');
			if(k==1)
			{
				ok=!ok;
				x=1;
			}
			break;
		case 11:
			yjwrite_com(0x80+0x40);
			yjwrite_date('R');yjwrite_date('_');yjwrite_date('l');yjwrite_date('o');
			yjwrite_date('n');yjwrite_date('g');yjwrite_date(':');yjwrite_date('_');
			yjwrite_date('_');yjwrite_date(' ');
			if(k==2)
			{
				receive_length++;
				if(receive_length==9)receive_length=1;
			}
			if(k==3)
			{
				receive_length--;
				if(receive_length==0)receive_length=8;
			}
			yjwrite_com(0x80+0x40+7);
			yjwrite_date('0'+receive_length/100);
			yjwrite_date('0'+receive_length/10%10);
			yjwrite_date('0'+receive_length%10);
			break;
	}
}

/*----------------------------
    UART1初始化
-----------------------------*/
void InitUART(void)
{
    SCON = 0x50;                //8位可变波特率	
	AUXR1= AUXR1 & 0x3F; 
    AUXR = 0x40;                //定时器1为1T模式
    TMOD = 0x20;                //定时器1为模式2(8位自动重载)
    TL1 = 0xDC;   				//设置波特率重装值
    TH1 = 0xDC;
    TR1 = 1;                    //定时器1开始工作
    ES = 1;                     //使能串口中断
    EA = 1;

}





void main()
{
	init_io();		//24L01init
	sta = SPI_Read(STATUS);	  // 读状态寄存器
	SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70);
	yjinit();  //1602init
	InitUART();
	EA=1;
	EX0=1;	 	//外部中断0允许
	IT0=1;	   	//下降沿触发

	RX_Mode();
	while(1)
	{
		uchar i,j;
		CheckButtons();
		yjwrite_com(0x80+5);
		yjwrite_date(table[abc/100]);
		yjwrite_date(table[abc/10%10]);
		yjwrite_date(table[abc%10]);
		if(ok)
		{
			for(i=0;i<32;i++)
			{
				TxBuf[i]=ziku1[abc*32+i];	
			} 
			abc++; 
			TX_Mode(TxBuf);	delayms(1000);
		    Check_ACK(1);
			delayms(1000);
			if(abc==length)
			ok=0;
			x=0;
		}
		RX_Mode();
		yjwrite_com(0x80);
		yjwrite_date(table[menu/100]);  
		yjwrite_date(table[menu/10%10]);
		yjwrite_date(table[menu%10]);
		if(flag_end)
		{
			for(i=0;i<receive_length;i++)
			{
				for(j=0;j<32;j++)
				{
					TxBuf[j]=ckou[j+32*i];
				}
				TX_Mode(TxBuf);	delayms(1000);
			    Check_ACK(1);
				delayms(1000);
			}
			yjwrite_com(0x80+0x40+13);
			yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');yjwrite_date(' ');
			flag_end=0;	
			yjwrite_com(0x80+9);
			yjwrite_date('0'+TxBuf[0]/16);	
			yjwrite_date('0'+TxBuf[0]%16);
			yjwrite_date('0'+TxBuf[1]/16);
			yjwrite_date('0'+TxBuf[1]%16);
			yjwrite_date('0'+TxBuf[2]/16);
			yjwrite_date('0'+TxBuf[2]%16);
		}
	}	
}
void int0()interrupt 0
{
	EX0=0;
	sta = SPI_Read(STATUS);	  // 读状态寄存器
	SPI_RW_Reg(WRITE_REG + STATUS, sta|0x70);
	RX_DR=sta>>6&0x01;
	if(RX_DR)
	{ 
		SPI_Read_Buf(RD_RX_PLOAD, RxBuf, TX_PLOAD_WIDTH);
		SPI_RW_Reg(WRITE_REG + STATUS, sta);
		if(RxBuf[2]==0x55)x=1;
	}
	EX0=1;
}
/*----------------------------
UART 中断服务程序
-----------------------------*/
void Uart() interrupt 4 using 1
{
	static i=0;
    if (RI)
    {
        RI = 0;                 //清除RI位
        ckou[i] = SBUF;              //P0显示串口数据
		i++;
		if(i==receive_length*32)
		{
			yjwrite_com(0x80+0x40+13);
			yjwrite_date('0'+receive_length/100);	
			yjwrite_date('0'+receive_length/10%10);
			yjwrite_date('0'+receive_length%10);
			flag_end=1;
			i=0;
		}
    }
}