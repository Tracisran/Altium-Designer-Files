#ifndef __24C64_H
#define __24C64_H
#include<reg52.h>
#include"system.h"
sbit scl=P2^5;	//AT24C02
sbit sda=P2^6;	//AT24C02
void delay(void);
void start1(void);
void stop1(void);
void respons(void);
void AT24C64_init(void);
void write_byte1(uchar date);
uchar read_byte1(void);
//void write_add(uchar address,uchar date);
void page_write(uint address,uchar * num);
uchar read_add(uint address);
void sequential_read(uint address,uchar length,uchar * numread);
#endif

