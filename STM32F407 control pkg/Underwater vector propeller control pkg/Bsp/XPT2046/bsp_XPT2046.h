#ifndef  __BSP_XPT2046_H
#define  __BSP_XPT2046_H

#include "bsp_lcd_ili9341.h"
#include "stdio.h"


#ifdef USE_HAL_DRIVER                                        // HAL�� ����
#include "stm32f4xx_hal.h"
#endif

#ifdef USE_STDPERIPH_DRIVER                                  // ��׼�� ����
#include "stm32f4xx.h"
#endif




/*****************************************************************************
 ** ���ų趨��
 ** ��ע��ʹ�õ���ģ��SPI, INT����ʹ��״̬��ѯ, ��ʹ���ж�;
****************************************************************************/
#ifdef USE_HAL_DRIVER                                        // HAL�� ����
#define    XPT2046_IRQ_GPIO         GPIOE                    // �����ź�ָʾ����(��ʹ���ж�) 
#define    XPT2046_IRQ_PIN          GPIO_PIN_4               
#define    XPT2046_IRQ_PORT_CLK     RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_CS_GPIO          GPIOD                    // ģ��SPI_CS
#define    XPT2046_CS_PIN           GPIO_PIN_13              
#define    XPT2046_CS_PORT_CLK      RCC_AHB1Periph_GPIOD     
                                                             
#define    XPT2046_CLK_GPIO         GPIOE                    // ģ��SPI_CLK
#define    XPT2046_CLK_PIN          GPIO_PIN_0               
#define    XPT2046_CLK_PORT_CLK     RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_MOSI_GPIO        GPIOE                    // ģ��SPI_MOSI
#define    XPT2046_MOSI_PIN         GPIO_PIN_2               
#define    XPT2046_MOSI_PORT_CLK    RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_MISO_GPIO        GPIOE                    // ģ��SPI_MISO
#define    XPT2046_MISO_PIN         GPIO_PIN_3  
#define    XPT2046_MISO_PORT_CLK    RCC_AHB1Periph_GPIOE
#endif

#ifdef USE_STDPERIPH_DRIVER                                  // ��׼�� ����
#define    XPT2046_IRQ_GPIO         GPIOE                    // �����ź�ָʾ����(��ʹ���ж�) 
#define    XPT2046_IRQ_PIN          GPIO_Pin_4               
#define    XPT2046_IRQ_PORT_CLK     RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_CS_GPIO          GPIOD                    // ģ��SPI_CS
#define    XPT2046_CS_PIN           GPIO_Pin_13              
#define    XPT2046_CS_PORT_CLK      RCC_AHB1Periph_GPIOD     
                                                             
#define    XPT2046_CLK_GPIO         GPIOE                    // ģ��SPI_CLK
#define    XPT2046_CLK_PIN          GPIO_Pin_0               
#define    XPT2046_CLK_PORT_CLK     RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_MOSI_GPIO        GPIOE                    // ģ��SPI_MOSI
#define    XPT2046_MOSI_PIN         GPIO_Pin_2               
#define    XPT2046_MOSI_PORT_CLK    RCC_AHB1Periph_GPIOE     
                                                             
#define    XPT2046_MISO_GPIO        GPIOE                    // ģ��SPI_MISO
#define    XPT2046_MISO_PIN         GPIO_Pin_3  
#define    XPT2046_MISO_PORT_CLK    RCC_AHB1Periph_GPIOE
#endif



/*****************************************************************************
 ** ����ȫ�ֺ�������                 
 *****************************************************************************/
// ��ʼ��
void      XPT2046_Init (uint16_t lcdWidth, uint16_t lcdHeight, uint8_t dir ); // ��ʼ��(����, У׼���);  ����: ��ʾ������ʾ����, ȡֵ��Χ: 1,2,3,4
// ����У׼
uint8_t   XPT2046_ReCalibration(void);                                        // ������������У׼; ���������겻��ʱ������һ�α��������ñʼ����ε��ʮ�ֹ���߼��ɡ�
// ������⡢��ȡ����; 
uint8_t   XPT2046_IsPressed(void);                                            // �жϴ���������״̬; ����:0-�ͷš�1-����; ��ֲLVGLʱ����lv_port_indev.c�ļ���touchpad_is_pressed()��������ô˺�������;
uint16_t  XPT2046_GetX(void);                                                 // ��ȡ���µ�X����ֵ; ��ֲLVGLʱ����lv_port_indev.c�ļ���touchpad_get_xy()���ñ��������ɻ��(*X)ֵ;
uint16_t  XPT2046_GetY(void);                                                 // ��ȡ���µ�Y����ֵ; ��ֲLVGLʱ����lv_port_indev.c�ļ���touchpad_get_xy()���ñ��������ɻ��(*Y)ֵ;  
// ���ʾ���õĺ���; LVGL���ò���������ɾ��, ��Ӱ��ײ�
void      XPT2046_Cmd(uint8_t status);                                        // ������⿪��; �Ǳ�Ҫ��; ��ʼ����Ĭ���ǿ�����; �����ֶ��رգ���ͣ�����ʵ��һЩ������Ȼ�����ֶ�����
void      XPT2046_TouchDown(void);                                            // �հ׺���, ���ð���ʱ�Ĵ���
void      XPT2046_TouchUp(void);                                              // �հ׺���, �����ͷ�ʱ�Ĵ���   




#endif 

