#include "sensorapp.h"
#include "common.h"
#include "i2c.h"

/*----------------------------温湿度采集----------------------------*/
htsensorstu htsensor;		//温湿度传感器控制
htacqstu htvalue;			//温湿度值结构体

/******************************************************
Fun:温湿度传感器采集是否开始
Input:void
Output:void
Return:TRUE:开始采集，FALSE:未开始采集或采集结束
******************************************************/
_Bool IsHTSensorAcqStart()
{
	return htsensor.acqflag;
}

/******************************************************
Fun:温湿度传感器采集结束
Input:void
Output:void
Return:void
******************************************************/
inline void HTSensorAcqStop()
{
	htsensor.acqflag = 0;
}

/******************************************************
Fun:温湿度传感器复位
Input:void
Output:void
Return:0:复位失败 1:复位成功
******************************************************/
_Bool HTSensorReset()
{
    u8 temp[2] = {0};
    
    temp[0] = HTRESETCMD;
    if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &temp[0], 1, 1000))
	{
		return 0;
	}
	return 1;
}

/******************************************************
Fun:温湿度传感器初始化
Input:void
Output:void
Return:0:初始化失败 1:初始化成功
******************************************************/
_Bool HTSensorInit()
{
	u8 temp[2] = {0};
	
	temp[0] = HTRDREDCMD;
	if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &temp[0], 1, 1000))
	{
		return 0;
	}
	if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, &temp[1], 1, 1000))
	{
		return 0;
	}
	if(0 == (0x40 & temp[1]))
	{
		temp[0] = HTWRREDCMD;
		temp[1] &= 0x7E;		//湿度12位 温度14位，其余默认 
		if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, temp, 2, 1000))
		{
			return 0;
		}
		return 1;
	}
	else
	{
//		HTSensorReset();
		return 0;
	}
}

/******************************************************
Fun:判断传感器电压是否正常
Input:void
Output:void
Return:0:电压值不对或读出错误 1:电压正常
******************************************************/
_Bool IsHTSensorVolRight()
{
	u8 temp[2] = {0};
	
	temp[0] = HTRDREDCMD;
	if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &temp[0], 1, 1000))
	{
		return 0;
	}
	if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, &temp[1], 1, 1000))
	{
		return 0;
	}
	if(0 == (0x40 & temp[1]))	//0:电压正常 1:电压值低
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/******************************************************
Fun:湿度值采集并进行转换
Input:void
Output:tvalue:湿度值
Return:1采集转换成功 0失败
******************************************************/
_Bool HValueAcq(u16* hvalue)
{
	u8 i;
	u8 cmd = 0;
	u8 hdata[2] = {0};
	float tempacq = 0;
	u16 tempvalue = 0;

	cmd = HACQHOSTCMD;
	for(i=0; i<3; i++)			//读三次
	{
		if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &cmd, 1, 1000))
		{
			return 0;
		}
		if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, hdata, 2, 1000))
		{
			return 0;
		}
		if(0 == (hdata[1] & 0x02))		//0:温度 1:湿度
		{
			return 0;
		}
		tempacq += hdata[0] << 8 | (hdata[1] & 0xF0);       //最后4位置0
	}

    tempacq /= 3;
    tempvalue = (u16)(12500*(tempacq/65536));	//算法需要

	if(600 >= tempvalue)
	{
		return 0;
	}
	
    *hvalue = tempvalue - 600;			//算法需要
    return 1;
}

/******************************************************
Fun:温度值采集并进行转换
Input:void
Output:tvalue:温度值结构体
Return:1采集转换成功 0失败
******************************************************/
_Bool TValueAcq(tacqstu* tvalue)
{
	u8 i;
	u8 cmd = 0;
	u8 tdata[2] = {0};
	float tempacq = 0;
	u16 tempvalue = 0;

	cmd = TACQHOSTCMD;
	for(i=0; i<3; i++)			//读三次
	{
		if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &cmd, 1, 1000))
		{
			return 0;
		}
		if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, tdata, 2, 1000))
		{
			return 0;
		}
		if(0 != (tdata[1] & 0x02))		//0:温度 1:湿度
		{
			return 0;
		}
		tempacq += tdata[0] << 8 | (tdata[1] & 0xFC);       //最后两位置0
	}

    tempacq /= 3;
    tempvalue = (u16)(17572*(tempacq/65536));	//算法需要
    if(4685 <= tempvalue)		//算法需要
    {
        tvalue->dir = 0;		//正
        tvalue->value = tempvalue - 4685;
        
        
    }
    else
    {
		tvalue->dir = 1;		//负
        tvalue->value = 4685 - tempvalue;
    }
    
    return 1;
}

/******************************************************
Fun:温湿度传感器采集处理
Input:void
Output:void
Return:void
******************************************************/
void HTSensorAcqHandle()
{
	if(IsHTSensorAcqStart())
	{
        HTSensorAcqStop();
        
        if(!IsHTSensorVolRight())
        {
	      	SensorLog("温湿度传感器电压低");
            return;
        }
        
        if(!TValueAcq(&htvalue.tvalue))
        {
           SensorLog("温度采集失败");
        }

		if(!HValueAcq(&htvalue.hvalue))
		{
           SensorLog("湿度采集失败");
		}
	}
}
//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------传感器采集----------------------------*/
acqdatastu acqdata;     //采集数据

/******************************************************
Fun:采集数据测试处理
Input:void
Output:void
Return:void
******************************************************/
void AcqDataTest()
{
	SensorLog("采集数据开始处理");
	memcpy(&acqdata.htdata.htvalue, &htvalue, sizeof(htacqstu));
	if(0 == htvalue.tvalue.dir)
	{
		printf("温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
	}
    else
    {
		printf("温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
    }
}

/******************************************************
Fun:采集到的数据是否开始处理
Input:void
Output:void
Return:TRUE:开始采集，FALSE:未开始采集或采集结束
******************************************************/
_Bool IsAcqDataStart()
{
	return acqdata.datasw;
}

/******************************************************
Fun:温湿度传感器采集结束
Input:void
Output:void
Return:void
******************************************************/
inline void AcqDataStop()
{
	acqdata.datasw = 0;
}

/******************************************************
Fun:传感器数据处理主循环
Input:void
Output:void
Return:void
******************************************************/
void SensorDataHandle()                                                                      
{
	if(IsAcqDataStart())
	{
		AcqDataStop();

		//采集数据处理
		AcqDataTest();
	}
}
//////////////////////////////华丽的分割线///////////////////////////
