#include "sensorapp.h"
#include "common.h"
#include "i2c.h"
#include "adc.h"

/*----------------------------��ʪ�Ȳɼ�----------------------------*/
htsensorstu htsensor;		//��ʪ�ȴ���������
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

/*----------------------------�������ɼ�----------------------------*/
acqdatastu acqdata;     //�ɼ�����

/******************************************************
Fun:�ɼ����ݲ��Դ���
Input:void
Output:void
Return:void
******************************************************/
void AcqDataTest()
{
	SensorLog("�ɼ����ݿ�ʼ����");
	memcpy(&acqdata.htdata.htvalue, &htvalue, sizeof(htacqstu));
	memcpy(&acqdata.airquadata, &airdata.airquadata, 1);
	if(0 == htvalue.tvalue.dir)
	{
		printf("�¶�:%.2f,ʪ��:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
	}
    else
    {
		printf("�¶�:%.2f,ʪ��:%.2f%\r\n", (float)(acqdata.htdata.htvalue.tvalue.value)/100, (float)(acqdata.htdata.htvalue.hvalue)/100);
    }
    printf("��������:%.2d%", acqdata.airquadata);
}

/******************************************************
Fun:�ɼ����������Ƿ�ʼ����
Input:void
Output:void
Return:TRUE:��ʼ�ɼ���FALSE:δ��ʼ�ɼ���ɼ�����
******************************************************/
_Bool IsAcqDataStart()
{
	return acqdata.datasw;
}

/******************************************************
Fun:��ʪ�ȴ������ɼ�����
Input:void
Output:void
Return:void
******************************************************/
inline void AcqDataStop()
{
	acqdata.datasw = 0;
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

/*----------------------------���������ɼ�----------------------------*/
airsensorstu airsensor;
airacqdatastu airdata;

/******************************************************
Fun:ADC�����ɼ���ʼ��
Input:void
Output:void
Return:0:��ʼ��ʧ�� 1:��ʼ���ɹ�
******************************************************/
_Bool ADCAirquaInit()
{
//	HAL_ADCEx_InjectedStart_IT(&hadc);
	HAL_ADC_Start_IT(&hadc);
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
		u8 vrl = 0;		//Vout(VRl)�����ѹ������0.1V
		u32 rsvalue = 0;	//Rs����ֵ������1

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

//////////////////////////////�����ķָ���///////////////////////////
