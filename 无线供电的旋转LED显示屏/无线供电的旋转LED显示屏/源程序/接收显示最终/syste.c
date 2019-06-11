#include"system.h"
void delayms(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)
		for(j=110;j>0;j--);
}