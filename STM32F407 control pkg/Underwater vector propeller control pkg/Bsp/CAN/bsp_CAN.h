#ifndef __BSP__CAN_H
#define __BSP__CAN_H

#include "can.h"                     // �ر�أ����ļ���CubeMX���ɵ�CAN.h�ļ�; ֱ�����ô��ļ������Լ��ٺ�Щͷ�ļ����ֶ�����
// #include "stm32f4xx_hal.h"
// #include "stm32f4xx_hal_can.h"
// #include "main.h"
// extern CAN_HandleTypeDef hcan1;   // �ر�أ����ṹ�壬��CubeMX���ɵ�can.c���壬��can.h������Ϊȫ�ֱ���; �����#include "can.h", ��������ʹ�ô���



/*****************************************************************************
 ** ��ֲ�����޸���
****************************************************************************/
#define CAN_RX_ID 0x123              // ���յı�ʶ������CAN1_FilterInit()��ʹ�ã��޷�������������ͣ�������csdn.net��CANɸѡ��
#define CAN_TX_ID 0x666              // ���͵ı���ID����CAN1_SendData()��ʹ��;



/*****************************************************************************
 ** ȫ�ֱ���
****************************************************************************/
typedef struct
{
    // ����                           // ����Ҫ������һ֡����ʱ�����Զ��巢�ͺ���CAN1_SendData()�У��޸����漸���������ٵ���HAL�⺯�� HAL_CAN_AddTxMessage( ), CAN1�ͻ��Զ���������
    uint8_t  TxFlag;                  // ����״̬��־�����ڼ�¼ CAN1_SendData() ״̬����ֵ
    CAN_TxHeaderTypeDef    TxHeader;  // HAL���CAN���ͽṹ�壬���Ա���£�
    // .StdId                         ��׼֡��ID, 11λ, ֵ��Χ��0x00~0x7FF
    // .ExtId                         ��չ֡��ID, 29λ, ֵ��Χ��0x00~0x1FFFFFF
    // .DLC                           ���յ����ֽ���, ��λ��byte, ֵ��Χ��0~8 
    // .IDE                           ֡��ʽ; 0_��׼֡��4_��չ֡
    // .RTR                           ֡����; 0_����֡��2_ң��֡
    // .TransmitGlobalTime            ʹ��ʱ�������ӵ�Data[6]��Data[7] 
    
    // ����                           // �����յ���һ֡����ʱ����������������жϻص������б���ֵ; ���ⲿ�ж� xCAN1.RxNum > 0�󣬼���ʹ��
    uint8_t  RxNum;                   // ���յ����ֽ�����Ҳ��Ϊ���ձ�־; xCAN1.RxHeader.DLCҲ����Ч�ֽ�������Ϊ��ֱ�۷��㣬���жϻص������У��Ѱ���ֵ���Ƶ���xCAN1.RxNum����; 
    uint8_t  RxData[9];               // ���յ�������; CANһ֡���������Ч����8�ֽڣ������п���9���ֽڣ���Ϊ���������ַ������������Ϣ������1�ֽ�0='\0'�����ַ���������;    
    CAN_RxHeaderTypeDef RxHeader;     // HAL���CAN���սṹ�壬���Ա���£�
    // .StdId                         ��׼֡��ID, 11λ, ֵ��Χ��0x00~0x7FF
    // .ExtId                         ��չ֡��ID, 29λ, ֵ��Χ��0x00~0x1FFFFFF
    // .DLC                           ���յ����ֽ���, ��λ��byte, ֵ��Χ��0~8 
    // .FilterMatchIndex              ɸѡ�����, ֵ��Χ��0x00~0xFF
    // .IDE                           ֡��ʽ; 0_��׼֡��4_��չ֡
    // .RTR                           ֡����; 0_����֡��2_ң��֡
    // .Timestamp                     ʹ��ʱ�䴥��ģʽʱ��ʱ�����ֵ��Χ��0x00~0xFFFF       
} xCAN_InfoDef;
extern xCAN_InfoDef  xCAN1;           // ��bsp_CAN.c�ж��壬���ڱ�h�ļ��У���extern����Ϊȫ�ֱ��������ڷ������CAN1���շ���Ϣ���������ݵ�
typedef CAN_HandleTypeDef hcan_t;




/*****************************************************************************
 ** ����ȫ�ֺ���
****************************************************************************/
void    CAN1_FilterInit(void);                                                              // ɸѡ�����ú���; 
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len);  // ���ݷ��ͺ���;
void bsp_can_init(void);                                                                    // CAN1ʹ��;
uint8_t canx_receive(hcan_t *hcan, uint16_t *rec_id);                                       // ��������,����xhan�ṹ�����ݣ����ݺͳ��ȣ�
void can1_rx_callback(void);                                                                // ���û����õĽ���������
void can2_rx_callback(void);                                                                // ���û����õĽ���������

#endif


// �ļ���β����Ҫ��������1����
