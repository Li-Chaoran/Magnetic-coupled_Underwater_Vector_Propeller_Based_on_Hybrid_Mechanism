#ifndef __BSP__UART_H
#define __BSP__UART_H

#include <stdarg.h>                               // 用于支持不定长参数
#include "stdio.h"                                // 用于支持函数printf、spritnf等
#include "string.h"                               // 用于支持字符串处理函数strset、strcpy、memset、metcpy等


#ifdef USE_STDPERIPH_DRIVER
    #include "stm32f4xx.h"                        // 标准库
#endif

#ifdef USE_HAL_DRIVER
    #include "stm32f4xx_hal.h"                    // HAL库
#endif



/*****************************************************************************
 ** 移植配置修改区
 ** 备注：除非有特殊要求，否则，下面参数已通用：RS485、蓝牙模块、ESP8266模块、串口屏等
****************************************************************************/
// 串口开关
#define UART1_EN                       1          // 串口1，0=关、1=启用;  倘若没用到UART1, 置0，就不会开辟UART1发送缓存、接收缓存，省一点资源;
#define UART2_EN                       1          // 串口2，0=关、1=启用;  同上;
#define UART3_EN                       1          // 串口3，0=关、1=启用;  同上;
#define UART4_EN                       1          // 串口4，0=关、1=启用;  同上;
#define UART5_EN                       1          // 串口5，0=关、1=启用;  同上;
#define UART6_EN                       1          // 串口5，0=关、1=启用;  同上;
// 发送缓冲区大小
#define UART1_TX_BUF_SIZE           2048          // 配置每组UART发送环形缓冲区数组的大小，单位：字节数; 
#define UART2_TX_BUF_SIZE            512          // -- 只有在前面串口开关在打开状态，才会定义具体的缓冲区数组
#define UART3_TX_BUF_SIZE            512          // -- 默认值：512，此值已能适配大部场景的通信; 如果与ESP8266之类的设备通信，可适当增大此值。
#define UART4_TX_BUF_SIZE            512          // -- 值范围：1~65535; 注意初始化后，不要超过芯片最大RAM值。
#define UART5_TX_BUF_SIZE            512          // -- 注意此值是一个环形缓冲区大小，决定每一帧或多帧数据进入发送前的总缓存字节数，先进先出。
#define UART6_TX_BUF_SIZE            512          //
// 接收缓冲区大小
#define UART1_RX_BUF_SIZE            512          // 配置每组UART接收缓冲区的大小，单位：字节; 此值影响范围：中断里的接收缓存大小，接收后数据缓存的大小
#define UART2_RX_BUF_SIZE            512          // --- 当接收到的一帧数据字节数，小于此值时，数据正常；
#define UART3_RX_BUF_SIZE            512          // --- 当接收到的一帧数据字节数，超过此值时，超出部分，将在中断中直接弃舍，直到此帧接收结束(发生空闲中断); 
#define UART4_RX_BUF_SIZE            512          // 
#define UART5_RX_BUF_SIZE            512          // 
#define UART6_RX_BUF_SIZE            512          // 
// 结束-配置修改



/*****************************************************************************
 ** 声明全局函数
 **
 ** 每个串口的函数:
 ** 初始化    1个   波特率-None-8-1
 ** 发送      3个   发送指定长度数据、字符串、AT指令 
 ** 接收      3个   获取字节数、获取数据、清0
****************************************************************************/
// UART1
void      UART1_Init(uint32_t ulBaudrate);                                      // 初始化串口1; GPIO引脚PA9+PA10、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART1_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART1_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART1_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART1_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART1_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART1_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
// UART2                                                                        
void      UART2_Init(uint32_t ulBaudrate);                                      // 初始化串口2; GPIO引脚PA2+PA3、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART2_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART2_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART2_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART2_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART2_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART2_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
// UART3                                                                        
void      UART3_Init(uint32_t ulBaudrate);                                      // 初始化串口3; GPIO引脚PB10+PB11、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART3_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART3_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART3_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART3_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART3_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART3_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
// UART4                                                            
void      UART4_Init(uint32_t ulBaudrate);                                      // 初始化串口4; GPIO引脚PC10+PC11、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART4_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART4_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART4_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART4_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART4_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART4_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
// UART5                                                                        
void      UART5_Init(uint32_t ulBaudrate);                                      // 初始化串口5; GPIO引脚PC12+PD2、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART5_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART5_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART5_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART5_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART5_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART5_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
// UART6                                                                        
void      UART6_Init(uint32_t ulBaudrate);                                      // 初始化串口6; GPIO引脚PC6+PC7、中断优先级、通信参数：波特率可设、8位数据、无校验、1个停止位
void      UART6_SendData(uint8_t *puData, uint16_t usNum);                      // 发送指定数据; 参数：数据地址、字节数
void      UART6_SendString(const char *pcString, ...);                          // 发送字符串;   参数：字符串地址; 使用方法如同printf
uint8_t   UART6_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // 本函数，针对ESP8266、蓝牙模块等AT固件，用于等待返回期待的信息; 参数：AT指令字符串、期待返回信息字符串、ms等待超时; 返回：0-执行失败、1-执行成功 
uint16_t  UART6_GetRxNum(void);                                                 // 获取接收到的最新一帧字节数
uint8_t * UART6_GetRxData(void);                                                // 获取接收到的数据 (缓存的地址)
void      UART6_ClearRx(void);                                                  // 清理接收到的数据 (清理最后一帧字节数，因为它是判断接收的标志)
                                                                                
// 调试辅助                                                                     
void      showData(uint8_t *puRxData, uint16_t usRxNum);                        // 经printf，发送到串口助手上，方便观察


#endif

