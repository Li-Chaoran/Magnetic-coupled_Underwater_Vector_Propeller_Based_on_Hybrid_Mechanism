/******************************************************************************
 * ��  �ߣ� ��ɽ��ѧ�ֳ�Ȼ
 * ��  �ܣ� ʵ����Ļ�������ֱ��������ַ�ʽ����ˮ��ʸ���ƽ����˶�
 * ��  ���� ������ؿ�ο�ħŮ�Ƽ�����Ȥ�Ƽ�������Ƽ���������
 * ��Ȩ�����ֳ�Ȼ������ؾ�
******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include "stdio.h"               // ��׼��
#include "bsp_UART.h"            // ��������
#include "bsp_CAN.h"             // CAN����
#include "usbh_hid.h"            // USB�м������ģʽ����
#include "bsp_ps2.h"             // PS2�ֱ�����
#include "bsp_LCD_ILI9341.h"     // LCD����
#include "bsp_XPT2046.h"         // ��������
#include "bsp_W25Q128.h"         // �ⲿFlash, ���д�����У׼����
#include "dm_motor_ctrl.h"       // ���������ƿ�
#include "dm_motor_drv.h"        // ������������
#include "driver_timer.h"        // �߾�����ʱ��

/*----------------------------------��ʼ��ȫ�ֱ���----------------------------*/
uint8_t volatile g_machine_mode = 0;           // �ƽ������Ʒ�ʽ��Ĭ��0-��Ļ���ƣ�1Ϊ�ֱ�����
uint16_t volatile Rovmotor_PWM = 750;          // ROV���ȫ��PWM
uint8_t volatile g_rov_status = 0;             // ROV���״ֵ̬��Ĭ��ʧ��
uint8_t volatile g_dm4310_status = 0;          // DM4310�ؽڵ��״̬��Ĭ��0Ϊʧ��
uint8_t volatile g_dm3910_1_status = 0;        // DM3910_1�ؽڵ��״̬��Ĭ��0Ϊʧ��
uint8_t volatile g_dm3910_2_status = 0;        // DM3910_2�ؽڵ��״̬��Ĭ��0Ϊʧ��
uint8_t volatile g_dm3910_3_status = 0;        // DM3910_3�ؽڵ��״̬��Ĭ��0Ϊʧ��
uint8_t volatile g_transmit_status = 0;        // CAN���ߵ���Ƿ���״̬��Ĭ��0Ϊ������

float volatile g_dm4310_target_pos = 0.0f;     // DM4310�ؽڵ��Ŀ��λ��
float volatile g_dm3910_1_target_pos = 0.0f;   // DM3910�ؽڵ��1Ŀ��λ��
float volatile g_dm3910_2_target_pos = 0.0f;   // DM3910�ؽڵ��2Ŀ��λ��
float volatile g_dm3910_3_target_pos = 0.0f;   // DM3910�ؽڵ��3Ŀ��λ��

double volatile g_dm4310_real_pos = 0.0f;      // DM3410�ؽڵ��ʵ��λ��
double volatile g_dm3910_1_real_pos = 0.0f;    // DM3910�ؽڵ��1ʵ��λ��
double volatile g_dm3910_2_real_pos = 0.0f;    // DM3910�ؽڵ��2ʵ��λ��
double volatile g_dm3910_3_real_pos = 0.0f;    // DM3910�ؽڵ��3ʵ��λ��

uint8_t volatile g_handup = 1;                 // 1��ʾ̧�֣�0��ʾ����
uint16_t volatile g_start_tick = 0;            // ϵͳ����ʱ�䣬����δ����200(�������ȵ�2s��)�ص������������á�
/*----------------------------------��ʼ��ȫ�ֱ���----------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_CAN1_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  
  /* Initialize user codes */
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);                // �����øߵ�ƽ��������
  UART1_Init(115200);                                                             // ��ʼ������1; ��д�õײ㣬����h�еĺ�������ʹ��; ����(PA9 +PA10)
  W25Q128_Init();                                                                 // �ⲿFlash ��ʼ�������ڲ��洢�к�����ģ����
  LCD_Init();                                                                     // LCD ILI9341 ��ʼ��������һ�±���
  LCD_SetDir(0);                                                                  // ������ʾ����0-������1-����
  XPT2046_Init(xLCD.width, xLCD.height, xLCD.dir);                                // ������� ��ʼ��; �������������ߡ���ʾ�ķ���
  /*---------------------------------------------��ʼ����� PWM�����PA3------------------------------------------------------*/
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,750); 
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
  g_rov_status = 1;                                                               // ����������Զ�ʹ��
  HAL_Delay(2000);
  /*---------------------------------------------��ʼ����� PWM�����PA3------------------------------------------------------*/
  
  /*---------------------------------------------��ʼ��������� CANͨѶ-----------------------------------------------------*/
  bsp_can_init();                                                                 // ��ʼ��CAN1��ɸѡ��; Ϊ������ԣ�����������Ϊ��������������֡��������׼֡����չ֡
  dm_motor_init();                                                                // ��ʼ��3��DM3519�����һ��DM4310���                                                                  
  HAL_Delay(2-1);                                                                 // ����ʱ����ֹ���ݴ���
  write_motor_data(motor[Motor1].id, 10, 1, 0, 0, 0);                             // �ݴ�DM4310���ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  save_motor_data(motor[Motor1].id, 10);                                          // д��DM4310���ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  write_motor_data(motor[Motor2].id, 10, 1, 0, 0, 0);                             // �ݴ�DM3519���1ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  save_motor_data(motor[Motor2].id, 10);                                          // д��DM3519���1ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
	HAL_Delay(2-1);
  write_motor_data(motor[Motor3].id, 10, 1, 0, 0, 0);                             // �ݴ�DM3519���2ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  save_motor_data(motor[Motor3].id, 10);                                          // д��DM4310���ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  write_motor_data(motor[Motor4].id, 10, 1, 0, 0, 0);                             // �ݴ�DM3519���3ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
  HAL_Delay(2-1);
  save_motor_data(motor[Motor4].id, 10);                                          // д��DM3519���1ΪMITģʽ��10Ϊ����ģʽ�Ĵ���
	HAL_Delay(10-1);
	dm_motor_enable(&hcan1, &motor[Motor1]);                                        // �Զ�ʹ��DM4310�����ʹ��can1�ڷ�������
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor2]);                                        // �Զ�ʹ��DM3519���1��ʹ��can1�ڷ�������
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor3]);                                        // �Զ�ʹ��DM3519���2��ʹ��can1�ڷ�������
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor4]);                                        // �Զ�ʹ��DM3519���3��ʹ��can1�ڷ�������
  HAL_Delay(10-1);
  g_dm4310_status = 1;                                                            // ����MD4310���ʹ�ܱ�־λ
  g_dm3910_1_status = 1;                                                          // ����DM3519���1ʹ�ܱ�־λ
  g_dm3910_2_status = 1;                                                          // ����DM3519���2ʹ�ܱ�־λ
  g_dm3910_3_status = 1;                                                          // ����DM3519���3ʹ�ܱ�־λ
	HAL_Delay(200);
  /*---------------------------------------------��ʼ��������� CANͨѶ-----------------------------------------------------*/
  
  /*---------------------------------------------����FreeRTOS ��������-----------------------------------------------------*/
  MX_FREERTOS_Init();                                                             // ��ʼ��freertos����
  osKernelStart();                                                                // ��������
  while (1){}
  /*---------------------------------------------����FreeRTOS ��������-----------------------------------------------------*/
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/******************************************************************************
 * ��  ���� HAL_GPIO_EXTI_Callback
 * ��  �ܣ� EXTI�жϻص�����
 * ��  ���� uint16_t  GPIO_Pin    ���ű�ţ�������Χ��GPIO_Pin_0 ~ GPIO_Pin_15
 * ����ֵ�� ��
 * ��  ע�� 1������ǻص������������жϷ������������ĵ��ã�EXTIx_IRQHandler() > HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x) > HAL_GPIO_EXTI_Callback(GPIO_PIN_x)
 *          2������ʹ������������ƣ���Ϊ����CubeMX����ʱ���ѱ�д���˸��ֺ������á�����������(��stm32xx_hal_gpio.c�ĵײ�); ��������ԭ��������������룬������д������
 *          3���ж����������󣬱��������Զ����ã���������˹�����; Ҳ��������жϱ�־��������ΪHAL_GPIO_EXTI_IRQHandler()���������в���;
 *          4�����ɵ�����EXTI�жϷ���������ͳһ������������������ű��������
 *          5���жϲ��������������ű�ţ�����֪�����ĸ��ⲿ�ж��߲������ж��ź�
******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_1_Pin)                                           // ����1 �������ж��źţ�KEY_1��PA0����ʱ����������ʱ�ߵ�ƽ�������ش���
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // ��ת��ɫLED��
        printf("KEY_1 ����\r");                                          // ͨ���������������ʾ
        XPT2046_ReCalibration();                                         // ����������У׼
    }

    if (GPIO_Pin == KEY_2_Pin)                                           // ����2 �������ж��źţ�KEY_2��PA1����ʱ����������ʱ�͵�ƽ���½��ش���
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // ��ת��ɫLED��
        printf("KEY_2 ����\r");                                          // ͨ���������������ʾ
    }
    if (GPIO_Pin == KEY_3_Pin)                                           // ����3 �������ж��źţ�KEY_3��PA4����ʱ���������µ͸ߵ�ƽ���½��ش���
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // ��ת��ɫLED��
        printf("KEY_3 ����\r");                                          // ͨ���������������ʾ
    }
}


extern USBH_StatusTypeDef USBH_HID_PS2_Decode(USBH_HandleTypeDef *phost);// PS2�ֱ����뺯��
/******************************************************************************
 * ��  ���� HAL_GPIO_EXTI_Callback
 * ��  �ܣ� USB HOST��ȡ���ݻص�����
 * ��  ���� USBH_HandleTypeDef *phost
 * ����ֵ�� ��
 * ��  ע�� �ú����Ƕ�ȡ�ֱ��źŵĻص���������ôUSB��ȡ���߳���MX_USB_HOST_Init�����в���
 *          ͨ���ڻص������и��µ����ֵ�Ϳ��������ֱ����Ƶ����Ч��
******************************************************************************/
void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
	USBH_HID_PS2_Decode(phost);                                                      // PS2�������ݽ���
  char strTem[100];                                                                // �½����飬���ڴ��PWMֵ�ñ�ǩ              
	extern PS2INFO_t ps2_info;                                                       // ��ȡ������ps2�ֱ���Ϣ
	PS2INFO_t* ps2 = &ps2_info;
	PS2KEY_State_t keyEvent;                                                         // ����ps2�ֱ������¼�����,���ڻ�ȡ�������¼�
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<���ߵ���ж���<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/  
  keyEvent = ps2->getKeyEvent(PS2KEY_L1);                                          // ��ⰴ��PS2KEY_L1�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){
    motor[Motor1].ctrl.vel_set 	= 3.0f;                                            // ��ǰ���ٶ�
    g_dm4310_target_pos = g_dm4310_target_pos + 0.2f;                              // ����motor[Motor1]�ṹ���з��͸������λ��
    motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_L2);                                          // ��ⰴ��PS2KEY_L2�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    motor[Motor1].ctrl.vel_set 	= -3.0f;                                           // ��ǰ���ٶ�
    g_dm4310_target_pos = g_dm4310_target_pos - 0.2f;                              // ����motor[Motor1]�ṹ���з��͸������λ��
    motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_UP);                                          // ��ⰴ��PS2KEY_UP�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_3_target_pos = g_dm3910_3_target_pos + 0.2*3591/187.0f;               // ����motor[Motor4]�ṹ���з��͸������λ��
    motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_DOWN);                                        // ��ⰴ��PS2KEY_DOWN�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_3_target_pos = g_dm3910_3_target_pos - 0.2*3591/187.0f;               // ����motor[Motor4]�ṹ���з��͸������λ��
    motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_LEFT);                                        // ��ⰴ��PS2KEY_UP�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_2_target_pos = g_dm3910_2_target_pos + 0.2*3591/187.0f;               // ����motor[Motor3]�ṹ���з��͸������λ��
    motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_RIGHT);                                       // ��ⰴ��PS2KEY_DOWN�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_2_target_pos = g_dm3910_2_target_pos - 0.2*3591/187.0f;               // ����motor[Motor3]�ṹ���з��͸������λ��
    motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_4PINK);                                       // ��ⰴ��PS2KEY_UP�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_1_target_pos = g_dm3910_1_target_pos + 0.2*3591/187.0f;               // ����motor[Motor2]�ṹ���з��͸������λ��
    motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_2RED);                                        // ��ⰴ��PS2KEY_DOWN�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_1_target_pos = g_dm3910_1_target_pos - 0.2*3591/187.0f;               // ����motor[Motor2]�ṹ���з��͸������λ��
    motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
  }
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>���ߵ���ж���>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/  

  keyEvent = ps2->getKeyEvent(PS2KEY_R1);                                          // ��ⰴ��PS2KEY_R1�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    Rovmotor_PWM = Rovmotor_PWM + 50;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                     // ����PA3�ӿڵ�PWMֵ
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);
    else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
    sprintf(strTem, "%d", Rovmotor_PWM); 
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_R2);                                          // ��ⰴ��PS2KEY_R2�������¼�(������˫��)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    Rovmotor_PWM = Rovmotor_PWM - 50;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                     // ����PA3�ӿڵ�PWMֵ
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                     // �û��������
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"��λ", 16, BLACK, GREY);
    else LCD_String(181,105-8,"��λ", 16, WHITE, RED);
    sprintf(strTem, "%d", Rovmotor_PWM); 
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);
  }
}


/******************************************************************************
 * ��  ���� HAL_TIM_PeriodElapsedCallback
 * ��  �ܣ� ���ڸ��»ص�����
 * ��  ע�� ��������TIM��CNT����жϻص���������������ڶ�ʱΪ20ms��
 *          ��TIM�ļ�����CNT�����1���ڼ���ʱ����(���ϼ�����CNT==ARR�����¼�����CNT==0);
 *          �����жϴ�����Ӳ���Զ���������жϷ��������̶����ñ�������
 *          ����TIM�����ڸ����жϣ����ǵ��ñ������������Ҫ�ں������ж�����һ��TIM�������ж�;
 * ��  ���� TIM_HandleTypeDef   *htim
 * ����ֵ�� ��
******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {                                      // �ж����ĸ�TIM�������жϣ�TIM1��ϵͳʱ��
    HAL_IncTick();
  }
  if (htim->Instance == TIM6)                                        // �ж����ĸ�TIM�������жϣ�TIM6��20ms�Ķ�ʱ��
  {
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
