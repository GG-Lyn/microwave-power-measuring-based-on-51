#include <AT89X52.h>
#include <Intrins.h>
#include <2pt.h>


//���Ŷ���
#define		DATA	P1          //LCD1602���ݶ˿�
sbit 		  DQ =  P2^7;			  //DS18B20���ݶ˿�
sbit 		  RS =	P2^0;
sbit 		  RW =	P2^1;
sbit 		  E  =	P2^2;


//�¶����ݴ洢�ṹ
typedef struct tagTempData
{
	unsigned char 					btThird;							//��λ����					
	unsigned char 					btSecond;							//ʮλ����
	unsigned char 					btFirst;							//��λ����
	unsigned char 					btDecimal;						//С�����һλ����
	unsigned char					  btNegative;						//�Ƿ�Ϊ����		
}TEMPDATA;
TEMPDATA m_TempData;


//DS18B20���к�

const unsigned char code ROMData1[8] = {0x28, 0x70, 0x62, 0x77, 0x91, 0x0b, 0x02, 0x26};	//Uin
const unsigned char code ROMData2[8] = {0x28, 0xe8, 0x8d, 0x77, 0x91, 0x19, 0x02, 0xfe};	//Uout

/*
const unsigned char code ROMData1[8] = {0x28, 0xff, 0xdd, 0x0f, 0x54, 0x17, 0x04, 0x66};	//U1
const unsigned char code ROMData2[8] = {0x28, 0xff, 0x0b, 0x0c, 0xa2, 0x16, 0x03, 0x37};	//U2
*/
/*
const unsigned char code ROMData3[8] = {0x28, 0x31, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0xB9};	//U3
const unsigned char code ROMData4[8] = {0x28, 0x32, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0xE0};	//U4
const unsigned char code ROMData5[8] = {0x28, 0x34, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0x52};	//U5
const unsigned char code ROMData6[8] = {0x28, 0x35, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0x65};	//U6
const unsigned char code ROMData7[8] = {0x28, 0x36, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0x3C};	//U7
const unsigned char code ROMData8[8] = {0x28, 0x37, 0xC5, 0xB8, 0x00, 0x00, 0x00, 0x0B};	//U8
*/

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
void WriteByte(unsigned char btData)
{
	unsigned char i, btBuffer;
	
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
unsigned char ReadByte()
{
	unsigned char i, btDest;

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
void MatchROM(const unsigned char *pMatchData)
{
	unsigned char i;

	Initialization();
	WriteByte(MATCH_ROM);
	for (i = 0; i < 8; i++) WriteByte(*(pMatchData + i));	
}


static TEMPDATA TempData1,TempData2;

	
//��ȡ�¶�ֵ
TEMPDATA ReadTemperature()
{
	TEMPDATA TempData;
	unsigned int iTempDataH;
	unsigned char btDot, iTempDataL;
	static unsigned char i = 0;

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

	//���оƬ��ʱ����MatchROM(ROMData)����WriteByte(SKIP_ROM)
	switch (i)
	{
		case 1 : MatchROM(ROMData1); break;			//ƥ��1
		case 2 : MatchROM(ROMData2); break;			//ƥ��2
		/*
		case 3 : MatchROM(ROMData3); break;			//ƥ��3
		case 4 : MatchROM(ROMData4); break;			//ƥ��4	
		case 5 : MatchROM(ROMData5); break;			//ƥ��5
		case 6 : MatchROM(ROMData6); break;			//ƥ��6
		case 7 : MatchROM(ROMData7); break;			//ƥ��7
		case 8 : MatchROM(ROMData8); break;			//ƥ��8
		*/
	}
	//WriteByte(SKIP_ROM);							//����ROMƥ��(����оƬʱ����任�������switch)
	WriteByte(READ_MEMORY);							//������
	iTempDataL = ReadByte();
	iTempDataH = ReadByte();	
	iTempDataH <<= 8;
	iTempDataH |= iTempDataL;

	if (iTempDataH & 0x8000)
	{
		TempData.btNegative = 1;
		iTempDataH = ~iTempDataH + 1;				//������
	}

	//Ϊ��ʡȥ������������Ŀ�����������������С�����ַֿ�����ķ���(û����������)
	btDot = (unsigned char)(iTempDataH & 0x000F);	//�õ�С������
	iTempDataH >>= 4;							//�õ���������
	btDot *= 5; 									//btDot*10/16�õ�ת�����С������
	btDot >>= 3;

	//���ݴ���
	TempData.btThird   = (unsigned char)iTempDataH / 100;
	TempData.btSecond  = (unsigned char)iTempDataH % 100 / 10;
	TempData.btFirst   = (unsigned char)iTempDataH % 10;
	TempData.btDecimal = btDot;	
	
	switch(i)
	{
		case 1 : TempData1=TempData; break;			//ƥ��1
		case 2 : TempData2=TempData; break;			//ƥ��2
	}
	
	//����������ط����¶ȴ��빫ʽ���������Ϊ����ֵ
	
	
	
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
void WriteCommand(unsigned char btCommand)
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
void WriteData(unsigned char btData)
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
void DisplayOne(bit bRow, unsigned char btColumn, unsigned char btData, bit bIsNumber)
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
void DisplayString(bit bRow, unsigned char btColumn, unsigned char *pData)
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

////�޷������ͣ�u16����ʾ����  Ҳ�ɴ���int��
//void Display_u16(bit bRow, u8 btColumn, u16 x)
//{
//	int k;
//	u8 u16str[5];
//	u16str[0] =  x/10000+0x30;
//	u16str[1] = (x/1000)%10+0x30;
//	u16str[2] = (x/100)%10+0x30;
//	u16str[3] = (x/10)%10+0x30;
//	u16str[4] =  x%10+0x30;
//	
//	for(k=0;k<5;k++)
//	{
//		DisplayOne(bRow, btColumn+k, u16str[k], 0);
//	} 
//}
/****************************************LCD��غ���****************************************/




//���ݴ����ӳ��򣨻�ȡ�¶Ȳ�ͨ��LCD��ʾ��
void DataProcess()
{
	m_TempData = ReadTemperature();
	
	if (m_TempData.btNegative)
		DisplayOne(1, 6, '-', 0);
	else 
		DisplayOne(1, 6, m_TempData.btThird, 1);

	DisplayOne(1, 7, m_TempData.btSecond, 1);
	DisplayOne(1, 8, m_TempData.btFirst, 1);
	DisplayOne(1, 10, m_TempData.btDecimal, 1);
}



/*****************************main*****************************/
void main()
{
	Clear();
	LCDInit();
	DisplayString(0, 0, "  Temperature");
	DisplayOne(1, 9, '.', 0);
	
	while (1)
	{
		DataProcess();
	}
}
/*****************************main*****************************/