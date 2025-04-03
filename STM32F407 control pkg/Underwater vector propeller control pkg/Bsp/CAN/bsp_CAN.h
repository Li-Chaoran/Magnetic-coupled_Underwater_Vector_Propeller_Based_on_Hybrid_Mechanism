#ifndef __BSP__CAN_H
#define __BSP__CAN_H

#include "can.h"                     // 特别地，本文件是CubeMX生成的CAN.h文件; 直接引用此文件，可以减少好些头文件的手动引用
// #include "stm32f4xx_hal.h"
// #include "stm32f4xx_hal_can.h"
// #include "main.h"
// extern CAN_HandleTypeDef hcan1;   // 特别地，本结构体，在CubeMX生成的can.c定义，在can.h中声明为全局变量; 如果已#include "can.h", 就无需再使用此行



/*****************************************************************************
 ** 移植配置修改区
****************************************************************************/
#define CAN_RX_ID 0x123              // 接收的标识符，在CAN1_FilterInit()中使用，无法用三言两语解释，自行上csdn.net刨CAN筛选器
#define CAN_TX_ID 0x666              // 发送的报文ID，在CAN1_SendData()中使用;



/*****************************************************************************
 ** 全局变量
****************************************************************************/
typedef struct
{
    // 发送                           // 当需要发送新一帧数据时，在自定义发送函数CAN1_SendData()中，修改下面几个变量，再调用HAL库函数 HAL_CAN_AddTxMessage( ), CAN1就会自动发送数据
    uint8_t  TxFlag;                  // 发送状态标志，用于记录 CAN1_SendData() 状态返回值
    CAN_TxHeaderTypeDef    TxHeader;  // HAL库的CAN发送结构体，其成员如下：
    // .StdId                         标准帧的ID, 11位, 值范围：0x00~0x7FF
    // .ExtId                         扩展帧的ID, 29位, 值范围：0x00~0x1FFFFFF
    // .DLC                           接收到的字节数, 单位：byte, 值范围：0~8 
    // .IDE                           帧格式; 0_标准帧、4_扩展帧
    // .RTR                           帧类型; 0_数据帧、2_遥控帧
    // .TransmitGlobalTime            使能时间戳，添加到Data[6]和Data[7] 
    
    // 接收                           // 当接收到新一帧数据时，下面变量，将在中断回调函数中被赋值; 在外部判断 xCAN1.RxNum > 0后，即可使用
    uint8_t  RxNum;                   // 接收到的字节数，也作为接收标志; xCAN1.RxHeader.DLC也是有效字节数，但为了直观方便，在中断回调函数中，已把其值复制到此xCAN1.RxNum变量; 
    uint8_t  RxData[9];               // 接收到的数据; CAN一帧数据最大有效负载8字节，数组中开辟9个字节，是为了适配以字符串输出调试信息，最后的1字节0='\0'，是字符串结束符;    
    CAN_RxHeaderTypeDef RxHeader;     // HAL库的CAN接收结构体，其成员如下：
    // .StdId                         标准帧的ID, 11位, 值范围：0x00~0x7FF
    // .ExtId                         扩展帧的ID, 29位, 值范围：0x00~0x1FFFFFF
    // .DLC                           接收到的字节数, 单位：byte, 值范围：0~8 
    // .FilterMatchIndex              筛选器编号, 值范围：0x00~0xFF
    // .IDE                           帧格式; 0_标准帧、4_扩展帧
    // .RTR                           帧类型; 0_数据帧、2_遥控帧
    // .Timestamp                     使用时间触发模式时，时间戳，值范围：0x00~0xFFFF       
} xCAN_InfoDef;
extern xCAN_InfoDef  xCAN1;           // 在bsp_CAN.c中定义，再在本h文件中，用extern声明为全局变量。用于方便管理CAN1的收发信息、变量数据等
typedef CAN_HandleTypeDef hcan_t;




/*****************************************************************************
 ** 声明全局函数
****************************************************************************/
void    CAN1_FilterInit(void);                                                              // 筛选器配置函数; 
uint8_t canx_send_data(CAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len);  // 数据发送函数;
void bsp_can_init(void);                                                                    // CAN1使能;
uint8_t canx_receive(hcan_t *hcan, uint16_t *rec_id);                                       // 接收数据,更新xhan结构体内容（内容和长度）
void can1_rx_callback(void);                                                                // 供用户调用的接收弱函数
void can2_rx_callback(void);                                                                // 供用户调用的接收弱函数

#endif


// 文件结尾，需要保留至少1空行
