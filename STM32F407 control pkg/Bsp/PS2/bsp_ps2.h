#ifndef __BSP_PS2_H
#define __BSP_PS2_H

#include <stdint.h>

//����ps2 V1 �ֱ��豸��ʶ��
#define Wired_PS2_V1_VID 0x0810
#define Wired_PS2_V1_PID 0x0001

//����ps2 V2 �ֱ��豸��ʶ��
#define Wired_PS2_V2_VID 0x0079
#define Wired_PS2_V2_PID 0x0006

//����ps2�ֱ���׿ģʽ�豸��ʶ��
#define Wireless_Android_PS2_VID 0x045E
#define Wireless_Android_PS2_PID 0x028E

//����ps2�ֱ�PCģʽ�豸��ʶ��
#define Wireless_PC_PS2_VID 0x0079
#define Wireless_PC_PS2_PID 0x0126

//PS2�ֱ����Ͷ���
typedef enum{
	UnKnown_Dev,          //δ֪�豸
	Wired_V1_PS2,            //����ps2�ֱ� V1
	Wired_V2_PS2,            //����ps2�ֱ� V2
	Wiredless_PC_PS2,     //����ps2�ֱ�pcģʽ
	Wiredless_Android_PS2,//����ps2�ֱ���׿ģʽ
}PS2_TYPE_t;

//��ʾ������״ֵ̬
#define PS2KEY_PressDOWN 1  //�����ɿ�
#define PS2KEY_PressUP   0  //��������

//PS2����״̬(��״̬��������˫��������)
typedef enum{
    PS2KEYSTATE_NONE,
    PS2KEYSTATE_SINGLECLICK,
    PS2KEYSTATE_DOUBLECLICK,
    PS2KEYSTATE_LONGCLICK
}PS2KEY_State_t;

//PS2�ֱ�����
typedef struct{
	uint8_t LX;      //4������ҡ��ֵ,ȡֵ0~255
	uint8_t LY;
	uint8_t RX;
	uint8_t RY;
	PS2KEY_State_t (*getKeyEvent)(uint8_t keybit); //��ȡ�����¼�,�е�����˫��������3���¼����Ի�ȡ,��ڲ���Ϊ������ö�ټ�ֵ
	uint8_t (*getKeyState)(uint8_t keybit);        //��ȡ������״̬,0��ʾ����,1��ʾ�ɿ�,��ڲ���Ϊ������ö�ټ�ֵ
}PS2INFO_t;

//PS2����λ��ö��(bit0~bit15�ֱ�Ϊ�����0~15)
enum 
{
	PS2KEY_SELECT	   = 0, //ѡ�񰴼�
	PS2KEY_LROCKER      , //����ҡ�˰��¼�ֵ
	PS2KEY_RROCKER      ,
	PS2KEY_START        , //��ʼ����
	PS2KEY_UP           , //�󰴼�����
	PS2KEY_RIGHT        ,
	PS2KEY_DOWN         ,
	PS2KEY_LEFT         ,
	PS2KEY_L2           ,	//���Ұ������ֵ
	PS2KEY_R2           ,
	PS2KEY_L1           ,  
	PS2KEY_R1           ,
	PS2KEY_1GREEN       , //�Ұ�������
	PS2KEY_2RED         , 
	PS2KEY_3BLUE        , 
	PS2KEY_4PINK           
};


#endif

