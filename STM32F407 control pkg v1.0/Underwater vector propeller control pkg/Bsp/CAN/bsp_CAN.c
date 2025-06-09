#include "bsp_CAN.h"
#include "stdio.h"
#include "string.h"



xCAN_InfoDef  xCAN1 = {0};      // 定义一个数据结构体，用于管理收、发的数据和信息



/******************************************************************************
 * 函  数： CAN1_SendData
 * 功  能： CAN发送数据函数
 * 参  数： uint8_t* msgData   需发送数据的地址
 *          uint8_t  len       发送的字节数; 最大值：8
 * 返回值： 发送状态; 成功-0、错误-1、忙错误-2、超时-3   
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len)
uint8_t CAN1_SendData(uint8_t *msgData, uint8_t len)
 ******************************************************************************/
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len)
{ 
    // 定义两个变量
    static uint32_t TxMailbox = 0;                                                 // 用于记录发送成功时所用的邮箱编号：0~2; 被发送函数HAL_CAN_AddTxMessage()赋值
    static uint32_t txStatus  = 0;                                                 // 用于记录发送状态; 成功-0、错误-1、忙错误-2、超时-3; 同上，被发送函数HAL_CAN_AddTxMessage()赋值
    // 限制字节数
    if(len>8)                                                                      // 判断字节是否超过8字节
        len=8;                                                                     // 如果超过8字节，只发送前8个字节
    // 配置帧信息
    xCAN1.TxHeader.StdId = id;                                                     // 帧ID, 在bsp_CAN.h中定义
    xCAN1.TxHeader.ExtId = 0;                                                      // 帧ID，拓展帧为0
    xCAN1.TxHeader.IDE   = CAN_ID_STD;                                             // 帧格式; 标准帧: CAN_ID_STD，注意修改上行为StdID; 扩展帧: CAN_ID_EXT, 注意修改上行为ExtID
    xCAN1.TxHeader.RTR   = CAN_RTR_DATA;                                           // 数据帧; 
    xCAN1.TxHeader.DLC   = len;                                                    // 数据字节数，
    xCAN1.TxHeader.TransmitGlobalTime = DISABLE;                                   // 使能时间戳添加到最后两个字节：Data[6]、Data[7]
    // 等待发送邮箱空闲
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)                           // 每次发送前，要等待至有发送邮箱空闲。共有3个发送邮箱。如果返回值为0，即没有发送邮箱空闲，继续等
    {                                                                              // 注意：如果发送超时失败，总线一直在忙的可能性极低，更大的可能是，CAN没有连接到收发器
        static uint8_t times = 0;
        HAL_Delay(1);
        if (times++ >= 5)
        {
            times = 0;
            printf("\r发送失败：等待超时！请检查CAN1的线路是否已连接到收发器\r");
            return 3;
        }
    }
    // 发送
    txStatus = HAL_CAN_AddTxMessage(hcan, &xCAN1.TxHeader, data, &TxMailbox);      // 发送   
    // 返回发送状态
    return txStatus;                                                               // 返回发送状态; 成功-0、错误-1、忙错误-2、超时-3   
}



/************************************************************************
* 函  数： bsp_can_init
* 功  能： CAN1使能
* 参  数： 无
* 返回值： 无
************************************************************************/
void bsp_can_init(void)
{ 
	CAN1_FilterInit();
	HAL_CAN_Start(&hcan1);   
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);	
}



/******************************************************************************
 * 函  数： CAN1_FilterInit
 * 功  能： CAN1筛选器初始化
 * 备  注： 为方便调试，本函数配置为：接收所有数据帧，包括标准帧、扩展帧
 * 参  数： 无
 * 返回值： 无
 ******************************************************************************/
void CAN1_FilterInit(void)
{
    CAN_FilterTypeDef sFilterConfig;

    // 筛选器配置
    sFilterConfig.FilterBank = 0;                                                                            // 筛选器组0 
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;                                                        // 屏蔽位模式
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;                                                       // 32位
    sFilterConfig.FilterIdHigh         = (((uint32_t)CAN_RX_ID << 3) & 0xFFFF0000) >> 16;                    // 要筛选的ID高位
    sFilterConfig.FilterIdLow          = (((uint32_t)CAN_RX_ID << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;  // 要筛选的ID低位
    sFilterConfig.FilterMaskIdHigh     = 0x0000;                                                             // 筛选器高16位的位匹配，位0-此位都通过、位1-此位需要与ID位相同
    sFilterConfig.FilterMaskIdLow      = 0x0000;                                                             // 筛选器低16位的位匹配，位0-此位都通过、位1-此位需要与ID位相同
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;                                                       // 筛选器被关联到FIFO 0 
    sFilterConfig.FilterActivation = ENABLE;                                                                 // 使能筛选器 
    sFilterConfig.SlaveStartFilterBank = 14;                                                                 // CAN1、CAN2共用28个筛选器(0~27)，此值设置CAN2的筛选器从哪个编号开始; 两个特别的值：当值=0时，不会为CAN1分配任何筛选器，当值=28时，CAN可以使用CAN1的所有筛选器。

    // 初始化筛选器
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)                                              // 初始化筛选器，并检查返回值：成功-0、错误-1、忙错误-2、超时-3    
    {
        Error_Handler();                                                                                     // 错误处理
    }
    
    // 开启CAN接收
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)                         // 开启CAN接收，并检查返回值：成功-0、错误-1、忙错误-2、超时-3  
    {
        Error_Handler();                                                                                     // 错误处理
    }
    
    // 开启CAN
    if (HAL_CAN_Start(&hcan1) != HAL_OK)                                                                     // 开启CAN，并检查返回值：成功-0、错误-1、忙错误-2、超时-3  
    {       
        Error_Handler();                                                                                     // 错误处理
    }
}


/******************************************************************************
 * 函  数： HAL_CAN_RxFifo0MsgPendingCallback
 * 功  能： CAN接收中断的回调函数
 *          注意：只有在筛选器中，配置接收规则使用FiFO0时，接收到新一帧数据时才会触发此回调函数
 *          如果使用的是FIFO1, 就会触发HAL_CAN_RxFifo1MsgPendingCallback()
 * 参  数： CAN_HandleTypeDef  *CanNum  
 * 返回值： 无
 ******************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(hcan_t *hcan)
{    
	if(hcan == &hcan1) {
		can1_rx_callback();
	}                                          
}



/************************************************************************
* 函  数:   canx_bsp_receive(CAN_HandleTypeDef *hcan, uint8_t *buf)
* 功  能:   接收数据,更新xhan结构体内容（内容和长度）
* 参  数:   hcan: CAN句柄
* 返回值:   rec_id:接收到数据的CAN设备ID
            接收的数据长度
************************************************************************/
uint8_t canx_receive(hcan_t *hcan, uint16_t *rec_id)
{	
    memset(xCAN1.RxData, 0, 9);                                                             // 清0; 存放接收有效数据的数组; CAN一帧数据最大有效负载8字节，数组中开辟9个字节，是为了适配以字符串输出调试信息，最后的1字节0='\0'，是字符串结束符; 
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &xCAN1.RxHeader, xCAN1.RxData) == HAL_OK)  // 把收到的数据存放到结构体备用，帧信息：RxHeader，数据：RxData
    {
      *rec_id = xCAN1.RxHeader.StdId;
      xCAN1.RxNum = xCAN1.RxHeader.DLC;                                                     // 接收字节数、标志位; 外部判断此值接收字节数是否大于0，以判断是否接收到新一帧数据
      return xCAN1.RxHeader.DLC;                                                            // 返回接收数据长度
    }
    else
      return 0;
}




/************************************************************************
* @brief:      	can1_rx_callback(void)
* @param:       void
* @retval:     	void
* @details:    	供用户调用的接收弱函数
************************************************************************/
__weak void can1_rx_callback(void)
{

}

/************************************************************************
* @brief:      	can2_rx_callback(void)
* @param:       void
* @retval:     	void
* @details:    	供用户调用的接收弱函数
************************************************************************/
__weak void can2_rx_callback(void)
{

}

// 文件结尾，需要保留至少1空行
