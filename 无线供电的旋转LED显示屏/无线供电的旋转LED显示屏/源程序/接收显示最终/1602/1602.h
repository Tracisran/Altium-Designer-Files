#ifndef __1602_H
#define __1602_H
#include"system.h"
sbit lcden=P2^4;
sbit lcdrs=P2^5;
void yjwrite_com(uchar com);
void yjwrite_date(uchar date);
void yjinit(void);
#endif


