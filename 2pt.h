#define		DATA	P1          //LCD1602���ݶ˿�

/************************************DS18B20 ROM��������************************************/
#define             SEARCH_ROM              0xF0                  //����ROM
#define             READ_ROM               	0x33                  //��ȡROM 
#define             MATCH_ROM               0x55                  //ƥ��ROM
#define             SKIP_ROM               	0xCC                  //����ROM 
#define             ALARM_SEARCH            0xEC                  //�澯����
/************************************DS18B20 ROM��������************************************/

/**************************************�洢����������**************************************/
#define             READ_POWER              0xB4                  //��18B20���緽ʽ
#define             TEMP_SWITCH             0x44                  //�����¶ȱ任 
#define             READ_MEMORY             0xBE                  //���ݴ�洢��
#define             WRITE_MEMORY            0x4E                  //д�ݴ�洢��
#define             COPY_MEMORY             0x48                  //�����ݴ�洢��
#define             ANEW_MOVE             	0xB8                  //���µ���E^2PROM�е�����
/**************************************�洢����������***************************************/


/****************һϵ����ʱ����****************/
void Delay16us()
{
  unsigned char a;
	for (a = 0; a < 4; a++);
}

void Delay60us()
{
	unsigned char a;
	for (a = 0; a < 18; a++);
}

void Delay480us()
{
	unsigned char a;
	for (a = 0; a < 158; a++);
}

void Delay240us()
{
	unsigned char a;
	for (a = 0; a < 78; a++);
}

void Delay500ms()
{
	unsigned char a, b, c;
	for (a = 0; a < 250; a++)
	for (b = 0; b < 3; b++)
	for (c = 0; c < 220; c++);
}
/****************һϵ����ʱ����****************/