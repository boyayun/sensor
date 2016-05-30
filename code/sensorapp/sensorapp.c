#include "sensorapp.h"
#include "common.h"
#include "i2c.h"
#include "adc.h"

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
	memcpy(&acqdata.airquadata, &airdata.airquadata, 1);
	if(0 == htvalue.tvalue.dir)
	{
		printf("温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
	}
    else
    {
		printf("温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
    }
    printf("空气质量:%.2d%", acqdata.airquadata);
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

/*----------------------------空气质量采集----------------------------*/
airsensorstu airsensor;
airacqdatastu airdata;

/******************************************************
Fun:ADC空气采集初始化
Input:void
Output:void
Return:0:初始化失败 1:初始化成功
******************************************************/
_Bool ADCAirquaInit()
{
//	HAL_ADCEx_InjectedStart_IT(&hadc);
	HAL_ADC_Start_IT(&hadc);
	return 1;
}

/******************************************************
Fun:空气质量传感器采集是否开始
Input:void
Output:void
Return:TRUE:开始采集，FALSE:未开始采集或采集结束
******************************************************/
_Bool IsAirSensorAcqStart()
{
	return airsensor.acqflag;
}

/******************************************************
Fun:空气质量传感器采集结束
Input:void
Output:void
Return:void
******************************************************/
inline void AirSensorAcqStop()
{
	airsensor.acqflag = 0;
}

/******************************************************
Fun:空气质量传感器采集处理
Input:void
Output:void
Return:void
******************************************************/
void AirSensorAcqHandle()
{
	if(IsAirSensorAcqStart())
	{
        AirSensorAcqStop();

        u8 static num = 0;		//为了提高准确性，进行多次采样
		static u16 tempairacq = 0;
		u8 vrl = 0;		//Vout(VRl)输出电压，精度0.1V
		u32 rsvalue = 0;	//Rs电阻值，精度1

		tempairacq += airdata.airacqvalue;
		num++;
		if(3 == num)
		{
			num = 0;
			tempairacq /= 3;
			vrl = (u8)((float)tempairacq/4096*VREF);
			rsvalue = (VTEST-vrl)*RLVALUE/vrl;
			airdata.airquadata = rsvalue*100/ROVALUE;
			tempairacq = 0;
		}
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	static u8 num = 0;
	
	switch(num)
	{
		case 0:
			airdata.airacqvalue = hadc->Instance->DR;
			MODIFY_REG(hadc->Instance->SQR5,
				ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
		    	ADC_SQR5_RK(ADC_CHANNEL_2, 1));
			break;
		
		case 1:
			MODIFY_REG(hadc->Instance->SQR5,
               ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
               ADC_SQR5_RK(ADC_CHANNEL_1, 1));
			break;
		
		default:
			break;
	}
	num++;
	if(2 == num)
	{
		num = 0;
	}
	HAL_ADC_Start_IT(hadc);
}

//////////////////////////////华丽的分割线///////////////////////////
