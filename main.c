#include <AT89X52.h>
#include <Intrins.h>
#include <math.h>
#include <2pt.h>

typedef unsigned int u16;
typedef unsigned char u8;


//���Ŷ���
#define		DATA	P1          //LCD1602���ݶ˿�
sbit 		  DQ =  P2^7;			  //DS18B20���ݶ˿�
sbit 		  RS =	P2^0;       //LCD1602 RS
sbit 		  RW =	P2^1;       //LCD1602 RW
sbit 		  E  =	P2^2;       //LCD1602 E
sbit      hall = P3^2;      //����ˮ����������������
sbit      led  = P2^5;      //����ˮ���������һ������LEDȡ��һ��


//�¶����ݴ洢�ṹ
typedef struct tagTempData
{
	u8 					btThird;			//��λ����					
	u8 					btSecond;			//ʮλ����
	u8 					btFirst;			//��λ����
	u8 					btDecimal;		//С�����һλ����
	u8					btNegative;		//�Ƿ�Ϊ����		
}TEMPDATA;
TEMPDATA m_TempData;


//DS18B20���к�
const u8 code ROMData1[8] = {0x28, 0x70, 0x62, 0x77, 0x91, 0x0b, 0x02, 0x26};	//Uin
const u8 code ROMData2[8] = {0x28, 0xe8, 0x8d, 0x77, 0x91, 0x19, 0x02, 0xfe};	//Uout


/*************************************ˮ��������غ���************************************/
//��ر�������
static u16 i;               //��ʱ��0������
static u16 count=0;         //��ˮ������1s�ڵ��������
static u16 Hz=1000;         //��¼1s�ڵ�������� ��Ƶ��

static float velocity;      //ˮ������
static float power;         //��������Ĺ���


//�ⲿ�ж�0������ˮ�������������
void Int0Init()
{
	IT0=1;                    //�½��ش���
	EX0=1;
	EA=1;
}

//�������յ������� ÿ����һ��led��˸һ��
void Int0()	interrupt 0
{
	count++;
	if(hall==0)
	{
		led=~led; 
	}
}


//��ʱ��0���1s�� hall ����������
void Timer0Init()
{
	TMOD=0x01;
	TH0=(65536-1000)/256;
	TL0=(65536-1000)%256;
	ET0=1;
	EA=1;
	TR0=1;
}

//ÿ1ms*1000=1s��¼һ�α����ڵ����������Hz
void Timer0() interrupt 1
{
	TH0=(65536-1000)/256;
	TL0=(65536-1000)%256;
	i++;
	if(i==1000)
		{
			i=0;
			Hz=count; 
			count=0;			
		}
}
/*************************************ˮ��������غ���************************************/


/**************************************DS18B20��غ���**************************************/
//оƬ��ʼ������
void Initialization()
{
	while(1)
	{
		DQ = 0;
		Delay480us();
		DQ = 1;
		Delay60us();
		if(!DQ)  				    //�յ�ds18b20��Ӧ���ź�
		{	
			DQ = 1;
			Delay240us();		  //��ʱ240us
			break;		
		}
	}
}

//�ӵ�λ��ʼдһ���ֽ�
void WriteByte(u8 btData)
{
	u8 i, btBuffer;
	for (i = 0; i < 8; i++)
	{
		btBuffer = btData >> i;
		if (btBuffer & 1)
		{
			DQ = 0;
			_nop_();
			_nop_();
			DQ = 1;
			Delay60us();
		}
		else
		{
			DQ = 0;
			Delay60us();
			DQ = 1;			
		}
	}
}

//�ӵ�λ��ʼ��һ���ֽ�
u8 ReadByte()
{
	u8 i, btDest;
	for (i = 0; i < 8; i++)
	{
		btDest >>= 1;
		DQ = 0;
		_nop_();
		_nop_();
		DQ = 1;
		Delay16us();
		if (DQ) btDest |= 0x80; 
		Delay60us();
	}
	return btDest;
}

//���к�ƥ�亯��
void MatchROM(const u8 *pMatchData)
{
	u8 i;
	Initialization();
	WriteByte(MATCH_ROM);
	for (i = 0; i < 8; i++) WriteByte(*(pMatchData + i));	
}

//��ȡ�¶�ֵ
static TEMPDATA TempData1,TempData2;

TEMPDATA ReadTemperature()
{
	TEMPDATA TempData;
	u16 iTempDataH;
	u8 btDot, iTempDataL;
	static u8 i = 0;

	TempData.btNegative = 0;						//�¶�Ϊ��
	i++;
	if (i==3)                           //N��18B20����������Ϊi==N+1  
		i = 1;
	Initialization();
	WriteByte(SKIP_ROM);							  //����ROMƥ��
	WriteByte(TEMP_SWITCH);							//����ת��
	Delay500ms();  									    //����һ�ξ���	
	Delay500ms(); 	 		
	Initialization();

	switch (i)
	{
		case 1 : MatchROM(ROMData1); break;			//ƥ��1
		case 2 : MatchROM(ROMData2); break;			//ƥ��2
	}
	WriteByte(READ_MEMORY);						  //������
	iTempDataL = ReadByte();
	iTempDataH = ReadByte();	
	iTempDataH <<= 8;
	iTempDataH |= iTempDataL;

	if (iTempDataH & 0x8000)
	{
		TempData.btNegative = 1;
		iTempDataH = ~iTempDataH + 1;		  //������
	}

	//Ϊ��ʡȥ������������Ŀ�����������������С�����ַֿ�����ķ���(û����������)
	btDot = (u8)(iTempDataH & 0x000F);	//�õ�С������
	iTempDataH >>= 4;							      //�õ���������
	btDot *= 5; 									      //btDot*10/16�õ�ת�����С������
	btDot >>= 3;

	//���ݴ���
	TempData.btThird   = (u8)iTempDataH / 100;
	TempData.btSecond  = (u8)iTempDataH % 100 / 10;
	TempData.btFirst   = (u8)iTempDataH % 10;
	TempData.btDecimal = btDot;	
	
	switch(i)
	{
		case 1 : TempData1=TempData; break;			//ƥ��1
		case 2 : TempData2=TempData; break;			//ƥ��2
	}
	
	return TempData;
}
/**************************************DS18B20��غ���**************************************/


/****************************************LCD��غ���****************************************/
//�ж�æ����
void Busy()
{
	DATA = 0xff;
	RS = 0;
	RW = 1;
   	while(DATA & 0x80){
			E = 0;
   		E = 1;
   	}
   	E = 0;
}

//дָ���
void WriteCommand(u8 btCommand)
{
	Busy();
	RS = 0;
	RW = 0;
	E = 1;
	DATA = btCommand;
	E = 0;
}

//оƬ��ʼ������
void LCDInit()
{
	WriteCommand(0x0c);	//����ʾ,�޹����ʾ
	WriteCommand(0x06);	//���ֲ���������Զ�����
	WriteCommand(0x38);	//������ʾģʽ:8λ2��5x7����
}

//д���ݺ���
void WriteData(u8 btData)
{
	Busy();
	RS = 1;
	RW = 0;
	E = 1;
	DATA = btData;
	E = 0;
}

//��������
void Clear(){
	WriteCommand(1);
}

//�����ַ���ʾ����
void DisplayOne(bit bRow, u8 btColumn, u8 btData, bit bIsNumber)
{
	if (bRow) 		
		WriteCommand(0xc0 + btColumn);  //��ʾ�ڵ�1��
	else 
		WriteCommand(0x80 + btColumn);  //��ʾ�ڵ�0��

	if (bIsNumber)
		WriteData(btData + 0x30);
	else
		WriteData(btData);
}

//�ַ�����ʾ����
void DisplayString(bit bRow, u8 btColumn, u8 *pData)
{
	while (*pData != '\0')
   	{
   		if (bRow) 
				WriteCommand(0xc0 + btColumn);	//��ʾ�ڵ�1��
   		else  	  
				WriteCommand(0x80 + btColumn);	//��ʾ�ڵ�0��
		WriteData(*(pData++));			        //Ҫ��ʾ������
		btColumn++;									        //������һ
   	}
}


//�ٶ���ʾ����
void DisplayVelocity()
{
	int v10=(int)(velocity*10);
	u8 u16str[5];
	u16str[0] =  v10/10000+0x30;
	u16str[1] = (v10/1000)%10+0x30;
	u16str[2] = (v10/100)%10+0x30;
	u16str[3] = (v10/10)%10+0x30;
	u16str[4] =  v10%10+0x30;
	
	DisplayOne(1, 5, u16str[2], 0);
  DisplayOne(1, 6, u16str[3], 0);
	DisplayOne(1, 8, u16str[4], 0);
}

//������ʾ����
void DisplayPower()
{
	int k;
	int p10=(int)(power*10);
	u8 u16str[5];
	u16str[0] =  p10/10000+0x30;
	u16str[1] = (p10/1000)%10+0x30;
	u16str[2] = (p10/100)%10+0x30;
	u16str[3] = (p10/10)%10+0x30;
	u16str[4] =  p10%10+0x30;
	
	for(k=0;k<4;k++)
	{
		DisplayOne(1, 10+k, u16str[k], 0);
	} 
	DisplayOne(1, 15, u16str[4], 0);
}
/****************************************LCD��غ���****************************************/


//���ݴ����ӳ��򣨻�ȡ����ʾ�¶ȡ����١����ʣ�
float float_TempData=0;
float deltaTemp=0;

void DataProcess()
{
	m_TempData = ReadTemperature();	
  DisplayOne(1, 0, m_TempData.btSecond, 1);
	DisplayOne(1, 1, m_TempData.btFirst, 1);
	DisplayOne(1, 3, m_TempData.btDecimal, 1);

	float_TempData= ((int)(m_TempData.btSecond-'0'))*10 + (int)(m_TempData.btFirst-'0') + ((int)(m_TempData.btDecimal-'0'))/10.0;
	
	
	deltaTemp= 1.0;  //���������¶Ȳ�
	
	velocity=Hz/24.0;
	DisplayVelocity();
	
	power =C_water * RO_water * velocity * deltaTemp / 60.0 ;  // W  J/(kg*C)  g/mL  L/min  Centigrade
	DisplayPower();
}


/*****************************main*****************************/
void main()
{
	Int0Init();
	Timer0Init();
	Clear();
	LCDInit();
	DisplayString(0, 0, "T/C  L/mi P/W");
	DisplayOne(1, 2, '.', 0);
	DisplayOne(1, 7, '.', 0);
	DisplayOne(1, 14, '.', 0);
	
	while (1)
	{
		DataProcess();
	}
}
/*****************************main*****************************/