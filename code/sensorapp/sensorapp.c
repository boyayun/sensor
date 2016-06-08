#include "sensorapp.h"
#include "common.h"
#include "i2c.h"
#include "adc.h"

/*----------------------------��ʪ�Ȳɼ�----------------------------*/
sensoracqstu htsensor;		//��ʪ�ȴ���������
htacqstu htvalue;			//��ʪ��ֵ�ṹ��

/******************************************************
Fun:��ʪ�ȴ������ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsHTSensorAcqStart()
{
	return htsensor.acqflag;
}

/******************************************************
Fun:��ʪ�ȴ������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void HTSensorAcqStop()
{
	htsensor.acqflag = 0;
}

/******************************************************
Fun:��ʪ�ȴ�������λ
Input:void
Output:void
Return:0:��λʧ�� 1:��λ�ɹ�
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
Fun:��ʪ�ȴ�������ʼ��
Input:void
Output:void
Return:0:��ʼ��ʧ�� 1:��ʼ���ɹ�
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
		temp[1] &= 0x7E;		//ʪ��12λ �¶�14λ������Ĭ��
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
Fun:�жϴ�������ѹ�Ƿ�����
Input:void
Output:void
Return:0:��ѹֵ���Ի�������� 1:��ѹ����
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
	if(0 == (0x40 & temp[1]))	//0:��ѹ���� 1:��ѹֵ��
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/******************************************************
Fun:ʪ��ֵ�ɼ�������ת��
Input:void
Output:tvalue:ʪ��ֵ
Return:1�ɼ�ת���ɹ� 0ʧ��
******************************************************/
_Bool HValueAcq(u16* hvalue)
{
	u8 i;
	u8 cmd = 0;
	u8 hdata[2] = {0};
	float tempacq = 0;
	u16 tempvalue = 0;

	cmd = HACQHOSTCMD;
	for(i=0; i<3; i++)			//������
	{
		if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &cmd, 1, 1000))
		{
			return 0;
		}
		if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, hdata, 2, 1000))
		{
			return 0;
		}
		if(0 == (hdata[1] & 0x02))		//0:�¶� 1:ʪ��
		{
			return 0;
		}
		tempacq += hdata[0] << 8 | (hdata[1] & 0xF0);       //���4λ��0
	}

    tempacq /= 3;
    tempvalue = (u16)(12500*(tempacq/65536));	//�㷨��Ҫ

	if(600 >= tempvalue)
	{
		return 0;
	}
	
    *hvalue = tempvalue - 600;			//�㷨��Ҫ
    return 1;
}

/******************************************************
Fun:�¶�ֵ�ɼ�������ת��
Input:void
Output:tvalue:�¶�ֵ�ṹ��
Return:1�ɼ�ת���ɹ� 0ʧ��
******************************************************/
_Bool TValueAcq(tacqstu* tvalue)
{
	u8 i;
	u8 cmd = 0;
	u8 tdata[2] = {0};
	float tempacq = 0;
	u16 tempvalue = 0;

	cmd = TACQHOSTCMD;
	for(i=0; i<3; i++)			//������
	{
		if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c2, HTSENSORADDR, &cmd, 1, 1000))
		{
			return 0;
		}
		if(HAL_OK != HAL_I2C_Master_Receive(&hi2c2, HTSENSORADDR, tdata, 2, 1000))
		{
			return 0;
		}
		if(0 != (tdata[1] & 0x02))		//0:�¶� 1:ʪ��
		{
			return 0;
		}
		tempacq += tdata[0] << 8 | (tdata[1] & 0xFC);       //�����λ��0
	}

    tempacq /= 3;
    tempvalue = (u16)(17572*(tempacq/65536));	//�㷨��Ҫ
    if(4685 <= tempvalue)		//�㷨��Ҫ
    {
        tvalue->dir = 0;		//��
        tvalue->value = tempvalue - 4685;
    }
    else
    {
		tvalue->dir = 1;		//��
        tvalue->value = 4685 - tempvalue;
    }
    
    return 1;
}

/******************************************************
Fun:��ʪ�ȴ������ɼ�����
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
	      	SensorLog("��ʪ�ȴ�������ѹ��");
            return;
        }
        
        if(!TValueAcq(&htvalue.tvalue))
        {
           SensorLog("�¶Ȳɼ�ʧ��");
        }

		if(!HValueAcq(&htvalue.hvalue))
		{
           SensorLog("ʪ�Ȳɼ�ʧ��");
		}
	}
}
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------���������ɼ�----------------------------*/
sensoracqstu airsensor;
airacqdatastu airdata;

/******************************************************
Fun:ADC�����ɼ���ʼ��
Input:void
Output:void
Return:0:��ʼ��ʧ�� 1:��ʼ���ɹ�
******************************************************/
_Bool ADCAirquaInit()
{
	HAL_ADC_Start(&hadc);
//	HAL_ADC_Start_IT(&hadc);
	return 1;
}

/******************************************************
Fun:���������������ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsAirSensorAcqStart()
{
	return airsensor.acqflag;
}

/******************************************************
Fun:���������������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void AirSensorAcqStop()
{
	airsensor.acqflag = 0;
}

/******************************************************
Fun:���������������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
void AirSensorAcqHandle()
{
	if(IsAirSensorAcqStart())
	{
        AirSensorAcqStop();

        u8 static num = 0;		//Ϊ�����׼ȷ�ԣ����ж�β���
		static u16 tempairacq = 0;
		float vrl = 0;		//Vout(VRl)�����ѹ������0.1V
		u32 rsvalue = 0;	//Rs����ֵ������1

		airdata.airacqvalue = HAL_ADC_GetValue(&hadc);
		
		  ADC_ChannelConfTypeDef sConfig;

		  sConfig.Channel = ADC_CHANNEL_2;
  			sConfig.Rank = 1;
			sConfig.SamplingTime = ADC_SAMPLETIME_16CYCLES;
  			HAL_ADC_ConfigChannel(&hadc, &sConfig);
//		MODIFY_REG(hadc.Instance->SQR5, ADC_SQR5_RK(ADC_SQR5_SQ1, 1), ADC_SQR5_RK(ADC_CHANNEL_2, 1));
		SET_BIT(hadc.Instance->CR2, ADC_CR2_SWSTART);
		
		tempairacq += airdata.airacqvalue;
		num++;
		if(3 == num)
		{
			num = 0;
			tempairacq /= 3;
			vrl = (float)tempairacq/4096*VREF;
			rsvalue = (u32)((VTEST-vrl)*RLVALUE/vrl);
			airdata.airquadata = rsvalue*100/ROVALUE;
			tempairacq = 0;
		}
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	static u8 num = 0;
	
	co2data.acqvalue = hadc->Instance->DR;
//	switch(num)
//	{
//		case 0:
//			airdata.airacqvalue = hadc->Instance->DR;
//			MODIFY_REG(hadc->Instance->SQR5,
//				ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
//		    	ADC_SQR5_RK(ADC_CHANNEL_2, 1));
//			break;
//		
//		case 1:
//			co2data.acqvalue = hadc->Instance->DR;
//			MODIFY_REG(hadc->Instance->SQR5,
//               ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
//               ADC_SQR5_RK(ADC_CHANNEL_4, 1));
//			break;
//			
//		case 2:
//		   smokedata.acqvalue = hadc->Instance->DR;
//		   	MODIFY_REG(hadc->Instance->SQR5,
//               ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
//               ADC_SQR5_RK(ADC_CHANNEL_5, 1));
//
//        case 3:
//        	therdata.acqvalue = hadc->Instance->DR;
//		   	MODIFY_REG(hadc->Instance->SQR5,
//               ADC_SQR5_RK(ADC_SQR5_SQ1, 1),
//               ADC_SQR5_RK(ADC_CHANNEL_1, 1));
//               
//		default:
//			break;
//	}
	num++;
	if(4 == num)
	{
		num = 0;
	}
	HAL_ADC_Start_IT(hadc);
}
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------CO2�ɼ�----------------------------*/
const u16 co2list[CO2LISTNUM]={
  /* 0,   1,   2,   3,   4,   5,   6,   7,   8,   9, */
	350, 360, 370 ,380, 390, 400, 420, 440, 460, 480, 
	500, 525, 550, 575, 600, 625, 650, 675, 700, 725, 
	750, 780, 810, 840, 870, 900, 930, 960, 990, 1020,
	1070,1120,1170,1220,1270,1320,1370,1420,1470,1520,
	1570,1620,1670,1720,1770,1820,1870,1920,1970,2020,
	2100,2200,2300,2400,2500,2600,2700,2800,2900,3000,
	3150,3300,3450,3600,3750,3900,4050,4250,4450,4650,
	4850,5050,5250,5450,5650,5850,6050,6300,6550,6800,
	7050,7300,7550,7800,8050,8450,8850,9250,9650,10000
};//CO2���ݵ�ֵ

sensoracqstu co2sensor;
co2datastu co2data;

/******************************************************
Fun:CO2�������ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsCo2SensorAcqStart()
{
	return co2sensor.acqflag;
}

/******************************************************
Fun:CO2�������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void Co2SensorAcqStop()
{
	co2sensor.acqflag = 0;
}

/******************************************************
Fun:CO2�������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
void Co2SensorAcqHandle()
{
	if(IsCo2SensorAcqStart())
	{
        Co2SensorAcqStop();

        u8 static num = 0;		//Ϊ�����׼ȷ�ԣ����ж�β���
		static u16 tempco2acq = 0;
		u16 emf = 0;		//�����ѹ

		co2data.acqvalue = HAL_ADC_GetValue(&hadc);
//				  ADC_ChannelConfTypeDef sConfig;
//
//		  sConfig.Channel = ADC_CHANNEL_4;
//  			sConfig.Rank = 1;
//			sConfig.SamplingTime = ADC_SAMPLETIME_16CYCLES;
//  			HAL_ADC_ConfigChannel(&hadc, &sConfig);
			//MODIFY_REG(hadc.Instance->SQR5, ADC_SQR5_RK(ADC_SQR5_SQ1, 1), ADC_SQR5_RK(ADC_CHANNEL_4, 1));
		SET_BIT(hadc.Instance->CR2, ADC_CR2_SWSTART);
		
		tempco2acq += co2data.acqvalue;
		num++;
		if(3 == num)
		{
			num = 0;
			tempco2acq /= 3;
			emf = (u16)((float)tempco2acq/4096*CO2VREF/CO2FACTOR);

//			emf = 200;	//TEST
			
			if(emf <= 300)
			{
				co2data.acqdata = co2list[300 - emf];
			}
			else
			{
				co2data.acqdata = 10000;
			}
			tempco2acq = 0;
		}
    }
}
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------PM2.5�ɼ�----------------------------*/
pm25acqstu	pm25acq;
pm25datastu	pm25data;

/******************************************************
Fun:PM2.5�ɼ���ʼ��
Input:void
Output:void
Return:0:��ʼ��ʧ�� 1:��ʼ���ɹ�
******************************************************/
_Bool PM25AcqInit()
{
	return 1;
}

/******************************************************
Fun:�Ƿ�PM2.5�������ɼ����ڵ�
Input:void
Output:void
Return:TRUE:�ɼ����ڵ���FALSE:�ɼ�����û�е�
******************************************************/
_Bool IsPM25PeriodTimeUp()
{
	return pm25acq.periodtimeup;
}

/******************************************************
Fun:PM2.5�ɼ�����ʱ��δ��
Input:void
Output:void
Return:void
******************************************************/
inline void PM25PeriodTimeNoUp()
{
	pm25acq.periodtimeup = 0;
}

/******************************************************
Fun:�����������AQI
Input:pm2.5��ֵugÿ������
Output:void
Return:void
******************************************************/
void AqiCalculate(u16 pm25value)
{
	if(pm25value <= 500)
	{
		u16 aqih = 0;	//��BPHi��Ӧ�Ŀ���������ָ��
		u16 aqil = 0;	//��BPLo��Ӧ�Ŀ���������ָ��
		u16 pmh = 0;	//��CP�������Ⱦ��Ũ����ֵ�ĸ�λֵ
		u16 pml = 0;	//��CP�������Ⱦ��Ũ����ֵ�ĵ�λֵ
		
		if(pm25value <= 35)				//��
		{
			aqil = 0;
			aqih = 50;
			pml = 0;
			pmh = 35;
		}
		else if((35 < pm25value) && (pm25value <= 75))		//��
		{
			aqil = 50;
			aqih = 100;
			pml = 35;
			pmh = 75;
		}
		else if((75 < pm25value) && (pm25value <= 115))		//�����Ⱦ
		{
			aqil = 100;
			aqih = 150;
			pml = 75;
			pmh = 115;
		}
		else if((115 < pm25value) && (pm25value <= 150))		//�ж���Ⱦ
		{
			aqil = 150;
			aqih = 200;
			pml = 115;
			pmh = 150;
		}
		else if((150 < pm25value) && (pm25value <= 250))		//�ض���Ⱦ
		{
			aqil = 200;
			aqih = 300;
			pml = 150;
			pmh = 250;
		}
		else if((250 < pm25value) && (pm25value <= 350))		//������Ⱦ
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
	else	//AQI����
	{
		pm25data.aqi = 500;
	}
}

/******************************************************
Fun:PM2.5�͵�ƽ�ɼ���ʼ
Input:void
Output:void
Return:void
******************************************************/
void inline PM25LowlevAcqStart()
{
	pm25acq.lowlevacqstart = 1;
}

/******************************************************
Fun:PM2.5�ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsPM25LowlevAcqStart()
{
	return pm25acq.lowlevacqstart;
}

/******************************************************
Fun:PM2.5�͵�ƽ�ɼ�ֹͣ
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
Fun:PM2.5�������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
void PM25AcqHandle()
{
	if(IsPM25PeriodTimeUp())
	{
		PM25PeriodTimeNoUp();
		
		float rt = 0;		//�������ʣ�RT=LT/UTx100%

		rt = (float)pm25data.lowlevvalue / PM25PERIOD*100;
		pm25data.countratio = (u16)(rt*100/PMFACTOR*1000);
		pm25data.massratio = (u16)((float)pm25data.countratio*MALMASS/VOLCONVERT/NAFACTOR);
		AqiCalculate(pm25data.massratio);

		pm25data.lowlevvalue = 0;
	}
}

//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------���������ɼ�----------------------------*/
const u16 smokelist[SMOKELISTNUM]={
  /* 0,   1,   2,   3,   4,   5,   6,   7,   8,  9, */
    200, 186, 172, 160, 148, 136, 124, 112, 100, 92, 
     85,  78,  71,  64,  57,  50,  45,  40,  35, 30,
	25,  23,  22,  20,  19,  17,  16,  14,  13,  11, 
	 10,  8,   6,   4,   2,   0
};//�������ݵ�ֵ

sensoracqstu smokeacq;	//����ɼ��ṹ��
smokedatastu smokedata;

/******************************************************
Fun:���������ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsSmokeSensorAcqStart()
{
	return smokeacq.acqflag;
}

/******************************************************
Fun:���������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void SmokeSensorAcqStop()
{
	smokeacq.acqflag = 0;
}

/******************************************************
Fun:���������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
void SmokeSensorAcqHandle()
{
	if(IsSmokeSensorAcqStart())
	{
        SmokeSensorAcqStop();

        u8 static num = 0;		//Ϊ�����׼ȷ�ԣ����ж�β���
		static u16 tempsmokeacq = 0;
		u8 vout = 0;			//�����ѹ

		smokedata.acqvalue = HAL_ADC_GetValue(&hadc);
//		  ADC_ChannelConfTypeDef sConfig;
//
//		  sConfig.Channel = ADC_CHANNEL_4;
//			sConfig.Rank = 1;
//			sConfig.SamplingTime = ADC_SAMPLETIME_16CYCLES;
//			HAL_ADC_ConfigChannel(&hadc, &sConfig);
//		MODIFY_REG(hadc.Instance->SQR5, ADC_SQR5_RK(ADC_SQR5_SQ1, 1), ADC_SQR5_RK(ADC_CHANNEL_2, 1));
		SET_BIT(hadc.Instance->CR2, ADC_CR2_SWSTART);

		tempsmokeacq += smokedata.acqvalue;
		num++;
		if(3 == num)
		{
			num = 0;
			tempsmokeacq /= 3;
			vout  = (u8)((float)tempsmokeacq/4096*SMOKEVREF);

//			vout = 30;	//test
			if(vout <= 56)
			{
				smokedata.acqdata = smokelist[vout-20];
			}
			else
			{
				smokedata.acqdata = 200;
			}
			tempsmokeacq = 0;
		}
    }
}

//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------�������ɼ�----------------------------*/
sensoracqstu theracq;	//�ȵ�Ѳɼ��ṹ��
therdatastu therdata; 	//�ȵ�����ݽṹ��

/******************************************************
Fun:�ȵ�Ѵ������ɼ��Ƿ�ʼ
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsTherSensorAcqStart()
{
	return theracq.acqflag;
}

/******************************************************
Fun:�ȵ�Ѵ������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void TherSensorAcqStop()
{
	theracq.acqflag = 0;
}

/******************************************************
Fun:�����ȵ���¶�
Input:�ȵ�ѵ�ѹ
Output:void
Return:void
******************************************************/
u8 CalculateTherTemp(float vtp)
{
	#define Sconv  	0.982	//������У������
	#define Alow 	0.06	//��������a
	#define Blow	1.5		//��������b
	#define Ahigh 	0.08	//��������a
	#define Bhigh	2.8		//��������b

	float vtpcorr = 0;	//У������¶�
	float tsen = 0;		//�������¶�
	float tcf = 0;		//TC factor
	float vtpreftc = 0; //�����¶�	
	
	vtpcorr = vtp / Sconv;
	if(0 == htvalue.tvalue.dir)
	{
		tsen = (float)htvalue.tvalue.value/100;
	}
	else
	{
		tsen = 0 - (float)htvalue.tvalue.value/100;
	}

	tcf = 1-(tsen-25)*0.0045;

	if(tsen <= 60)		//�����¶Ȳ�
	{
		vtpreftc = tsen*Alow - Blow;
	}
	else
	{
		vtpreftc = tsen*Ahigh - Bhigh;
	}
	
	vtpreftc *= tcf;		//�����¶Ȳ���
	vtpreftc += vtpcorr;	//�����ȵ�ѵ�ѹ
	vtpreftc /= tcf;		//TCFУ����ĵ�ѹ

	if(vtpreftc <= 2)
	{
		return (u8)((vtpreftc+Blow)/Alow);
	}
	else
	{
		return (u8)((vtpreftc+Bhigh)/Ahigh);
	}
}

/******************************************************
Fun:�ȵ�Ѵ������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
void TherSensorAcqHandle()
{
	if(IsTherSensorAcqStart())
	{
        TherSensorAcqStop();

        u8 static num = 0;		//Ϊ�����׼ȷ�ԣ����ж�β���
		static u16 temptheracq = 0;
		float vtp = 0;			//�ȵ�ѵ�ѹ

		temptheracq += smokedata.acqvalue;
		num++;
		if(3 == num)
		{
			num = 0;
			temptheracq /= 3;
//			temptheracq = 500;	//TEST
			vtp  = (float)temptheracq/4096*THERVREF;		//0.01mv
			therdata.acqdata = CalculateTherTemp(vtp);
			temptheracq = 0;
		}
    }
}

//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------�������ɼ�----------------------------*/
acqupstu acqdata;     //�ɼ�����

/******************************************************
Fun:�ɼ����ݲ��Դ���
Input:void
Output:void
Return:void
******************************************************/
void AcqDataTest()
{
	SensorLog("�ɼ����ݿ�ʼ����");
	memcpy(&acqdata.htup.htvalue, &htvalue, sizeof(htacqstu));
	if(0 == htvalue.tvalue.dir)
	{
		printf("    �¶�:%.2f,ʪ��:%.2f%\r\n", (float)(acqdata.htup.htvalue.tvalue.value)/100, (float)(acqdata.htup.htvalue.hvalue)/100);
	}
    else
    {
		printf("    �¶�:%.2f,ʪ��:%.2f%\r\n", (float)(acqdata.htup.htvalue.tvalue.value)/100, (float)(acqdata.htup.htvalue.hvalue)/100);
    }
    
    acqdata.airquaup.airquavalue = airdata.airquadata;
    printf("    ��������:%.2d%\r\n", acqdata.airquaup.airquavalue);

	memcpy(&acqdata.co2up.co2data, &co2data.acqdata, 2);
	printf("    CO2Ũ��:%.2dppm\r\n", acqdata.co2up.co2data);
	if(10000 ==	acqdata.co2up.co2data)
	{
		SensorLog("co2�������ɼ�������Χ");
	}

    memcpy(&acqdata.pm25up.countratio, &pm25data.countratio, 6);
    printf("    PM2.5������:%.2d/283mil, ����Ũ��:%.2dug/m3, ��������ָ��AQI: %.2d\r\n", acqdata.pm25up.countratio, acqdata.pm25up.massratio, acqdata.pm25up.aqi);

    memcpy(&acqdata.smokeup.smokedata, &smokedata.acqdata, 1);
    printf("    ����Ũ��: %.2f%\r\n", (float)acqdata.smokeup.smokedata/10);
	if(200 == smokedata.acqdata)
	{
		SensorLog("���������ɼ�������Χ");
	}

    memcpy(&acqdata.therup.therdata, &therdata.acqdata, 1);
    printf("    �������: %.2d\r\n", acqdata.therup.therdata);
    
	if(acqdata.therup.therdata >= 40)
	{
		SensorLog("�����¶��쳣");
	}
	else if(acqdata.therup.therdata >= HUMANTEMP)
	{
		SensorLog("��������");
	}
	else
	{
		SensorLog("��������");
	}
}

/******************************************************
Fun:�ɼ����������Ƿ�ʼ����
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsAcqDataStart()
{
	return acqdata.upsw;
}

/******************************************************
Fun:��ʪ�ȴ������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void AcqDataStop()
{
	acqdata.upsw = 0;
}

/******************************************************
Fun:���������ݴ�����ѭ��
Input:void
Output:void
Return:void
******************************************************/
void SensorDataHandle()                                                                      
{
	if(IsAcqDataStart())
	{
		AcqDataStop();

		//�ɼ����ݴ���
		AcqDataTest();
	}
}
//////////////////////////////�����ķָ���///////////////////////////

