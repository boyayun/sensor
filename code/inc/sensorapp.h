#ifndef __SENSORAPP_H
#define __SENSORAPP_H

#include "common.h"

#define SensorLog(M, ...)   custom_log("数据采集", M, ##__VA_ARGS__)

/*----------------------------温湿度采集----------------------------*/
#define HTSENSORADDR    0x80		//温湿度传感器I2C地址

#define TACQHOSTCMD 	0xE3		//温度采集主机保持命令
#define HACQHOSTCMD 	0xE5		//湿度采集主机保持命令
#define TACQUHOSTCMD 	0xF3		//温度采集非主机保持命令
#define HACQUHOSTCMD 	0xF5		//湿度采集非主机保持命令
#define HTWRREDCMD		0xE6		//写用户寄存器
#define HTRDREDCMD      0xE7		//读用户寄存器
#define HTRESETCMD		0xFE		//复位

typedef struct
{
	u8 dir;			//温度方向:0为正,1为负
	u16 value;		//温度值，精度为0.01摄氏度
}tacqstu;			//温度值结构体

typedef struct
{
	u16 hvalue;			//湿度值，精度为0.01%
	tacqstu tvalue;		
}htacqstu;			//温湿度值结构体

typedef struct
{
	_Bool acqflag;		//确定什么进行采集，1有效
}htsensorstu;			//温湿度传感器控制结构体

extern htsensorstu htsensor;		//温湿度传感器采集结构体

/******************************************************
Fun:温湿度传感器采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline HTSensorAcqStart()
{
	htsensor.acqflag = 1;
}

extern _Bool HTSensorInit();
extern void HTSensorAcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------传感器采集----------------------------*/
typedef struct
{
	u8 datasw;				//温湿度值上报控制
	htacqstu htvalue;		
}HTdatastu;				//温湿度数据结构体

typedef struct
{
	u8 datasw;		//数据开关 1有效开始处理
	HTdatastu htdata;
}acqdatastu;		//采集数据结构体

extern acqdatastu acqdata;     //采集数据

/******************************************************
Fun:采集数据开始处理
Input:void
Output:void
Return:void
******************************************************/
void inline AcqDataStart()
{
	acqdata.datasw = 1;
}

extern void SensorDataHandle();
//////////////////////////////华丽的分割线///////////////////////////

#endif /*sensorapp.h*/
