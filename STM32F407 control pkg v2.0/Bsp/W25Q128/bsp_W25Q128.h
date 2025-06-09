#ifndef __BSP__W25Q_H
#define __BSP__W25Q_H

#include <stdio.h>

#ifdef USE_HAL_DRIVER  
#include "stm32f4xx_hal.h"
#endif

#ifdef USE_STDPERIPH_DRIVER  
#include <stm32f4xx.h>
#endif



/*****************************************************************************
 ** 引脚定义
 ** 移植时，如果使用SPI1,只需要修改这个区域
****************************************************************************/
#define  GBK_STORAGE_ADDR               0x00A00000       // 汉字GBK字库起始地址,魔女开发板的W25Q128已保存宋体12、16、24、32号字体

#define  W25Q128_SPI                    SPI1             // SPI端口

#ifdef USE_HAL_DRIVER                                    // HAL库
// 引脚复用功能
#define  W25Q128_SPI_AFx                GPIO_AF5_SPI1    // 引脚复用编号
// SCK
#define  W25Q128_SCK_GPIO               GPIOA            // SCK引脚; 时钟同步引脚
#define  W25Q128_SCK_PIN                GPIO_PIN_5    
// MISO                              
#define  W25Q128_MISO_GPIO              GPIOA            // MISO引脚; 主机输入从机输出
#define  W25Q128_MISO_PIN               GPIO_PIN_6                
// MOSI                                  
#define  W25Q128_MOSI_GPIO              GPIOA            // MOSI引脚; 主机输出从机输入
#define  W25Q128_MOSI_PIN               GPIO_PIN_7                
// CS
#define  W25Q128_CS_GPIO                GPIOC            // 片选引脚; SPI总线中，主机拉低哪个设备的CS线，那个设备就接受通信
#define  W25Q128_CS_PIN                 GPIO_PIN_13      
#endif

#ifdef USE_STDPERIPH_DRIVER                              // 标准库
// SCK
#define  W25Q128_SCK_GPIO               GPIOA  
#define  W25Q128_SCK_PIN                GPIO_Pin_5        
#define  W25Q128_SCK_PINSOURCE          GPIO_PinSource5
#define  W25Q128_SCK_AF                 GPIO_AF_SPI1
// MISO                                
#define  W25Q128_MISO_GPIO              GPIOA 
#define  W25Q128_MISO_PIN               GPIO_Pin_6                
#define  W25Q128_MISO_PINSOURCE         GPIO_PinSource6
#define  W25Q128_MISO_AF                GPIO_AF_SPI1
// MOSI                                   
#define  W25Q128_MOSI_GPIO              GPIOA 
#define  W25Q128_MOSI_PIN               GPIO_Pin_7                
#define  W25Q128_MOSI_PINSOURCE         GPIO_PinSource7
#define  W25Q128_MOSI_AF                GPIO_AF_SPI1
// CS
#define  W25Q128_CS_GPIO                GPIOC 
#define  W25Q128_CS_PIN                 GPIO_Pin_13      
#endif




/*****************************************************************************
 ** 声明全局变量
****************************************************************************/
typedef struct
{
    uint8_t   FlagInit;                 // 初始化状态   0:失败, 1:成功
    uint8_t   FlagGBKStorage;           // GBK字库标志; 0=没有, 1=可用; 作用: 用于判断地址段的写保护, 防止字库被错误写履盖; 并可作LCD的中文输出判断
    char      type[20];                 // 型号
    uint16_t  StartupTimes;             // 记录启动次数
} xW25Q_TypeDef;
extern xW25Q_TypeDef  xW25Q128;         // 声明全局结构体, 用于记录w25qxx信息





/*****************************************************************************
 ** 声明全局函数
 ** 为统一代码以方便移植，已封装成4个对外函数，可完成对其所有存取操作
****************************************************************************/
uint8_t W25Q128_Init(void);                                                    // 初始化
void    W25Q128_ReadData(uint32_t addr, uint8_t *pData, uint16_t num);         // 读数据：addr-地址，*pData-读取后数据缓存，num-要读取的字节数
void    W25Q128_WriteData(uint32_t addr, uint8_t *pData, uint16_t num);        // 写数据：addr-地址，*pData-待写的数据缓存，num-要写入的字节数
void    W25Q128_ReadFontData(uint8_t *pFont, uint8_t size, uint8_t *fontData); // 读取字模：*pFont-汉字，size-字号，*fontData-读取到的字模点阵数据



#endif




