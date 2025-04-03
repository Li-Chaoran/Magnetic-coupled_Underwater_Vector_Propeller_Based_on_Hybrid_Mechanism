#include "bsp_CAN.h"
#include "stdio.h"
#include "string.h"



xCAN_InfoDef  xCAN1 = {0};      // ����һ�����ݽṹ�壬���ڹ����ա��������ݺ���Ϣ



/******************************************************************************
 * ��  ���� CAN1_SendData
 * ��  �ܣ� CAN�������ݺ���
 * ��  ���� uint8_t* msgData   �跢�����ݵĵ�ַ
 *          uint8_t  len       ���͵��ֽ���; ���ֵ��8
 * ����ֵ�� ����״̬; �ɹ�-0������-1��æ����-2����ʱ-3   
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len)
uint8_t CAN1_SendData(uint8_t *msgData, uint8_t len)
 ******************************************************************************/
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len)
{ 
    // ������������
    static uint32_t TxMailbox = 0;                                                 // ���ڼ�¼���ͳɹ�ʱ���õ������ţ�0~2; �����ͺ���HAL_CAN_AddTxMessage()��ֵ
    static uint32_t txStatus  = 0;                                                 // ���ڼ�¼����״̬; �ɹ�-0������-1��æ����-2����ʱ-3; ͬ�ϣ������ͺ���HAL_CAN_AddTxMessage()��ֵ
    // �����ֽ���
    if(len>8)                                                                      // �ж��ֽ��Ƿ񳬹�8�ֽ�
        len=8;                                                                     // �������8�ֽڣ�ֻ����ǰ8���ֽ�
    // ����֡��Ϣ
    xCAN1.TxHeader.StdId = id;                                                     // ֡ID, ��bsp_CAN.h�ж���
    xCAN1.TxHeader.ExtId = 0;                                                      // ֡ID����չ֡Ϊ0
    xCAN1.TxHeader.IDE   = CAN_ID_STD;                                             // ֡��ʽ; ��׼֡: CAN_ID_STD��ע���޸�����ΪStdID; ��չ֡: CAN_ID_EXT, ע���޸�����ΪExtID
    xCAN1.TxHeader.RTR   = CAN_RTR_DATA;                                           // ����֡; 
    xCAN1.TxHeader.DLC   = len;                                                    // �����ֽ�����
    xCAN1.TxHeader.TransmitGlobalTime = DISABLE;                                   // ʹ��ʱ�����ӵ���������ֽڣ�Data[6]��Data[7]
    // �ȴ������������
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)                           // ÿ�η���ǰ��Ҫ�ȴ����з���������С�����3���������䡣�������ֵΪ0����û�з���������У�������
    {                                                                              // ע�⣺������ͳ�ʱʧ�ܣ�����һֱ��æ�Ŀ����Լ��ͣ�����Ŀ����ǣ�CANû�����ӵ��շ���
        static uint8_t times = 0;
        HAL_Delay(1);
        if (times++ >= 5)
        {
            times = 0;
            printf("\r����ʧ�ܣ��ȴ���ʱ������CAN1����·�Ƿ������ӵ��շ���\r");
            return 3;
        }
    }
    // ����
    txStatus = HAL_CAN_AddTxMessage(hcan, &xCAN1.TxHeader, data, &TxMailbox);      // ����   
    // ���ط���״̬
    return txStatus;                                                               // ���ط���״̬; �ɹ�-0������-1��æ����-2����ʱ-3   
}



/************************************************************************
* ��  ���� bsp_can_init
* ��  �ܣ� CAN1ʹ��
* ��  ���� ��
* ����ֵ�� ��
************************************************************************/
void bsp_can_init(void)
{ 
	CAN1_FilterInit();
	HAL_CAN_Start(&hcan1);   
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);	
}



/******************************************************************************
 * ��  ���� CAN1_FilterInit
 * ��  �ܣ� CAN1ɸѡ����ʼ��
 * ��  ע�� Ϊ������ԣ�����������Ϊ��������������֡��������׼֡����չ֡
 * ��  ���� ��
 * ����ֵ�� ��
 ******************************************************************************/
void CAN1_FilterInit(void)
{
    CAN_FilterTypeDef sFilterConfig;

    // ɸѡ������
    sFilterConfig.FilterBank = 0;                                                                            // ɸѡ����0 
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;                                                        // ����λģʽ
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;                                                       // 32λ
    sFilterConfig.FilterIdHigh         = (((uint32_t)CAN_RX_ID << 3) & 0xFFFF0000) >> 16;                    // Ҫɸѡ��ID��λ
    sFilterConfig.FilterIdLow          = (((uint32_t)CAN_RX_ID << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;  // Ҫɸѡ��ID��λ
    sFilterConfig.FilterMaskIdHigh     = 0x0000;                                                             // ɸѡ����16λ��λƥ�䣬λ0-��λ��ͨ����λ1-��λ��Ҫ��IDλ��ͬ
    sFilterConfig.FilterMaskIdLow      = 0x0000;                                                             // ɸѡ����16λ��λƥ�䣬λ0-��λ��ͨ����λ1-��λ��Ҫ��IDλ��ͬ
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;                                                       // ɸѡ����������FIFO 0 
    sFilterConfig.FilterActivation = ENABLE;                                                                 // ʹ��ɸѡ�� 
    sFilterConfig.SlaveStartFilterBank = 14;                                                                 // CAN1��CAN2����28��ɸѡ��(0~27)����ֵ����CAN2��ɸѡ�����ĸ���ſ�ʼ; �����ر��ֵ����ֵ=0ʱ������ΪCAN1�����κ�ɸѡ������ֵ=28ʱ��CAN����ʹ��CAN1������ɸѡ����

    // ��ʼ��ɸѡ��
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)                                              // ��ʼ��ɸѡ��������鷵��ֵ���ɹ�-0������-1��æ����-2����ʱ-3    
    {
        Error_Handler();                                                                                     // ������
    }
    
    // ����CAN����
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)                         // ����CAN���գ�����鷵��ֵ���ɹ�-0������-1��æ����-2����ʱ-3  
    {
        Error_Handler();                                                                                     // ������
    }
    
    // ����CAN
    if (HAL_CAN_Start(&hcan1) != HAL_OK)                                                                     // ����CAN������鷵��ֵ���ɹ�-0������-1��æ����-2����ʱ-3  
    {       
        Error_Handler();                                                                                     // ������
    }
}


/******************************************************************************
 * ��  ���� HAL_CAN_RxFifo0MsgPendingCallback
 * ��  �ܣ� CAN�����жϵĻص�����
 *          ע�⣺ֻ����ɸѡ���У����ý��չ���ʹ��FiFO0ʱ�����յ���һ֡����ʱ�Żᴥ���˻ص�����
 *          ���ʹ�õ���FIFO1, �ͻᴥ��HAL_CAN_RxFifo1MsgPendingCallback()
 * ��  ���� CAN_HandleTypeDef  *CanNum  
 * ����ֵ�� ��
 ******************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(hcan_t *hcan)
{    
	if(hcan == &hcan1) {
		can1_rx_callback();
	}                                          
}



/************************************************************************
* ��  ��:   canx_bsp_receive(CAN_HandleTypeDef *hcan, uint8_t *buf)
* ��  ��:   ��������,����xhan�ṹ�����ݣ����ݺͳ��ȣ�
* ��  ��:   hcan: CAN���
* ����ֵ:   rec_id:���յ����ݵ�CAN�豸ID
            ���յ����ݳ���
************************************************************************/
uint8_t canx_receive(hcan_t *hcan, uint16_t *rec_id)
{	
    memset(xCAN1.RxData, 0, 9);                                                             // ��0; ��Ž�����Ч���ݵ�����; CANһ֡���������Ч����8�ֽڣ������п���9���ֽڣ���Ϊ���������ַ������������Ϣ������1�ֽ�0='\0'�����ַ���������; 
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &xCAN1.RxHeader, xCAN1.RxData) == HAL_OK)  // ���յ������ݴ�ŵ��ṹ�屸�ã�֡��Ϣ��RxHeader�����ݣ�RxData
    {
      *rec_id = xCAN1.RxHeader.StdId;
      xCAN1.RxNum = xCAN1.RxHeader.DLC;                                                     // �����ֽ�������־λ; �ⲿ�жϴ�ֵ�����ֽ����Ƿ����0�����ж��Ƿ���յ���һ֡����
      return xCAN1.RxHeader.DLC;                                                            // ���ؽ������ݳ���
    }
    else
      return 0;
}




/************************************************************************
* @brief:      	can1_rx_callback(void)
* @param:       void
* @retval:     	void
* @details:    	���û����õĽ���������
************************************************************************/
__weak void can1_rx_callback(void)
{

}

/************************************************************************
* @brief:      	can2_rx_callback(void)
* @param:       void
* @retval:     	void
* @details:    	���û����õĽ���������
************************************************************************/
__weak void can2_rx_callback(void)
{

}

// �ļ���β����Ҫ��������1����
