/******************************************************************************
 * 作  者： 燕山大学林楚然
 * 功  能： 实现屏幕交互和手柄交互两种方式控制水下矢量推进器运动
 * 参  考： 代码相关库参考魔女科技，轮趣科技，达妙科技，百问网
 * 版权归于林楚然，盗版必究
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
#include "stdio.h"               // 标准库
#include "bsp_UART.h"            // 串口驱动
#include "bsp_CAN.h"             // CAN驱动
#include "usbh_hid.h"            // USB中间件交互模式驱动
#include "bsp_ps2.h"             // PS2手柄驱动
#include "bsp_LCD_ILI9341.h"     // LCD驱动
#include "bsp_XPT2046.h"         // 触屏驱动
#include "bsp_W25Q128.h"         // 外部Flash, 存有触屏的校准数据
#include "dm_motor_ctrl.h"       // 达妙电机控制库
#include "dm_motor_drv.h"        // 达妙电机驱动库
#include "driver_timer.h"        // 高精度延时库

/*----------------------------------初始化全局变量----------------------------*/
uint8_t volatile g_machine_mode = 0;           // 推进器控制方式，默认0-屏幕控制，1为手柄控制
uint16_t volatile Rovmotor_PWM = 750;          // ROV电机全局PWM
uint8_t volatile g_rov_status = 0;             // ROV电机状态值，默认失能
uint8_t volatile g_dm4310_status = 0;          // DM4310关节电机状态，默认0为失能
uint8_t volatile g_dm3910_1_status = 0;        // DM3910_1关节电机状态，默认0为失能
uint8_t volatile g_dm3910_2_status = 0;        // DM3910_2关节电机状态，默认0为失能
uint8_t volatile g_dm3910_3_status = 0;        // DM3910_3关节电机状态，默认0为失能
uint8_t volatile g_transmit_status = 0;        // CAN总线电机是否发送状态，默认0为不发送

float volatile g_dm4310_target_pos = 0.0f;     // DM4310关节电机目标位置
float volatile g_dm3910_1_target_pos = 0.0f;   // DM3910关节电机1目标位置
float volatile g_dm3910_2_target_pos = 0.0f;   // DM3910关节电机2目标位置
float volatile g_dm3910_3_target_pos = 0.0f;   // DM3910关节电机3目标位置

double volatile g_dm4310_real_pos = 0.0f;      // DM3410关节电机实际位置
double volatile g_dm3910_1_real_pos = 0.0f;    // DM3910关节电机1实际位置
double volatile g_dm3910_2_real_pos = 0.0f;    // DM3910关节电机2实际位置
double volatile g_dm3910_3_real_pos = 0.0f;    // DM3910关节电机3实际位置

uint8_t volatile g_handup = 1;                 // 1表示抬手，0表示按下
uint16_t volatile g_start_tick = 0;            // 系统启动时间，计数未到达200(启动调度的2s内)回调函数不起作用。
/*----------------------------------初始化全局变量----------------------------*/

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
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);                // 引脚置高电平，红亮灭
  UART1_Init(115200);                                                             // 初始化串口1; 已写好底层，调用h中的函数即可使用; 引脚(PA9 +PA10)
  W25Q128_Init();                                                                 // 外部Flash 初始化，其内部存储有汉字字模数据
  LCD_Init();                                                                     // LCD ILI9341 初始化，开启一下背光
  LCD_SetDir(0);                                                                  // 设置显示方向：0-竖屏、1-横屏
  XPT2046_Init(xLCD.width, xLCD.height, xLCD.dir);                                // 触摸检测 初始化; 参数：屏宽、屏高、显示的方向
  /*---------------------------------------------初始化电调 PWM输出是PA3------------------------------------------------------*/
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,750); 
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
  g_rov_status = 1;                                                               // 螺旋桨电机自动使能
  HAL_Delay(2000);
  /*---------------------------------------------初始化电调 PWM输出是PA3------------------------------------------------------*/
  
  /*---------------------------------------------初始化各个电机 CAN通讯-----------------------------------------------------*/
  bsp_can_init();                                                                 // 初始化CAN1的筛选器; 为方便调试，本函数配置为：接收所有数据帧，包括标准帧、扩展帧
  dm_motor_init();                                                                // 初始化3个DM3519电机和一个DM4310电机                                                                  
  HAL_Delay(2-1);                                                                 // 给延时，防止数据错乱
  write_motor_data(motor[Motor1].id, 10, 1, 0, 0, 0);                             // 暂存DM4310电机为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  save_motor_data(motor[Motor1].id, 10);                                          // 写入DM4310电机为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  write_motor_data(motor[Motor2].id, 10, 1, 0, 0, 0);                             // 暂存DM3519电机1为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  save_motor_data(motor[Motor2].id, 10);                                          // 写入DM3519电机1为MIT模式，10为控制模式寄存器
	HAL_Delay(2-1);
  write_motor_data(motor[Motor3].id, 10, 1, 0, 0, 0);                             // 暂存DM3519电机2为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  save_motor_data(motor[Motor3].id, 10);                                          // 写入DM4310电机为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  write_motor_data(motor[Motor4].id, 10, 1, 0, 0, 0);                             // 暂存DM3519电机3为MIT模式，10为控制模式寄存器
  HAL_Delay(2-1);
  save_motor_data(motor[Motor4].id, 10);                                          // 写入DM3519电机1为MIT模式，10为控制模式寄存器
	HAL_Delay(10-1);
	dm_motor_enable(&hcan1, &motor[Motor1]);                                        // 自动使能DM4310电机，使用can1口发送数据
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor2]);                                        // 自动使能DM3519电机1，使用can1口发送数据
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor3]);                                        // 自动使能DM3519电机2，使用can1口发送数据
  HAL_Delay(10-1);
  dm_motor_enable(&hcan1, &motor[Motor4]);                                        // 自动使能DM3519电机3，使用can1口发送数据
  HAL_Delay(10-1);
  g_dm4310_status = 1;                                                            // 更新MD4310电机使能标志位
  g_dm3910_1_status = 1;                                                          // 更新DM3519电机1使能标志位
  g_dm3910_2_status = 1;                                                          // 更新DM3519电机2使能标志位
  g_dm3910_3_status = 1;                                                          // 更新DM3519电机3使能标志位
	HAL_Delay(200);
  /*---------------------------------------------初始化各个电机 CAN通讯-----------------------------------------------------*/
  
  /*---------------------------------------------配置FreeRTOS 开启调度-----------------------------------------------------*/
  MX_FREERTOS_Init();                                                             // 初始化freertos配置
  osKernelStart();                                                                // 开启调度
  while (1){}
  /*---------------------------------------------配置FreeRTOS 开启调度-----------------------------------------------------*/
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
 * 函  数： HAL_GPIO_EXTI_Callback
 * 功  能： EXTI中断回调函数
 * 参  数： uint16_t  GPIO_Pin    引脚编号，参数范围：GPIO_Pin_0 ~ GPIO_Pin_15
 * 返回值： 无
 * 备  注： 1：这个是回调函数，不是中断服务函数。完整的调用：EXTIx_IRQHandler() > HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x) > HAL_GPIO_EXTI_Callback(GPIO_PIN_x)
 *          2：必须使用这个函数名称，因为它在CubeMX生成时，已被写好了各种函数调用、函数弱定义(在stm32xx_hal_gpio.c的底部); 不建议在原弱定义中增添代码，而是重写本函数
 *          3：中断条件触发后，本函数被自动调用，无需进行人工调用; 也无需进行中断标志的清理，因为HAL_GPIO_EXTI_IRQHandler()函数中已有操作;
 *          4：生成的所有EXTI中断服务函数，都统一调用这个函数，以引脚编号作参数
 *          5：判断参数传进来的引脚编号，即可知道是哪个外部中断线产生的中断信号
******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_1_Pin)                                           // 按键1 产生的中断信号：KEY_1、PA0、闲时下拉、按下时高电平、上升沿触发
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // 反转红色LED灯
        printf("KEY_1 按下\r");                                          // 通过串口助手输出提示
        XPT2046_ReCalibration();                                         // 触摸屏重新校准
    }

    if (GPIO_Pin == KEY_2_Pin)                                           // 按键2 产生的中断信号：KEY_2、PA1、闲时上拉、按下时低电平、下降沿触发
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // 反转红色LED灯
        printf("KEY_2 按下\r");                                          // 通过串口助手输出提示
    }
    if (GPIO_Pin == KEY_3_Pin)                                           // 按键3 产生的中断信号：KEY_3、PA4、闲时上拉、按下低高电平、下降沿触发
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);              // 反转红色LED灯
        printf("KEY_3 按下\r");                                          // 通过串口助手输出提示
    }
}


extern USBH_StatusTypeDef USBH_HID_PS2_Decode(USBH_HandleTypeDef *phost);// PS2手柄解码函数
/******************************************************************************
 * 函  数： HAL_GPIO_EXTI_Callback
 * 功  能： USB HOST读取数据回调函数
 * 参  数： USBH_HandleTypeDef *phost
 * 返回值： 无
 * 备  注： 该函数是读取手柄信号的回调函数，那么USB读取的线程在MX_USB_HOST_Init函数中产生
 *          通过在回调函数中更新电机的值就可以做到手柄控制电机的效果
******************************************************************************/
void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
	USBH_HID_PS2_Decode(phost);                                                      // PS2数据数据解码
  char strTem[100];                                                                // 新建数组，用于存放PWM值得标签              
	extern PS2INFO_t ps2_info;                                                       // 获取解码后的ps2手柄信息
	PS2INFO_t* ps2 = &ps2_info;
	PS2KEY_State_t keyEvent;                                                         // 定义ps2手柄按键事件变量,用于获取按键的事件
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<总线电机判断区<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/  
  keyEvent = ps2->getKeyEvent(PS2KEY_L1);                                          // 检测按键PS2KEY_L1发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){
    motor[Motor1].ctrl.vel_set 	= 3.0f;                                            // 给前馈速度
    g_dm4310_target_pos = g_dm4310_target_pos + 0.2f;                              // 更新motor[Motor1]结构体中发送给电机的位置
    motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_L2);                                          // 检测按键PS2KEY_L2发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    motor[Motor1].ctrl.vel_set 	= -3.0f;                                           // 给前馈速度
    g_dm4310_target_pos = g_dm4310_target_pos - 0.2f;                              // 更新motor[Motor1]结构体中发送给电机的位置
    motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_UP);                                          // 检测按键PS2KEY_UP发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_3_target_pos = g_dm3910_3_target_pos + 0.2*3591/187.0f;               // 更新motor[Motor4]结构体中发送给电机的位置
    motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_DOWN);                                        // 检测按键PS2KEY_DOWN发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_3_target_pos = g_dm3910_3_target_pos - 0.2*3591/187.0f;               // 更新motor[Motor4]结构体中发送给电机的位置
    motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_LEFT);                                        // 检测按键PS2KEY_UP发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_2_target_pos = g_dm3910_2_target_pos + 0.2*3591/187.0f;               // 更新motor[Motor3]结构体中发送给电机的位置
    motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_RIGHT);                                       // 检测按键PS2KEY_DOWN发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_2_target_pos = g_dm3910_2_target_pos - 0.2*3591/187.0f;               // 更新motor[Motor3]结构体中发送给电机的位置
    motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_4PINK);                                       // 检测按键PS2KEY_UP发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_1_target_pos = g_dm3910_1_target_pos + 0.2*3591/187.0f;               // 更新motor[Motor2]结构体中发送给电机的位置
    motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_2RED);                                        // 检测按键PS2KEY_DOWN发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    g_dm3910_1_target_pos = g_dm3910_1_target_pos - 0.2*3591/187.0f;               // 更新motor[Motor2]结构体中发送给电机的位置
    motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
  }
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>总线电机判断区>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/  

  keyEvent = ps2->getKeyEvent(PS2KEY_R1);                                          // 检测按键PS2KEY_R1发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    Rovmotor_PWM = Rovmotor_PWM + 50;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                     // 更新PA3接口的PWM值
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
    else LCD_String(181,105-8,"复位", 16, WHITE, RED);
    sprintf(strTem, "%d", Rovmotor_PWM); 
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);
  }
  
  keyEvent = ps2->getKeyEvent(PS2KEY_R2);                                          // 检测按键PS2KEY_R2发生的事件(单击或双击)
  if (keyEvent == PS2KEYSTATE_SINGLECLICK || keyEvent == PS2KEYSTATE_DOUBLECLICK){ 
    Rovmotor_PWM = Rovmotor_PWM - 50;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                     // 更新PA3接口的PWM值
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                     // 用户界面更新
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
    else LCD_String(181,105-8,"复位", 16, WHITE, RED);
    sprintf(strTem, "%d", Rovmotor_PWM); 
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);
  }
}


/******************************************************************************
 * 函  数： HAL_TIM_PeriodElapsedCallback
 * 功  能： 周期更新回调函数
 * 备  注： 本函数是TIM的CNT溢出中断回调函数，这里的周期定时为20ms。
 *          当TIM的计数器CNT，完成1周期计数时触发(向上计数：CNT==ARR、向下计数：CNT==0);
 *          上述中断触发后，硬件自动调用相关中断服务函数，继而调用本函数。
 *          所有TIM的周期更新中断，都是调用本函数，因此需要在函数内判断是哪一个TIM触发的中断;
 * 参  数： TIM_HandleTypeDef   *htim
 * 返回值： 无
******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {                                      // 判断是哪个TIM产生的中断，TIM1是系统时钟
    HAL_IncTick();
  }
  if (htim->Instance == TIM6)                                        // 判断是哪个TIM产生的中断，TIM6是20ms的定时器
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
