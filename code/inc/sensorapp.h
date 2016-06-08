#ifndef __SENSORAPP_H
#define __SENSORAPP_H

#include "common.h"

#define SensorLog(M, ...)   custom_log("数据采集", M, ##__VA_ARGS__)
typedef struct
{
	_Bool acqflag;		//确定什么进行采集，1有效
}sensoracqstu;			//传感器控制结构体

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

//typedef struct
//{
//	_Bool acqflag;		//确定什么进行采集，1有效
//}htsensorstu;			//温湿度传感器控制结构体

extern sensoracqstu htsensor;		//温湿度传感器采集结构体

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

/*----------------------------空气质量传感器采集----------------------------*/
#define ROVALUE 16000	//RO的值:清洁空气中的传感器电阻值
#define RLVALUE 10000	//RL的值:负载电阻的值
#define VREF   30		//参考电压 3V,精度0.1
#define VTEST  50		//测试电压 5V，精度0.1

//typedef struct
//{
//	_Bool acqflag;		//确定什么进行采集，1有效
//}airsensorstu;		//空气质量采集结构体

typedef struct
{
	u16 airacqvalue;	//采集的电压值
	u8 airquadata;		//空气质量数据 RS/RO，精度0.01
}airacqdatastu;		//空气质量数据结构体

extern sensoracqstu airsensor;
extern airacqdatastu airdata;
/******************************************************
Fun:空气质量采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline AirSensorAcqStart()
{
	airsensor.acqflag = 1;
}

extern _Bool ADCAirquaInit();
extern void AirSensorAcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------CO2传感器采集----------------------------*/
#define CO2LISTNUM 90	//CO2数据的个数
#define CO2VREF   3000  //参考电压 3000mV
#define CO2FACTOR 5	    //co2电压放大倍数

//typedef struct
//{
//	_Bool acqflag;		//确定什么进行采集，1有效
//}co2acqstu;			//CO2采集结构体

typedef struct
{
	u16 acqvalue;	//采集的电压值
	u16 acqdata;		//CO2数据 ppm
}co2datastu;		//CO2数据结构体

extern sensoracqstu co2sensor;
extern co2datastu co2data;

/******************************************************
Fun:CO2传感器采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline Co2SensorAcqStart()
{
	co2sensor.acqflag = 1;
}

extern void Co2SensorAcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------PM2.5采集----------------------------*/
#define PMFACTOR 	160		//pm2.5采集计算斜率a的值   精度0.01  rt = a*pm2.5+k
#define PMOFFSET	0		//pm2.5采集计算偏移量k的值 精度0.01  rt = a*pm2.5+k

#define VOLCONVERT	283		//体积单位转换，转换成ml  10的-6次方
#define NAFACTOR	6		//阿伏加德罗常数  10的23次方
#define MALMASS		270		//摩尔质量 270*10的8次方g/mol

#define PM25PERIOD  30000	//PM2.5采集周期 30000ms 

typedef struct
{
	u8 periodtimeup;	//30s周期采集时间到 1时间到 0时间未到
	u8 lowlevacqstart;	//PM2.5低电平采集开始
}pm25acqstu;			//PM2.5采集结构体

typedef struct
{
	u16 lowlevvalue;	//低电平值
	u16 countratio;		//粒子数/283mil
	u16 massratio;		//质量浓度/立方米
	u16 aqi;			//空气质量指数
}pm25datastu;			//PM2.5数据结构体

extern pm25acqstu	pm25acq;
extern pm25datastu	pm25data;

/******************************************************
Fun:PM2.5采集周期时间到
Input:void
Output:void
Return:void
******************************************************/
void inline PM25PeriodTimeUp()
{
	pm25acq.periodtimeup = 1;
}

extern _Bool PM25AcqInit();
extern _Bool IsPM25LowlevAcqStart();
extern void PM25AcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------烟雾传感器采集----------------------------*/
#define SMOKELISTNUM 36 //烟雾数据个数
#define SMOKEVREF  60	//参考电压  精度0.01V
#define HUMANTEMP 25	//人体体表温度
//typedef struct
//{
//	_Bool acqflag;		//确定什么进行采集，1有效
//}smokeacqstu;			//烟雾采集结构体

typedef struct
{
	u16 acqvalue;	//采集的电压值
	u8 acqdata;		//烟雾数据 %
}smokedatastu;	//烟雾数据结构体

extern sensoracqstu smokeacq;	//烟雾采集结构体
extern smokedatastu smokedata; //烟雾数据结构体
/******************************************************
Fun:烟雾传感器采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline SmokeSensorAcqStart()
{
	smokeacq.acqflag = 1;
}

extern void SmokeSensorAcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------热电堆传感器采集----------------------------*/
#define THERLISTNUM 21 //热电堆数据个数
#define THERVREF  30	//参考电压  精度1mV

typedef struct
{
	u16 acqvalue;	//采集的电压值
	u8 acqdata;		
}therdatastu;		//热电堆数据结构体

extern sensoracqstu theracq;	//热电堆采集结构体
extern therdatastu therdata; 	//热电堆数据结构体

/******************************************************
Fun:热电堆传感器采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline TherSensorAcqStart()
{
	theracq.acqflag = 1;
}

extern void TherSensorAcqHandle();
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------传感器采集----------------------------*/

typedef struct
{
	u8 upsw;				//温湿度值上报控制
	htacqstu htvalue;		
}HTupstu;				//温湿度上报数据结构体

typedef struct
{
	u8 upsw;				//空气质量上报控制
	u8 airquavalue;			//空气质量上报数据 RS/RO
}airquaupstu;

typedef struct
{
	u8 upsw;			//CO2上报控制
	u16 co2data;			//CO2上报数据 ppm
}co2upstu;

typedef struct
{
	u8 upsw;			//PM2.5上报控制
	u16 countratio;		//粒子数/283mil
	u16 massratio;		//质量浓度/立方米
	u16 aqi;			//空气质量指数	
}pm25upstu;			//pm2.5上报数据结构体

typedef struct
{
	u8 upsw;			//烟雾上报控制
	u8 smokedata;		//烟雾上报数据 %
}smokeupstu;			//烟雾上报数据结构体

typedef struct
{
	u8 upsw;			//热电堆上报控制
	u8 therdata;		//热电堆上报数据 
}therupstu;			//热电堆上报数据结构体

typedef struct
{
	u8 upsw;				//数据开关 1有效开始处理
	HTupstu htup;			//温湿度上报数据
	airquaupstu airquaup;	//空气质量上报数据
	co2upstu co2up;			//CO2上传数据
	pm25upstu	pm25up;		//PM2.5上报数据
	smokeupstu  smokeup;	//烟雾上报数据
	therupstu   therup;		//热电堆上报数据
}acqupstu;				//采集上报数据结构体

extern acqupstu acqdata;     //采集数据

/******************************************************
Fun:采集数据开始处理
Input:void
Output:void
Return:void
******************************************************/
void inline AcqDataStart()
{
	acqdata.upsw = 1;
}

extern void SensorDataHandle();
//////////////////////////////华丽的分割线///////////////////////////

#endif /*sensorapp.h*/
