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
 ** ���Ŷ���
 ** ��ֲʱ�����ʹ��SPI1,ֻ��Ҫ�޸��������
****************************************************************************/
#define  GBK_STORAGE_ADDR               0x00A00000       // ����GBK�ֿ���ʼ��ַ,ħŮ�������W25Q128�ѱ�������12��16��24��32������

#define  W25Q128_SPI                    SPI1             // SPI�˿�

#ifdef USE_HAL_DRIVER                                    // HAL��
// ���Ÿ��ù���
#define  W25Q128_SPI_AFx                GPIO_AF5_SPI1    // ���Ÿ��ñ��
// SCK
#define  W25Q128_SCK_GPIO               GPIOA            // SCK����; ʱ��ͬ������
#define  W25Q128_SCK_PIN                GPIO_PIN_5    
// MISO                              
#define  W25Q128_MISO_GPIO              GPIOA            // MISO����; ��������ӻ����
#define  W25Q128_MISO_PIN               GPIO_PIN_6                
// MOSI                                  
#define  W25Q128_MOSI_GPIO              GPIOA            // MOSI����; ��������ӻ�����
#define  W25Q128_MOSI_PIN               GPIO_PIN_7                
// CS
#define  W25Q128_CS_GPIO                GPIOC            // Ƭѡ����; SPI�����У����������ĸ��豸��CS�ߣ��Ǹ��豸�ͽ���ͨ��
#define  W25Q128_CS_PIN                 GPIO_PIN_13      
#endif

#ifdef USE_STDPERIPH_DRIVER                              // ��׼��
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
 ** ����ȫ�ֱ���
****************************************************************************/
typedef struct
{
    uint8_t   FlagInit;                 // ��ʼ��״̬   0:ʧ��, 1:�ɹ�
    uint8_t   FlagGBKStorage;           // GBK�ֿ��־; 0=û��, 1=����; ����: �����жϵ�ַ�ε�д����, ��ֹ�ֿⱻ����д�ĸ�; ������LCD����������ж�
    char      type[20];                 // �ͺ�
    uint16_t  StartupTimes;             // ��¼��������
} xW25Q_TypeDef;
extern xW25Q_TypeDef  xW25Q128;         // ����ȫ�ֽṹ��, ���ڼ�¼w25qxx��Ϣ





/*****************************************************************************
 ** ����ȫ�ֺ���
 ** Ϊͳһ�����Է�����ֲ���ѷ�װ��4�����⺯��������ɶ������д�ȡ����
****************************************************************************/
uint8_t W25Q128_Init(void);                                                    // ��ʼ��
void    W25Q128_ReadData(uint32_t addr, uint8_t *pData, uint16_t num);         // �����ݣ�addr-��ַ��*pData-��ȡ�����ݻ��棬num-Ҫ��ȡ���ֽ���
void    W25Q128_WriteData(uint32_t addr, uint8_t *pData, uint16_t num);        // д���ݣ�addr-��ַ��*pData-��д�����ݻ��棬num-Ҫд����ֽ���
void    W25Q128_ReadFontData(uint8_t *pFont, uint8_t size, uint8_t *fontData); // ��ȡ��ģ��*pFont-���֣�size-�ֺţ�*fontData-��ȡ������ģ��������



#endif




