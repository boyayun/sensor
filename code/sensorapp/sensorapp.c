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

/*----------------------------PM2.5采集----------------------------*/
pm25acqstu	pm25acq;
pm25datastu	pm25data;

/******************************************************
Fun:PM2.5采集初始化
Input:void
Output:void
Return:0:初始化失败 1:初始化成功
******************************************************/
_Bool PM25AcqInit()
{
	return 1;
}

/******************************************************
Fun:是否PM2.5传感器采集周期到
Input:void
Output:void
Return:TRUE:采集周期到，FALSE:采集周期没有到
******************************************************/
_Bool IsPM25PeriodTimeUp()
{
	return pm25acq.periodtimeup;
}

/******************************************************
Fun:PM2.5采集周期时间未到
Input:void
Output:void
Return:void
******************************************************/
inline void PM25PeriodTimeNoUp()
{
	pm25acq.periodtimeup = 0;
}

/******************************************************
Fun:计算空气质量AQI
Input:pm2.5的值ug每立方米
Output:void
Return:void
******************************************************/
void AqiCalculate(u16 pm25value)
{
	if(pm25value <= 500)
	{
		u16 aqih = 0;	//与BPHi对应的空气质量分指数
		u16 aqil = 0;	//与BPLo对应的空气质量分指数
		u16 pmh = 0;	//与CP相近的污染物浓度限值的高位值
		u16 pml = 0;	//与CP相近的污染物浓度限值的低位值
		
		if(pm25value <= 35)				//优
		{
			aqil = 0;
			aqih = 50;
			pml = 0;
			pmh = 35;
		}
		else if((35 < pm25value) && (pm25value <= 75))		//良
		{
			aqil = 50;
			aqih = 100;
			pml = 35;
			pmh = 75;
		}
		else if((75 < pm25value) && (pm25value <= 115))		//轻度污染
		{
			aqil = 100;
			aqih = 150;
			pml = 75;
			pmh = 115;
		}
		else if((115 < pm25value) && (pm25value <= 150))		//中度污染
		{
			aqil = 150;
			aqih = 200;
			pml = 115;
			pmh = 150;
		}
		else if((150 < pm25value) && (pm25value <= 250))		//重度污染
		{
			aqil = 200;
			aqih = 300;
			pml = 150;
			pmh = 250;
		}
		else if((250 < pm25value) && (pm25value <= 350))		//严重污染
		{
			aqil = 300;
			aqih = 400;
			pml = 250;
			pmh = 350;
		}
		else
		{
			aqil = 400;
			aqih = 500;
			pml = 350;
			pmh = 500;
		}

		pm25data.aqi = (pm25value - pml)*(aqih-aqil)/(pmh-pml)+aqil;
	}
	else	//AQI爆表
	{
		pm25data.aqi = 500;
	}
}

/******************************************************
Fun:PM2.5低电平采集开始
Input:void
Output:void
Return:void
******************************************************/
void inline PM25LowlevAcqStart()
{
	pm25acq.lowlevacqstart = 1;
}

/******************************************************
Fun:PM2.5采集是否开始
Input:void
Output:void
Return:TRUE:开始采集，FALSE:未开始采集或采集结束
******************************************************/
_Bool IsPM25LowlevAcqStart()
{
	return pm25acq.lowlevacqstart;
}

/******************************************************
Fun:PM2.5低电平采集停止
Input:void
Output:void
Return:void
******************************************************/
inline void PM25LowlevAcqStop()
{
	pm25acq.lowlevacqstart = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3))
	{
		PM25LowlevAcqStop();
	}
	else
	{
		PM25LowlevAcqStart();
	}
}

/******************************************************
Fun:PM2.5传感器采集处理
Input:void
Output:void
Return:void
******************************************************/
void PM25AcqHandle()
{
	if(IsPM25PeriodTimeUp())
	{
		PM25PeriodTimeNoUp();
		
		float rt = 0;		//低脉冲率：RT=LT/UTx100%

		rt = (float)pm25data.lowlevvalue / PM25PERIOD*100;
		pm25data.countratio = (u16)(rt*100/PMFACTOR*1000);
		pm25data.massratio = (u16)((float)pm25data.countratio*MALMASS/VOLCONVERT/NAFACTOR);
		AqiCalculate(pm25data.massratio);

		pm25data.lowlevvalue = 0;
	}
}

//////////////////////////////华丽的分割线///////////////////////////

/*----------------------------传感器采集----------------------------*/
acqupstu acqdata;     //采集数据

/******************************************************
Fun:采集数据测试处理
Input:void
Output:void
Return:void
******************************************************/
void AcqDataTest()
{
	SensorLog("采集数据开始处理");
	memcpy(&acqdata.htup.htvalue, &htvalue, sizeof(htacqstu));
	if(0 == htvalue.tvalue.dir)
	{
		printf("    温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htup.htvalue.tvalue.value)/100, (float)(acqdata.htup.htvalue.hvalue)/100);
	}
    else
    {
		printf("    温度:%.2f,湿度:%.2f%\r\n", (float)(acqdata.htup.htvalue.tvalue.value)/100, (float)(acqdata.htup.htvalue.hvalue)/100);
    }
    
    acqdata.airquaup.airquavalue = airdata.airquadata;
    printf("    空气质量:%.2d%\r\n", acqdata.airquaup.airquavalue);

    memcpy(&acqdata.pm25up.countratio, &pm25data.countratio, 6);
    printf("    PM2.5粒子数:%.2d/283mil, 质量浓度:%.2dug/m3, 空气质量指数AQI: %.2d\r\n", acqdata.pm25up.countratio, acqdata.pm25up.massratio, acqdata.pm25up.aqi);
}

/******************************************************
Fun:采集到的数据是否开始处理
Input:void
Output:void
Return:TRUE:开始采集，FALSE:未开始采集或采集结束
******************************************************/
_Bool IsAcqDataStart()
{
	return acqdata.upsw;
}

/******************************************************
Fun:温湿度传感器采集结束
Input:void
Output:void
Return:void
******************************************************/
inline void AcqDataStop()
{
	acqdata.upsw = 0;
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

