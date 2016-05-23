//#include "stm32l1xx_hal.h"
//#include "usart.h"
//#include "gpio.h"
#include "uartapp.h"
#include "stdio.h"
#include "init.h"

#ifndef DEVEICE
	#include "devproanaly.h"
#else
	#include "deveice.h"
#endif

/******************************************
Fun: 硬件初始化函数
Input: NULL
Output:NULL
Return:NULL
******************************************/
void HardInit(void)
{
	HTSensorInit();
}

/******************************************
Fun: 参数初始化函数
Input: NULL
Output:NULL
Return:NULL
******************************************/
void ParaInit(void)
{
//	DevParaInit();
}
