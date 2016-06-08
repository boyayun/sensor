#ifndef __SENSORAPP_H
#define __SENSORAPP_H

#include "common.h"

#define SensorLog(M, ...)   custom_log("���ݲɼ�", M, ##__VA_ARGS__)
typedef struct
{
	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
}sensoracqstu;			//���������ƽṹ��

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

//typedef struct
//{
//	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
//}htsensorstu;			//��ʪ�ȴ��������ƽṹ��

extern sensoracqstu htsensor;		//��ʪ�ȴ������ɼ��ṹ��

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

/*----------------------------���������������ɼ�----------------------------*/
#define ROVALUE 16000	//RO��ֵ:�������еĴ���������ֵ
#define RLVALUE 10000	//RL��ֵ:���ص����ֵ
#define VREF   30		//�ο���ѹ 3V,����0.1
#define VTEST  50		//���Ե�ѹ 5V������0.1

//typedef struct
//{
//	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
//}airsensorstu;		//���������ɼ��ṹ��

typedef struct
{
	u16 airacqvalue;	//�ɼ��ĵ�ѹֵ
	u8 airquadata;		//������������ RS/RO������0.01
}airacqdatastu;		//�����������ݽṹ��

extern sensoracqstu airsensor;
extern airacqdatastu airdata;
/******************************************************
Fun:���������ɼ���ʼ
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
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------CO2�������ɼ�----------------------------*/
#define CO2LISTNUM 90	//CO2���ݵĸ���
#define CO2VREF   3000  //�ο���ѹ 3000mV
#define CO2FACTOR 5	    //co2��ѹ�Ŵ���

//typedef struct
//{
//	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
//}co2acqstu;			//CO2�ɼ��ṹ��

typedef struct
{
	u16 acqvalue;	//�ɼ��ĵ�ѹֵ
	u16 acqdata;		//CO2���� ppm
}co2datastu;		//CO2���ݽṹ��

extern sensoracqstu co2sensor;
extern co2datastu co2data;

/******************************************************
Fun:CO2�������ɼ���ʼ
Input:void
Output:void
Return:void
******************************************************/
void inline Co2SensorAcqStart()
{
	co2sensor.acqflag = 1;
}

extern void Co2SensorAcqHandle();
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------PM2.5�ɼ�----------------------------*/
#define PMFACTOR 	160		//pm2.5�ɼ�����б��a��ֵ   ����0.01  rt = a*pm2.5+k
#define PMOFFSET	0		//pm2.5�ɼ�����ƫ����k��ֵ ����0.01  rt = a*pm2.5+k

#define VOLCONVERT	283		//�����λת����ת����ml  10��-6�η�
#define NAFACTOR	6		//�����ӵ��޳���  10��23�η�
#define MALMASS		270		//Ħ������ 270*10��8�η�g/mol

#define PM25PERIOD  30000	//PM2.5�ɼ����� 30000ms 

typedef struct
{
	u8 periodtimeup;	//30s���ڲɼ�ʱ�䵽 1ʱ�䵽 0ʱ��δ��
	u8 lowlevacqstart;	//PM2.5�͵�ƽ�ɼ���ʼ
}pm25acqstu;			//PM2.5�ɼ��ṹ��

typedef struct
{
	u16 lowlevvalue;	//�͵�ƽֵ
	u16 countratio;		//������/283mil
	u16 massratio;		//����Ũ��/������
	u16 aqi;			//��������ָ��
}pm25datastu;			//PM2.5���ݽṹ��

extern pm25acqstu	pm25acq;
extern pm25datastu	pm25data;

/******************************************************
Fun:PM2.5�ɼ�����ʱ�䵽
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
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------���������ɼ�----------------------------*/
#define SMOKELISTNUM 36 //�������ݸ���
#define SMOKEVREF  60	//�ο���ѹ  ����0.01V
#define HUMANTEMP 25	//��������¶�
//typedef struct
//{
//	_Bool acqflag;		//ȷ��ʲô���вɼ���1��Ч
//}smokeacqstu;			//����ɼ��ṹ��

typedef struct
{
	u16 acqvalue;	//�ɼ��ĵ�ѹֵ
	u8 acqdata;		//�������� %
}smokedatastu;	//�������ݽṹ��

extern sensoracqstu smokeacq;	//����ɼ��ṹ��
extern smokedatastu smokedata; //�������ݽṹ��
/******************************************************
Fun:���������ɼ���ʼ
Input:void
Output:void
Return:void
******************************************************/
void inline SmokeSensorAcqStart()
{
	smokeacq.acqflag = 1;
}

extern void SmokeSensorAcqHandle();
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------�ȵ�Ѵ������ɼ�----------------------------*/
#define THERLISTNUM 21 //�ȵ�����ݸ���
#define THERVREF  30	//�ο���ѹ  ����1mV

typedef struct
{
	u16 acqvalue;	//�ɼ��ĵ�ѹֵ
	u8 acqdata;		
}therdatastu;		//�ȵ�����ݽṹ��

extern sensoracqstu theracq;	//�ȵ�Ѳɼ��ṹ��
extern therdatastu therdata; 	//�ȵ�����ݽṹ��

/******************************************************
Fun:�ȵ�Ѵ������ɼ���ʼ
Input:void
Output:void
Return:void
******************************************************/
void inline TherSensorAcqStart()
{
	theracq.acqflag = 1;
}

extern void TherSensorAcqHandle();
//////////////////////////////�����ķָ���///////////////////////////

/*----------------------------�������ɼ�----------------------------*/

typedef struct
{
	u8 upsw;				//��ʪ��ֵ�ϱ�����
	htacqstu htvalue;		
}HTupstu;				//��ʪ���ϱ����ݽṹ��

typedef struct
{
	u8 upsw;				//���������ϱ�����
	u8 airquavalue;			//���������ϱ����� RS/RO
}airquaupstu;

typedef struct
{
	u8 upsw;			//CO2�ϱ�����
	u16 co2data;			//CO2�ϱ����� ppm
}co2upstu;

typedef struct
{
	u8 upsw;			//PM2.5�ϱ�����
	u16 countratio;		//������/283mil
	u16 massratio;		//����Ũ��/������
	u16 aqi;			//��������ָ��	
}pm25upstu;			//pm2.5�ϱ����ݽṹ��

typedef struct
{
	u8 upsw;			//�����ϱ�����
	u8 smokedata;		//�����ϱ����� %
}smokeupstu;			//�����ϱ����ݽṹ��

typedef struct
{
	u8 upsw;			//�ȵ���ϱ�����
	u8 therdata;		//�ȵ���ϱ����� 
}therupstu;			//�ȵ���ϱ����ݽṹ��

typedef struct
{
	u8 upsw;				//���ݿ��� 1��Ч��ʼ����
	HTupstu htup;			//��ʪ���ϱ�����
	airquaupstu airquaup;	//���������ϱ�����
	co2upstu co2up;			//CO2�ϴ�����
	pm25upstu	pm25up;		//PM2.5�ϱ�����
	smokeupstu  smokeup;	//�����ϱ�����
	therupstu   therup;		//�ȵ���ϱ�����
}acqupstu;				//�ɼ��ϱ����ݽṹ��

extern acqupstu acqdata;     //�ɼ�����

/******************************************************
Fun:�ɼ����ݿ�ʼ����
Input:void
Output:void
Return:void
******************************************************/
void inline AcqDataStart()
{
	acqdata.upsw = 1;
}

extern void SensorDataHandle();
//////////////////////////////�����ķָ���///////////////////////////

#endif /*sensorapp.h*/
