#ifndef __24L01_H
#define __24L01_H
#include"reg52.h"
#include"system.h"






 

#define TX_ADR_WIDTH   5  // 5字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH 32  // 数据通道有效数据宽度













sbit CE   = P2^0;  //芯片的模式控制线，在CSN为低时，CE与CONFIG寄存器共同决定状态 Chip Enable pin signal (output)
sbit CSN  = P3^5;  //芯片的片选线，CSN为低电平芯片工作 Slave Select pin, (output to CSN, nRF24L01)  
sbit SCK  = P2^1;  //芯片控制的时钟线（SPI时钟） Master Out, Slave In pin (output)	
sbit MOSI = P3^4;  //芯片控制数据线，串行输入 Serial Clock pin, (output)  
sbit MISO = P2^2;  //芯片控制数据线，串行输出 Master In, Slave Out pin (input)
sbit IRQ  = P3^3;  // 采用中断写时必须接到P32或P33上Interrupt signal, from nRF24L01 (input)

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

void init_24L01(void);
uchar SPI_RW(uchar byte);
uchar SPI_RW_Reg(uchar reg, uchar value);
uchar SPI_Read(uchar reg);
uchar SPI_Read_Buf(uchar reg, uchar * pBuf, uchar bytes);
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes);
void RX_Mode(void);
void TX_Mode(uchar * BUF);
uchar Check_ACK(bit clear);	 

#endif

