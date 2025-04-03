#ifndef __BSP__UART_H
#define __BSP__UART_H

#include <stdarg.h>                               // ����֧�ֲ���������
#include "stdio.h"                                // ����֧�ֺ���printf��spritnf��
#include "string.h"                               // ����֧���ַ���������strset��strcpy��memset��metcpy��


#ifdef USE_STDPERIPH_DRIVER
    #include "stm32f4xx.h"                        // ��׼��
#endif

#ifdef USE_HAL_DRIVER
    #include "stm32f4xx_hal.h"                    // HAL��
#endif



/*****************************************************************************
 ** ��ֲ�����޸���
 ** ��ע������������Ҫ�󣬷������������ͨ�ã�RS485������ģ�顢ESP8266ģ�顢��������
****************************************************************************/
// ���ڿ���
#define UART1_EN                       1          // ����1��0=�ء�1=����;  ����û�õ�UART1, ��0���Ͳ��Ὺ��UART1���ͻ��桢���ջ��棬ʡһ����Դ;
#define UART2_EN                       1          // ����2��0=�ء�1=����;  ͬ��;
#define UART3_EN                       1          // ����3��0=�ء�1=����;  ͬ��;
#define UART4_EN                       1          // ����4��0=�ء�1=����;  ͬ��;
#define UART5_EN                       1          // ����5��0=�ء�1=����;  ͬ��;
#define UART6_EN                       1          // ����5��0=�ء�1=����;  ͬ��;
// ���ͻ�������С
#define UART1_TX_BUF_SIZE           2048          // ����ÿ��UART���ͻ��λ���������Ĵ�С����λ���ֽ���; 
#define UART2_TX_BUF_SIZE            512          // -- ֻ����ǰ�洮�ڿ����ڴ�״̬���Żᶨ�����Ļ���������
#define UART3_TX_BUF_SIZE            512          // -- Ĭ��ֵ��512����ֵ��������󲿳�����ͨ��; �����ESP8266֮����豸ͨ�ţ����ʵ������ֵ��
#define UART4_TX_BUF_SIZE            512          // -- ֵ��Χ��1~65535; ע���ʼ���󣬲�Ҫ����оƬ���RAMֵ��
#define UART5_TX_BUF_SIZE            512          // -- ע���ֵ��һ�����λ�������С������ÿһ֡���֡���ݽ��뷢��ǰ���ܻ����ֽ������Ƚ��ȳ���
#define UART6_TX_BUF_SIZE            512          //
// ���ջ�������С
#define UART1_RX_BUF_SIZE            512          // ����ÿ��UART���ջ������Ĵ�С����λ���ֽ�; ��ֵӰ�췶Χ���ж���Ľ��ջ����С�����պ����ݻ���Ĵ�С
#define UART2_RX_BUF_SIZE            512          // --- �����յ���һ֡�����ֽ�����С�ڴ�ֵʱ������������
#define UART3_RX_BUF_SIZE            512          // --- �����յ���һ֡�����ֽ�����������ֵʱ���������֣������ж���ֱ�����ᣬֱ����֡���ս���(���������ж�); 
#define UART4_RX_BUF_SIZE            512          // 
#define UART5_RX_BUF_SIZE            512          // 
#define UART6_RX_BUF_SIZE            512          // 
// ����-�����޸�



/*****************************************************************************
 ** ����ȫ�ֺ���
 **
 ** ÿ�����ڵĺ���:
 ** ��ʼ��    1��   ������-None-8-1
 ** ����      3��   ����ָ���������ݡ��ַ�����ATָ�� 
 ** ����      3��   ��ȡ�ֽ�������ȡ���ݡ���0
****************************************************************************/
// UART1
void      UART1_Init(uint32_t ulBaudrate);                                      // ��ʼ������1; GPIO����PA9+PA10���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART1_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART1_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART1_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART1_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART1_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART1_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
// UART2                                                                        
void      UART2_Init(uint32_t ulBaudrate);                                      // ��ʼ������2; GPIO����PA2+PA3���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART2_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART2_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART2_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART2_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART2_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART2_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
// UART3                                                                        
void      UART3_Init(uint32_t ulBaudrate);                                      // ��ʼ������3; GPIO����PB10+PB11���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART3_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART3_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART3_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART3_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART3_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART3_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
// UART4                                                            
void      UART4_Init(uint32_t ulBaudrate);                                      // ��ʼ������4; GPIO����PC10+PC11���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART4_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART4_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART4_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART4_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART4_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART4_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
// UART5                                                                        
void      UART5_Init(uint32_t ulBaudrate);                                      // ��ʼ������5; GPIO����PC12+PD2���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART5_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART5_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART5_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART5_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART5_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART5_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
// UART6                                                                        
void      UART6_Init(uint32_t ulBaudrate);                                      // ��ʼ������6; GPIO����PC6+PC7���ж����ȼ���ͨ�Ų����������ʿ��衢8λ���ݡ���У�顢1��ֹͣλ
void      UART6_SendData(uint8_t *puData, uint16_t usNum);                      // ����ָ������; ���������ݵ�ַ���ֽ���
void      UART6_SendString(const char *pcString, ...);                          // �����ַ���;   �������ַ�����ַ; ʹ�÷�����ͬprintf
uint8_t   UART6_SendAT(char *pcAT, char *pcAckString, uint16_t usTimeOutMs);    // �����������ESP8266������ģ���AT�̼������ڵȴ������ڴ�����Ϣ; ������ATָ���ַ������ڴ�������Ϣ�ַ�����ms�ȴ���ʱ; ���أ�0-ִ��ʧ�ܡ�1-ִ�гɹ� 
uint16_t  UART6_GetRxNum(void);                                                 // ��ȡ���յ�������һ֡�ֽ���
uint8_t * UART6_GetRxData(void);                                                // ��ȡ���յ������� (����ĵ�ַ)
void      UART6_ClearRx(void);                                                  // ������յ������� (�������һ֡�ֽ�������Ϊ�����жϽ��յı�־)
                                                                                
// ���Ը���                                                                     
void      showData(uint8_t *puRxData, uint16_t usRxNum);                        // ��printf�����͵����������ϣ�����۲�


#endif

