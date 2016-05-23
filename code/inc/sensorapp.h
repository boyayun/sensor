#ifndef __SENSORAPP_H
#define __SENSORAPP_H

#include "common.h"

#define SensorLog(M, ...)   custom_log("���ݲɼ�", M, ##__VA_ARGS__)

/*----------------------------��ʪ�Ȳɼ�----------------------------*/
#define HTSENSORADDR    0x80		//��ʪ�ȴ�����I2C��ַ

#define TACQHOSTCMD 	0xE3		//�¶Ȳɼ�������������
#define HACQHOSTCMD 	0xE5		//ʪ�Ȳɼ�������������
#define TACQUHOSTCMD 	0xF3		//�¶Ȳɼ���������������
#define HACQUHOSTCMD 	0xF5		//ʪ�Ȳɼ���������������
#define HTWRREDCMD		0xE6		//д�û��Ĵ���
#define HTRDREDCMD      0xE7		//���û��Ĵ���
#define HTRESETCMD		0xFE		//��λ

typedef struct
{
	u8 dir;			//�¶ȷ���:0Ϊ��,1Ϊ��
	u16 value;		//�¶�ֵ������Ϊ0.01���϶�
}tacqstu;			//�¶�ֵ�ṹ��

typedef struct
{
	u16 hvalue;			//ʪ��ֵ������Ϊ0.01%
	tacqstu tvalue;		
}htacqstu;			//��ʪ��ֵ�ṹ��

typedef struct
{
	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
}htsensorstu;			//��ʪ�ȴ��������ƽṹ��

extern htsensorstu htsensor;		//��ʪ�ȴ������ɼ��ṹ��

/******************************************************
Fun:��ʪ�ȴ������ɼ���ʼ
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
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------�������ɼ�----------------------------*/
typedef struct
{
	u8 datasw;				//��ʪ��ֵ�ϱ�����
	htacqstu htvalue;		
}HTdatastu;				//��ʪ�����ݽṹ��

typedef struct
{
	u8 datasw;		//���ݿ��� 1��Ч��ʼ����
	HTdatastu htdata;
}acqdatastu;		//�ɼ����ݽṹ��

extern acqdatastu acqdata;     //�ɼ�����

/******************************************************
Fun:�ɼ����ݿ�ʼ����
Input:void
Output:void
Return:void
******************************************************/
void inline AcqDataStart()
{
	acqdata.datasw = 1;
}

extern void SensorDataHandle();
//////////////////////////////�����ķָ���///////////////////////////

#endif /*sensorapp.h*/
