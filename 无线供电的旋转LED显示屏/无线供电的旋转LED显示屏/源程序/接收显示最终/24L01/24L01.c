#include"24l01.h"
//#include"1602.h"

sbit KEY1=P1^0;
sbit KEY2=P1^1;
uchar code table12[]="0123456789abcdef";



//使用刘卫杰测试程序
uchar code TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01};//定义一个静态发送地址
uchar TxBuf[5]={0};	 // 
uchar RxBuf[TX_PLOAD_WIDTH]={0};
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
void init_24L01(void)
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
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH); 
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
	while(IRQ);

	sta = SPI_RW(NOP);                    // 返回状态寄存器
	MAX_RT=sta>>4&0x01;
	RX_DR=sta>>6&0x01;
	TX_DS=sta>>5&0x01;
	if(MAX_RT)//重发次数溢出中断，写1清除中断
		if(clear)// 是否清除TX FIFO，没有清除在复位MAX_RT中断标志后重发
			SPI_RW(FLUSH_TX);//TX FIFO寄存器满标志1是满，0不满
	SPI_RW_Reg(WRITE_REG + STATUS, sta);  
	// 清除TX_DS或MAX_RT中断标志，接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清楚中断标志
	IRQ = 1;
	
	if(TX_DS)//数据发送完成中断，写1清除中断
		return(0x00);
	else
		return(0xff);
}