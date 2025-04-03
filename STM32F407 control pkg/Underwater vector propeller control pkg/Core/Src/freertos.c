/******************************************************************************
 * ��  �ߣ� ��ɽ��ѧ�ֳ�Ȼ
 * ��  �ܣ� ʵ����Ļ�������ֱ��������ַ�ʽ����ˮ��ʸ���ƽ����˶�
 * ��  ���� ������ؿ�ο�ħŮ�Ƽ�����Ȥ�Ƽ�������Ƽ���������
 * ��Ȩ�����ֳ�Ȼ������ؾ�
******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
#include "bsp_UART.h"
#include "bsp_CAN.h"
#include "usb_host.h"
#include "bsp_LCD_ILI9341.h"
#include "bsp_W25Q128.h"
#include "bsp_XPT2046.h"
#include "dm_motor_ctrl.h"
#include "dm_motor_drv.h"
#include "driver_timer.h"


/* Private typedef -----------------------------------------------------------*/
extern void MX_USB_HOST_Process(void);

/* ���������� */
TaskHandle_t xCommunicationTaskHandle = NULL;
TaskHandle_t xTouch_ManageTaskHandle = NULL;
TaskHandle_t xMotorTaskHandle = NULL;
BaseType_t ret;  // �����ж����񴴽��Ƿ�ɹ��ı���
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/******************************************************************************
 * ��  ���� CommunicationTask
 * ��  �ܣ� ͨѶ(USART+CAN)����������
 * ��  ���� ��
 * ����ֵ�� ��
 * ��  ע�� ֻ��Ҫ�⿪�����ע�;Ϳ��Խ���ͨѶ����
******************************************************************************/
void CommunicationTask(void *params){
  while(1){
        /** 10ms��ʱ,�ͷſ���Ȩ **/        
        vTaskDelay(10);                                                                                              
//        /** ���UART1�Ƿ��յ����� (��������) **/                                                        
//        if (UART1_GetRxNum())                                                                            // �ж�UART1�Ƿ��ȡ����һ֡���ݣ��������صĽ����ֽ���������0����Ϊ�յ���һ֡����
//        {                                                                                               
//            uint8_t *rxData = UART1_GetRxData();                                                         // ��ȡ���ݵĵ�ַ
//            uint16_t rxNum = UART1_GetRxNum();                                                           // ��ȡ���ݵ��ֽ���
//                                                                                                        
//            printf("\r<<<<< USART1 �յ�һ֡���� \r");                                                    // ��ʾ
//            printf("�ֽ�����%d \r", rxNum);                                                              // ��ʾ�ֽ���
//            printf("ASCII : %s\r", (char *)rxData);                                                      // ��ʾ���ݣ���ASCII��ʽ��ʾ�������ַ����ķ�ʽ��ʾ
//            printf("16����: ");                                                                          // ��ʾ���ݣ���16���Ʒ�ʽ����ʾÿһ���ֽڵ�ֵ
//            for (uint16_t i = 0; i < rxNum; i++)                                                         // ����ֽ����
//                printf("0x%X ", rxData[i]);                                                              // ��16������ʾ
//            printf("\r\r");                                                                              // ��ʾ����
//                                                                                                        
//            // UART1_SendString("Hello World��\r");                                                      // ����ʾ�� UART1_SendString( )��ʹ��; �ɽ�����pritnf�ĸ�ʽ������
//            // CAN1_SendData(rxData, 8);                                                                    // ����ʾ�� CAN1_SendData( )��ʹ��, �����ڷ����ַ��������顢�ṹ�����ݵ�
//            UART1_ClearRx();                                                                             // ��Ҫ����0���ձ�־������0���յ����ֽ���; ÿ�δ���������ݣ���Ҫ�������������0
//        }
//        /** ���CAN1�Ƿ��յ����� **/
//        if (xCAN1.RxNum)                                                                                 // ���ձ�־λ���ֽ���; ����ֵ�Ƿ����0�����ж��Ƿ���յ���һ֡����
//        {
//            static uint16_t  canRxCNT = 1;                                                               // ���ڼ����ѽ����˶���֡����; �Ǳ�Ҫ;
//            printf("\r****** CAN ���յ���%d֡������ ******", canRxCNT++);                                // ׼����CAN֡���ģ���ϸ��������������������۲����
//            printf("\r ֡���ͣ�%s",   xCAN1.RxHeader.RTR ? "ң��֡" : "����֡");                         // ֡����
//            printf("\r ֡��ʽ��%s",   xCAN1.RxHeader.IDE ? "��չ֡" : "��׼֡");                         // ֡��ʽ
//            printf("\r ��ʶ����0x%X", xCAN1.RxHeader.IDE ? xCAN1.RxHeader.ExtId : xCAN1.RxHeader.StdId); // ֡ID
//            printf("\r �ֽ�����%d",   xCAN1.RxHeader.DLC);                                               // �ֽ���
//            printf("\r ɸѡ��ƥ����ţ�%d", xCAN1.RxHeader.FilterMatchIndex);                            // ɸѡ��ƥ�����; ��ɸѡ����ţ��ǲ�һ���ġ���ţ���ɸѡ��0��ʼ��ÿ��16λ��ɸѡ��+2, 32λ��+1, û�б�ʹ�õ�ɸѡ����Ĭ����16λ��+2;
//            printf("\r ��ʾ����(16����)��");                                                             // 16���Ʒ�ʽ��ʾ���ݣ�����۲���ʵ����
//            for (uint8_t i = 0; i < xCAN1.RxNum; i++)                                                    // ����ֽ���ʾCAN������ֵ
//                printf(" 0x%X ", xCAN1.RxData[i]);                                                       // ��ʽ��16������ʾ
//            printf("\r");                                                                                // ���У���ʹÿ֡�����������

//            xCAN1.RxNum = 0;                                                                             // �ֽ�����0; ÿ�δ�����һ֡���ݣ���Ҫ��xCAN1.RxNum������0������xCAN1.RxNum=0���Է�����һ�ֵ��жϣ������ظ�����ͬһ֡����;

//            if (strstr((char *)xCAN1.RxData, "CAN_Test"))                                                // �ж��Ƿ���Թ��߷�������ָ������
//            {
//                static char strTemp[8] = "CAN_OK";                                                       // ���Թ���Լ���õķ�������
//                canx_send_data(&hcan1, CAN_TX_ID, (uint8_t *)strTemp, 8);                                // ����CAN����
//            }
//        }
//        /** ÿ500ms����һ֡CAN���ģ�����CAN�����ϵ������豸���Խ��� **/
//        static uint16_t msCAN = 0;                                                                   
//        if (++msCAN == 500)                                                                              // ÿ500msִ��һ��
//        {                                                                                             
//            msCAN = 0;                                                                                   // ������0
//            static uint8_t msgData[30] = { 0, 2, 4, 6, 8, 10, 12, 14 };                                  // ����һ���򵥵����飬����CAN�ķ���ʾ��; ���￪����30�ֽڵĿռ䣬��ʱֻ��ֵ��ǰ8���ֽڣ���22���ֽ�ֵΪ0;
//            canx_send_data(&hcan1, CAN_TX_ID, msgData, 6);                                               // CAN1��������(�Զ��庯��)��ÿ500msһ�Σ��Է����ⲿ�豸���Խ���; CANÿ֡���8�ֽڣ�����ֻ��6�ֽ���Ϊ��ʾ����CAN���͵��ֽ��������鶨��Ĵ�С��ûһëǮ��ϵ��
//        }                                                                                                                                                                                                   
        /** ÿ500ms��˸һ����ɫLED, �Է����ⲿ�۲�������� **/                                        
        static uint16_t msLED = 0;
        if(g_start_tick<200)g_start_tick++;                                                             // g_start_tickС��200���򲻶�ȡcan����������
        if (++msLED == 50)                                                                              // ÿ500msִ��һ��
        {                                                                                             
            msLED = 0;                                                                                   // ������0
            HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);                                        // ���ɵ���˸��ɫLED�������ⲿ�۲�ϵͳ�Ƿ���������
        }
    }
};


/******************************************************************************
 * ��  ���� Motor_Task
 * ��  �ܣ� �����������
 * ��  ���� ��
 * ����ֵ�� ��
 * ��  ע�� ʹ�ÿ⺯�������Եķ���MITģʽ����֡
******************************************************************************/
void Motor_Task(void  *params){
  TickType_t pretime;
  pretime = xTaskGetTickCount();
  while(1){
    dm_motor_ctrl_send(&hcan1, &motor[Motor1]);  // ���ڷ��͵��λ�ÿ���
    dm_motor_ctrl_send(&hcan1, &motor[Motor2]);
    vTaskDelay(1);                               // ��ֹ��֡
    dm_motor_ctrl_send(&hcan1, &motor[Motor3]);
    dm_motor_ctrl_send(&hcan1, &motor[Motor4]);
    vTaskDelayUntil(&pretime, 2);                // ��500HZ���ڷ���λ�ÿ���
  }
};



/******************************************************************************
 * ��  ���� Touch_ManageTask
 * ��  �ܣ� ���ڼ�ⴥ���͹�������������������
 * ��  ���� ��
 * ����ֵ�� ��
 * ��  ע�� ���е���˼�Ѿ�ȫ������
******************************************************************************/
void Touch_ManageTask(void *params){
//��ҳ������ʾ
    LCD_Fill(0, 0, 240, 320, WHITE);                                                // �������������ɫ
    extern const unsigned char yanda_152_60[ ];                                     // ͼƬ��������; �����font.h�ļ��С��������ʾ����������ʹ��, ̫����Դ�ˡ�
    LCD_Image(45,  150,  152, 60, yanda_152_60);                                    // ͼƬ��ʾ��x���ꡢY���ꡢͼƬ��ȡ�ͼƬ�߶ȣ�ͼƬ�����ַ; ע�⣬�����ͼƬ�ߴ���ָ��ʵ�ߴ�
    LCD_String(10, 285, "����������겻�ԣ�������1��������У׼", 12, BLACK, WHITE);   // ��ʾ����;
    LCD_String(80, 80, "���ߣ��ֳ�Ȼ", 12, YELLOW, WHITE);     // ��ʾ����;
    LCD_String(68, 100, "ָ����ʦ��Ҷ�ݼ�", 12, YELLOW, WHITE);     // ��ʾ����;
    LCD_String(35, 8, "���ڻ���������", 24, BLACK, WHITE);     // ��ʾ����;
    LCD_String(40, 40, "�����ˮ��ʸ���ƽ���", 16, DARKBLUE, WHITE);     // ��ʾ����;
    vTaskDelay(3000);
    
    // ���ƽ�����ʾ
    LCD_Fill(0, 0, 240, 320, BLACK);                                                 // �������������ɫ
    LCD_String(10, 8, "ˮ �� ʸ �� �� ��", 24, WHITE, BLACK);                        // ��ʾ����;
    LCD_String(58, 40, "�� �� �� ��", 24, YELLOW, BLACK);                            // ��ʾ����;
    LCD_Line(0, 37, 240, 37, GRAY);                                                  // ��������    
    LCD_Line(0, 38, 240, 38, GRAY);                                                  // �������ߣ��Ӵ�
    //LCD_String(18, 270, "���ڻ��������Ĵ����ˮ��ʸ���ƽ���", 12, WHITE, BLACK);     // ��ʾ����;

    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<�����������<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //���״̬��ǩ��ʾ
    LCD_String(14, 72, "���������״̬��", 16, WHITE, BLACK); 
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                       // �ж�pwm��ֵ����ʾ���״̬Ϊֹͣ����ת�ͷ�ת
    LCD_Rectangle(12, 70, 176, 90, GRAY);                                            // ���ƾ��ο�
    //��ť-��ʾ
    LCD_BOTTON_SUB(22, 105, BLACK, WHITE);
    //��ť+��ʾ
    LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
    //PWM��ǩ��ʾ
    LCD_String(80, 105-8, "PWM:", 16, WHITE, BLACK);
    //ʹ�ܺ�ǿͣ��ť��ʾ
    Judge_enable(g_rov_status, 181,72);                                             // �ж�status��ֵ����ʾ����Ƿ�ʹ��
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);        // �ж�pwm��ֵ����ʾ����Ƿ�λ
    else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
    //pwm��ֵ��ǩ��ʾ
    char strTem[100];                                                           
    sprintf(strTem, "%d", Rovmotor_PWM);                                          // ʹ��sprintf�����ַ�����ʽ���������������
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                            
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>�����������>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM4310�ؽڵ����<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //���״̬��ǩ��ʾ
    LCD_String(14, 118, "�����״̬��", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm4310_status, 110, 118);                                 // �жϷ���֡status��ֵ����ʾ���״̬
    LCD_Rectangle(12, 116, 176, 136, GRAY);                                          // ���ƾ��ο�
    //ʹ��,���Ͱ�ť��ʾ
    Judge_enable(g_dm4310_status, 181,118);                                          // �ж�status��ֵ����ʾ����Ƿ�ʹ��
    LCD_String(181, 151-8, "����", 16, BLACK, GREY);                                 // ��������㰴ť
    //��ť-��ʾ
    LCD_BOTTON_SUB(22, 151, BLACK, WHITE);
    //��ť+��ʾ
    LCD_BOTTON_ADD(50, 151, BLACK, WHITE);
    //λ�ñ�ǩ��ʾ
    LCD_String(80, 151-8, "POS:", 16, WHITE, BLACK);
    //λ����ֵ��ǩ��ʾ                                                        
    sprintf(strTem, "%3.1f", g_dm4310_real_pos*180/3.14f);                            // ʹ��sprintf�����ַ�����ʽ���������������
    LCD_String(120, 151-8, strTem, 16, WHITE, BLACK);  
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM4310�ؽڵ����>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��1��<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //���״̬��ǩ��ʾ
    LCD_String(14, 164, "�ؽ�1״̬��", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_1_status, 110, 164);                                 // �жϷ���֡status��ֵ����ʾ���״̬
    LCD_Rectangle(12, 162, 176, 182, GRAY);                                          // ���ƾ��ο�
    //ʹ��,������㰴ť��ʾ
    Judge_enable(g_dm3910_1_status, 181,164);                                          // �ж�status��ֵ����ʾ����Ƿ�ʹ��
    LCD_String(181, 197-8, "����", 16, BLACK, GREY);                                   // ��������㰴ť
    //��ť-��ʾ
    LCD_BOTTON_SUB(22, 197, BLACK, WHITE);
    //��ť+��ʾ
    LCD_BOTTON_ADD(50, 197, BLACK, WHITE);
    //λ�ñ�ǩ��ʾ
    LCD_String(80, 197-8, "POS:", 16, WHITE, BLACK);
    //λ����ֵ��ǩ��ʾ                                                        
    sprintf(strTem, "%3.1f", g_dm3910_1_real_pos*180/3.14f);                            // ʹ��sprintf�����ַ�����ʽ���������������
    LCD_String(120, 197-8, strTem, 16, WHITE, BLACK);  
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��1��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��2��<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //���״̬��ǩ��ʾ
    LCD_String(14, 210, "�ؽ�2״̬��", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_2_status, 110, 210);                                 // �жϷ���֡status��ֵ����ʾ���״̬
    LCD_Rectangle(12, 208, 176, 228, GRAY);                                          // ���ƾ��ο�
    //ʹ��,������㰴ť��ʾ
    Judge_enable(g_dm3910_2_status, 181,210);                                          // �ж�status��ֵ����ʾ����Ƿ�ʹ��
    LCD_String(181, 243-8, "����", 16, BLACK, GREY);                                   // ��������㰴ť
    //��ť-��ʾ
    LCD_BOTTON_SUB(22, 243, BLACK, WHITE);
    //��ť+��ʾ
    LCD_BOTTON_ADD(50, 243, BLACK, WHITE);
    //λ�ñ�ǩ��ʾ
    LCD_String(80, 243-8, "POS:", 16, WHITE, BLACK);
    //λ����ֵ��ǩ��ʾ                                                        
    sprintf(strTem, "%3.1f", g_dm3910_2_real_pos*180/3.14f);                            // ʹ��sprintf�����ַ�����ʽ���������������
    LCD_String(120, 243-8, strTem, 16, WHITE, BLACK);
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��2��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��3��<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //���״̬��ǩ��ʾ
    LCD_String(14, 256, "�ؽ�3״̬��", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_3_status, 110, 256);                                 // �жϷ���֡status��ֵ����ʾ���״̬
    LCD_Rectangle(12, 254, 176, 274, GRAY);                                          // ���ƾ��ο�
    //ʹ��,������㰴ť��ʾ
    Judge_enable(g_dm3910_3_status, 181,256);                                          // �ж�status��ֵ����ʾ����Ƿ�ʹ��
    LCD_String(181, 289-8, "����", 16, BLACK, GREY);                                   // ��������㰴ť
    //��ť-��ʾ
    LCD_BOTTON_SUB(22, 289, BLACK, WHITE);
    //��ť+��ʾ
    LCD_BOTTON_ADD(50, 289, BLACK, WHITE);
    //λ�ñ�ǩ��ʾ
    LCD_String(80, 289-8, "POS:", 16, WHITE, BLACK);
    //λ����ֵ��ǩ��ʾ                                                        
    sprintf(strTem, "%3.1f", g_dm3910_3_real_pos*180/3.14f);                            // ʹ��sprintf�����ַ�����ʽ���������������
    LCD_String(120, 289-8, strTem, 16, WHITE, BLACK);
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��3��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<�ױ߿�����<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    Judge_transmit(g_transmit_status, 181, 310-8);                                   // �ж��Ƿ񴴽�CAN��������ť
    LCD_String(15, 310-8, "ҳ��һ", 16, WHITE, GREEN);                               // ����һ��ҳ��һ������ͬʱɾ��ҳ������������
    LCD_String(65, 310-8, "ҳ���", 16, BLACK, GRAY);                               // ����һ��ҳ���������ͬʱɾ��ҳ��һ���������
    Judge_machine_mode(g_machine_mode, 115,310-8);                                  // ��ʾ��ǰ�Ŀ���ģʽ�����¿����л�
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>�ױ߿�����>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    while (1)
    {
        uint16_t touch_x = 0;
        uint16_t touch_y = 0;
        /** 1ms��ʱ **/
        vTaskDelay(1);                                                  
        /** ��鴥�����Ƿ��£���������ش��� **/
        if (XPT2046_IsPressed() && g_handup == 1)                          // ��鴥�����Ƿ��̧��״̬����; ���أ�0-δ���¡�1-����
        {
          vTaskDelay(10);                                                  //rtos�иĳ�vtaskdelay���У��Ͳ��ö�ʱ���ٶ�ʱ��
          if (XPT2046_IsPressed()){
            g_handup = 0;                                                  // ȷ�ϴ�������
            touch_x = XPT2046_GetX();                                      // ��ȡ������xλ��
            touch_y = XPT2046_GetY();                                      // ��ȡ������yλ��
            LCD_DrawPoint(touch_x, touch_y, GREEN);                        // �ڰ��µ�λ�ã�����; ���ڲ���
            static char str[20] = {0};                                     // �½����飬���ڴ�������ַ���
            sprintf(str, "X:%3d  Y:%3d", touch_x, touch_y);                // ��ʽ�������ַ���
            LCD_String(8, 5, str, 12, WHITE, BLACK);                       // ��ʾ�����ַ���
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<�����������<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 72<touch_y && touch_y<88 && g_machine_mode==0){   // ���������ʹ�ܰ�ť��⣬������ر�
              if (g_rov_status == 0){g_rov_status = 1;HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);}
              else if (g_rov_status == 1) {g_rov_status = 0;HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_4);}
              Judge_enable(g_rov_status, 181,72);                                               // �ж�status��ֵ����ʾ����Ƿ�ʹ��
              }
            
            if (181<touch_x && touch_x<213 && 97<touch_y && touch_y<113 && g_machine_mode==0){  // �����������λ��ť���
              Rovmotor_PWM = 750;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // ����PA3�ӿڵ�PWMֵ
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
              if (Rovmotor_PWM != 75) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);
              else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                        // �ж�pwm��ֵ����ʾ���״̬Ϊֹͣ����ת�ͷ�ת
              }
            
            if (14<touch_x && touch_x<30 && 97<touch_y && touch_y<113 && g_machine_mode==0){    // ���������pwm-��ť���
              Rovmotor_PWM = Rovmotor_PWM - 50;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // ����PA3�ӿڵ�PWMֵ
              LCD_BOTTON_SUB(22, 105, BLACK, BLUE);                                             // ��ʾ��ť������Ķ���-��ť��ɫ
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
              if (Rovmotor_PWM != 750) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);
              else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                                 // ������ʾ��PWMֵ
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 105, BLACK, WHITE);                                            // ���»��ư�ť-��ť�ָ�
              }
            
            if (42<touch_x && touch_x<58 && 97<touch_y && touch_y<113 && g_machine_mode==0){    //���������pwm+��ť���
              Rovmotor_PWM = Rovmotor_PWM + 50;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // ����PA3�ӿڵ�PWMֵ
              LCD_BOTTON_ADD(50, 105, BLACK, BLUE);
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
              if (Rovmotor_PWM != 750) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);
              else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>�����������>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM4310�ؽڵ����<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 118<touch_y && touch_y<134 && g_machine_mode==0){  // �������ʹ�ܰ�ť���
                if (g_dm4310_status == 0){                                                       // ʹ��DM4310���
                  //g_dm4310_status = 1;
                  dm_motor_enable(&hcan1, &motor[Motor1]);
                  vTaskDelay(200);                                                               // ��ʱ����ֹ������
                }
                else {                                                                           // ʧ��DM4310���
                  //g_dm4310_status = 0;
                  dm_motor_disable(&hcan1, &motor[Motor1]);
                  vTaskDelay(200);                                                               // ��ʱ����ֹ������
                }                                                      
                Judge_enable(g_dm4310_status, 181,118);                                          // �ж�status��ֵ����ʾ����Ƿ�ʹ��              
              }
            
            if (181<touch_x && touch_x<213 && 143<touch_y && touch_y<159 && g_machine_mode==0){   // ��ǰλ����Ϊ���İ�ť���
                //������Ϊ���ĺ���
                LCD_String(181, 151-8, "����", 16, BLACK, BLUE);                     // ��ʾ��ť������Ķ���-��ť��ɫ
                save_pos_zero(&hcan1, motor[Motor1].id, motor[Motor1].ctrl.mode);    // ���浱ǰλ��Ϊ�õ�����
                if (g_dm4310_real_pos>0)motor[Motor1].ctrl.vel_set 	= -3.0f;         // ��Ϊ����и��أ���Ҫ��ǰ�����ٶ�
                else motor[Motor1].ctrl.vel_set = 3.0f;                              // ���λ���Ǹ��ģ���Ҫ�����ٶ������������֮���ٶ�
                g_dm4310_target_pos = 0.0f;
                motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
                vTaskDelay(50);                                                      // �ȴ�һ��
                LCD_String(181, 151-8, "����", 16, BLACK, WHITE);                    // ���»��ư�ť-��ť�ָ�             
              }
            
            if (14<touch_x && touch_x<30 && 143<touch_y && touch_y<159 && g_machine_mode==0){    // �������pos-��ť��⣬����ı����target��pos��ʾ����real
              motor[Motor1].ctrl.vel_set 	= -3.0f;                                  // ��ǰ���ٶ�
              g_dm4310_target_pos = g_dm4310_target_pos - 0.2f;                     // ����motor[Motor1]�ṹ���з��͸������λ��
              motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
              LCD_BOTTON_SUB(22, 151, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 151, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            
            if (43<touch_x && touch_x<58 && 143<touch_y && touch_y<159 && g_machine_mode==0){   // �������pos+��ť��⣬����ı����target��pos��ʾ����real
              motor[Motor1].ctrl.vel_set 	= 3.0f;                                   // ��ǰ���ٶ�
              g_dm4310_target_pos = g_dm4310_target_pos + 0.2f;                     // ����motor[Motor1]�ṹ���з��͸������λ��
              motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
              LCD_BOTTON_ADD(50, 151, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 151, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM4310�ؽڵ����>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��1��0x02<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 164<touch_y && touch_y<180 && g_machine_mode==0){ // DM3910���1ʹ�ܰ�ť���
                if (g_dm3910_1_status == 0){                                         // ʹ��DM3519���1
                  //g_dm3910_1_status = 1;
                  dm_motor_enable(&hcan1, &motor[Motor2]);
                  vTaskDelay(200);
                }
                else  {                                                              // ʧ��DM3519���1
                  //g_dm3910_1_status = 0;                                       
                  dm_motor_disable(&hcan1, &motor[Motor2]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_1_status, 181,164);                              // �ж�status��ֵ����ʾ����Ƿ�ʹ��              
              }
            
            if (181<touch_x && touch_x<213 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // ��ǰλ����Ϊ���İ�ť���
                //������Ϊ���ĺ���
                LCD_String(181, 197-8, "����", 16, BLACK, BLUE);                     // ��ʾ��ť������Ķ���-��ť��ɫ
                save_pos_zero(&hcan1,motor[Motor2].id, motor[Motor2].ctrl.mode);
                g_dm3910_1_target_pos = 0.0f;
                motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
                vTaskDelay(80);
                LCD_String(181, 197-8, "����", 16, BLACK, WHITE);                    // ���»��ư�ť-��ť�ָ�             
              }
            
            if (14<touch_x && touch_x<30 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // �������pos-��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_1_target_pos = g_dm3910_1_target_pos - 0.2*3591/187.0f;
              motor[Motor2].ctrl.pos_set = g_dm3910_1_target_pos;
              LCD_BOTTON_SUB(22, 197, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 197, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            
            if (43<touch_x && touch_x<58 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // �������pos+��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_1_target_pos = g_dm3910_1_target_pos + 0.2*3591/187.0f;
              motor[Motor2].ctrl.pos_set = g_dm3910_1_target_pos;
              LCD_BOTTON_ADD(50, 197, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 197, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��1��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��2��0x03<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 210<touch_y && touch_y<226 && g_machine_mode==0){ // DM3910���2ʹ�ܰ�ť���
                if (g_dm3910_2_status == 0){
                  dm_motor_enable(&hcan1,&motor[Motor3]);
                  vTaskDelay(200);
                }
                else {
                  dm_motor_disable(&hcan1,&motor[Motor3]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_2_status, 181,210);                            // �ж�status��ֵ����ʾ����Ƿ�ʹ��              
              }
            
            if (181<touch_x && touch_x<213 && 235<touch_y && touch_y<251 && g_machine_mode==0){ // ��ǰλ����Ϊ���İ�ť���
                //������Ϊ���ĺ���
                LCD_String(181, 243-8, "����", 16, BLACK, BLUE);                     // ��ʾ��ť������Ķ���-��ť��ɫ
                save_pos_zero(&hcan1,motor[Motor3].id, motor[Motor3].ctrl.mode);
                g_dm3910_2_target_pos = 0.0f;
                motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
                vTaskDelay(80);
                LCD_String(181, 243-8, "����", 16, BLACK, WHITE);                    // ���»��ư�ť-��ť�ָ�             
              }
            
            if (14<touch_x && touch_x<30 && 235<touch_y && touch_y<251 && g_machine_mode==0){  // �������pos-��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_2_target_pos = g_dm3910_2_target_pos - 0.2*3591/187.0f;
              motor[Motor3].ctrl.pos_set = g_dm3910_2_target_pos;
              LCD_BOTTON_SUB(22, 243, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 243, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            
            if (43<touch_x && touch_x<58 && 235<touch_y && touch_y<251 && g_machine_mode==0){ // �������pos+��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_2_target_pos = g_dm3910_2_target_pos + 0.2*3591/187.0f ;
              motor[Motor3].ctrl.pos_set = g_dm3910_2_target_pos;
              LCD_BOTTON_ADD(50, 243, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 243, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��2��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910�ؽڵ��3��0x04<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/  
            if (181<touch_x && touch_x<213 && 256<touch_y && touch_y<272 && g_machine_mode==0){ // DM3910���3ʹ�ܰ�ť���
                if (g_dm3910_3_status == 0){
                  dm_motor_enable(&hcan1,&motor[Motor4]);
                  vTaskDelay(200);
                }
                else {
                  dm_motor_disable(&hcan1,&motor[Motor4]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_3_status, 181,256);                            // �ж�status��ֵ����ʾ����Ƿ�ʹ��              
              }
            
            if (181<touch_x && touch_x<213 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // ��ǰλ����Ϊ���İ�ť���
                //������Ϊ���ĺ���
                LCD_String(181, 289-8, "����", 16, BLACK, BLUE);                     // ��ʾ��ť������Ķ���-��ť��ɫ
                save_pos_zero(&hcan1,motor[Motor4].id, motor[Motor4].ctrl.mode);
                g_dm3910_3_target_pos = 0.0f;
                motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
                vTaskDelay(80);
                LCD_String(181, 289-8, "����", 16, BLACK, WHITE);                    // ���»��ư�ť-��ť�ָ�             
              }
            
            if (14<touch_x && touch_x<30 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // �������pos-��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_3_target_pos = g_dm3910_3_target_pos - 0.2*3591/187.0f;
              motor[Motor4].ctrl.pos_set = g_dm3910_3_target_pos;
              LCD_BOTTON_SUB(22, 289, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 289, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            
            if (43<touch_x && touch_x<58 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // �������pos+��ť��⣬����ı����target��pos��ʾ����real
              g_dm3910_3_target_pos = g_dm3910_3_target_pos + 0.2*3591/187.0f;
              motor[Motor4].ctrl.pos_set = g_dm3910_3_target_pos;
              LCD_BOTTON_ADD(50, 289, BLACK, BLUE);                                 // ��ʾ��ť������Ķ���-��ť��ɫ
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 289, BLACK, WHITE);                                // ���»��ư�ť-��ť�ָ�
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910�ؽڵ��3��>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<�ױ���<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 302<touch_y && touch_y<318 && g_machine_mode==0){ // ********���е���Ƿ񴴽���������ť���*******
              if (g_transmit_status == 0){                                         // ���������������
                g_transmit_status = 1;
                ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);
              }
              else if (g_transmit_status == 1){                                    // ɾ��������Ϳ�������
                g_transmit_status = 0;               
                vTaskDelete(xMotorTaskHandle);            
              }
              Judge_transmit(g_transmit_status, 181, 310-8); 
              }
            
            if (115<touch_x && touch_x<179 && 302<touch_y && touch_y<318){          // �ı��ƽ����Ŀ���״̬����Ļ���ƺ��ֱ����Ƶ��л�
                if (g_machine_mode == 0){                                           // �����ֱ�ģʽ
                  g_machine_mode = 1;
                  MX_USB_HOST_Init();                                               // �����ֱ�����
                }
                else if (g_machine_mode == 1){
                  g_machine_mode = 0;                                               // ���½�����Ļģʽ
                  vTaskDelete(hUsbHostFS.thread);                                   // ɾ������init�������ֱ�����
                }
                Judge_machine_mode(g_machine_mode, 115,310-8);             
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>�ױ���>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
            } 
          }
      if (XPT2046_IsPressed() == 0) g_handup = 1;                      //̧��״̬��⣬����
    }

}
/* USER CODE END FunctionPrototypes */
void StartDefaultTask(void const * argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* add threads, ... */
  ret = xTaskCreate(CommunicationTask, "CommunicationTask", 128, NULL, osPriorityNormal, &xCommunicationTaskHandle);  // ����ͨѶ���ԣ������ж�ϵͳ����û����
  ret = xTaskCreate(Touch_ManageTask, "Touch_ManageTask", 128, NULL, osPriorityNormal, &xTouch_ManageTaskHandle);   // ���ڼ�ⴥ���͹�������������������
  //ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);                      // �������������������ֻ��������������

}
