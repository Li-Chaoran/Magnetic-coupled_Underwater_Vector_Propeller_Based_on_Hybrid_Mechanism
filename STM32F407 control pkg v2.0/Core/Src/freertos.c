/******************************************************************************
 * 作  者： 燕山大学林楚然
 * 功  能： 实现屏幕交互和手柄交互两种方式控制水下矢量推进器运动
 * 参  考： 代码相关库参考魔女科技，轮趣科技，达妙科技，百问网
 * 版权归于林楚然，盗版必究
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
#include "IKRES.h"
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
extern void MX_USB_HOST_Process(void);

/* 声明任务句柄 */
TaskHandle_t xCommunicationTaskHandle = NULL;
static TaskHandle_t xTouch_ManageTaskHandle[10] = {0};               // 页面一句柄组
static TaskHandle_t xIkTouchManageTaskHandle[10] = {0};              // 页面二句柄组
static uint8_t xTouch_ManageTaskHandleCount = 0;                     // 页面一句柄下标
static uint8_t xIkTouchManageTaskHandleCount = 0;                    // 页面二句柄下标
TaskHandle_t xMotorTaskHandle = NULL;                                // 电机任务句柄
BaseType_t ret;                                                      // 接收判断任务创建是否成功的变量
osThreadId defaultTaskHandle;                                        // 默认任务句柄
void Touch_ManageTask(void *params);                                 // 声明任务函数，防止编译器报错


/* Private Reusable function -----------------------------------------------*/
/******************************************************************************
 * 函  数： ReflashIk2Motor
 * 功  能： 用于更新逆运动学数据到电机
 * 参  数： 无
 * 返回值： 无
 * 备  注： 调用该函数就可以将逆运动学数据更新到电机目标位置
******************************************************************************/
void ReflashIk2Motor(int8_t x, int8_t y){
  g_dm3910_1_target_pos = -(float)(ik_solution_array[1][(x+30)*61+30+y]*2.5f*3.591f/187.0f);   //需要注意的是这里的yingshe_y要反转正负号，因为绕着x轴转动逆时针为正,同时需要乘齿数比
  g_dm3910_2_target_pos = -(float)(ik_solution_array[2][(x+30)*61+30+y]*2.5f*3.591f/187.0f);
  g_dm3910_3_target_pos = -(float)(ik_solution_array[0][(x+30)*61+30+y]*2.5f*3.591f/187.0f);
//  g_dm3910_1_target_pos = (float)(ik_solution_array[2][(x+30)*61+30-y]);   // 单纯读取数组
//  g_dm3910_2_target_pos = (float)(ik_solution_array[0][(x+30)*61+30-y]);
//  g_dm3910_3_target_pos = (float)(ik_solution_array[1][(x+30)*61+30-y]);
  motor[Motor2].ctrl.pos_set = g_dm3910_1_target_pos;
  motor[Motor3].ctrl.pos_set = g_dm3910_2_target_pos;
  motor[Motor4].ctrl.pos_set = g_dm3910_3_target_pos;
  //调试屏幕显示部分
//  char poss[60] = {0};
//  sprintf(poss, "1:%3d 2:%3d 3:%3d", (int)g_dm3910_1_target_pos, (int)g_dm3910_2_target_pos,(int)g_dm3910_3_target_pos);  // 格式化坐标字符串
//  LCD_String(8, 170, poss, 12, WHITE, BLACK);                       // 显示坐标字符串
};

/* Private function prototypes -----------------------------------------------*/
/******************************************************************************
 * 函  数： CommunicationTask
 * 功  能： 通讯(USART+CAN)与闪灯任务
 * 参  数： 无
 * 返回值： 无
 * 备  注： 只需要解开后面的注释就可以进行通讯调试
******************************************************************************/
void CommunicationTask(void *params){
  while(1){
        /** 10ms延时,释放控制权 **/        
        vTaskDelay(10);                                                                                              
//        /** 检查UART1是否收到数据 (串口助手) **/                                                        
//        if (UART1_GetRxNum())                                                                            // 判断UART1是否获取到新一帧数据：函数返回的接收字节数，大于0，即为收到新一帧数据
//        {                                                                                               
//            uint8_t *rxData = UART1_GetRxData();                                                         // 获取数据的地址
//            uint16_t rxNum = UART1_GetRxNum();                                                           // 获取数据的字节数
//                                                                                                        
//            printf("\r<<<<< USART1 收到一帧数据 \r");                                                    // 提示
//            printf("字节数：%d \r", rxNum);                                                              // 显示字节数
//            printf("ASCII : %s\r", (char *)rxData);                                                      // 显示数据，以ASCII方式显示，即以字符串的方式显示
//            printf("16进制: ");                                                                          // 显示数据，以16进制方式，显示每一个字节的值
//            for (uint16_t i = 0; i < rxNum; i++)                                                         // 逐个字节输出
//                printf("0x%X ", rxData[i]);                                                              // 以16进制显示
//            printf("\r\r");                                                                              // 显示换行
//                                                                                                        
//            // UART1_SendString("Hello World！\r");                                                      // 本行示范 UART1_SendString( )的使用; 可接受像pritnf的格式化参数
//            // CAN1_SendData(rxData, 8);                                                                    // 本行示范 CAN1_SendData( )的使用, 可用于发送字符串、数组、结构体数据等
//            UART1_ClearRx();                                                                             // 重要：清0接收标志，即清0接收到的字节数; 每次处理完成数据，就要调用这个函数清0
//        }
//        /** 检查CAN1是否收到数据 **/
//        if (xCAN1.RxNum)                                                                                 // 接收标志位、字节数; 检查此值是否大于0，以判断是否接收到新一帧数据
//        {
//            static uint16_t  canRxCNT = 1;                                                               // 用于计算已接收了多少帧数据; 非必要;
//            printf("\r****** CAN 接收到第%d帧新数据 ******", canRxCNT++);                                // 准备把CAN帧报文，详细地输出到串口软件，方便观察调试
//            printf("\r 帧类型：%s",   xCAN1.RxHeader.RTR ? "遥控帧" : "数据帧");                         // 帧类型
//            printf("\r 帧格式：%s",   xCAN1.RxHeader.IDE ? "扩展帧" : "标准帧");                         // 帧格式
//            printf("\r 标识符：0x%X", xCAN1.RxHeader.IDE ? xCAN1.RxHeader.ExtId : xCAN1.RxHeader.StdId); // 帧ID
//            printf("\r 字节数：%d",   xCAN1.RxHeader.DLC);                                               // 字节数
//            printf("\r 筛选器匹配序号：%d", xCAN1.RxHeader.FilterMatchIndex);                            // 筛选器匹配序号; 和筛选器编号，是不一样的。大概：从筛选器0开始，每个16位宽筛选器+2, 32位宽+1, 没有被使用的筛选器，默认是16位宽，+2;
//            printf("\r 显示数据(16进制)：");                                                             // 16进制方式显示数据，方便观察真实数据
//            for (uint8_t i = 0; i < xCAN1.RxNum; i++)                                                    // 逐个字节显示CAN的数据值
//                printf(" 0x%X ", xCAN1.RxData[i]);                                                       // 格式：16进制显示
//            printf("\r");                                                                                // 换行，以使每帧的输出更清晰

//            xCAN1.RxNum = 0;                                                                             // 字节数清0; 每次处理完一帧数据，需要将xCAN1.RxNum变量清0，即：xCAN1.RxNum=0，以方便下一轮的判断，避免重复处理同一帧数据;

//            if (strstr((char *)xCAN1.RxData, "CAN_Test"))                                                // 判断是否测试工具发过来的指定数据
//            {
//                static char strTemp[8] = "CAN_OK";                                                       // 测试工具约定好的返回数据
//                canx_send_data(&hcan1, CAN_TX_ID, (uint8_t *)strTemp, 8);                                // 发送CAN报文
//            }
//        }
//        /** 每500ms发送一帧CAN报文，方便CAN总线上的其它设备测试接收 **/
//        static uint16_t msCAN = 0;                                                                   
//        if (++msCAN == 500)                                                                              // 每500ms执行一次
//        {                                                                                             
//            msCAN = 0;                                                                                   // 计数清0
//            static uint8_t msgData[30] = { 0, 2, 4, 6, 8, 10, 12, 14 };                                  // 定义一个简单的数组，用于CAN的发送示范; 这里开辟了30字节的空间，暂时只赋值了前8个字节，后22个字节值为0;
//            canx_send_data(&hcan1, CAN_TX_ID, msgData, 6);                                               // CAN1发送数据(自定义函数)，每500ms一次，以方便外部设备测试接收; CAN每帧最大8字节，这里只发6字节是为了示范：CAN发送的字节数与数组定义的大小，没一毛钱关系。
//        }                                                                                                                                                                                                   
        /** 每500ms闪烁一次蓝色LED, 以方便外部观察运行情况 **/                                        
        static uint16_t msLED = 0;
        if(g_start_tick<200)g_start_tick++;                                                             // g_start_tick小于200，则不读取can反馈的数据
        if (++msLED == 50)                                                                              // 每500ms执行一次
        {                                                                                             
            msLED = 0;                                                                                   // 计数清0
            HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);                                        // 规律地闪烁蓝色LED，方便外部观察系统是否正常运行
        }
    }
};


/******************************************************************************
 * 函  数： Motor_Task
 * 功  能： 电机控制任务
 * 参  数： 无
 * 返回值： 无
 * 备  注： 使用库函数周期性的发送MIT模式控制帧
******************************************************************************/
void Motor_Task(void  *params){
  TickType_t pretime;
  pretime = xTaskGetTickCount();
  while(1){
    dm_motor_ctrl_send(&hcan1, &motor[Motor1]);  // 周期发送电机位置控制
    dm_motor_ctrl_send(&hcan1, &motor[Motor2]);
    vTaskDelay(1);                               // 防止丢帧
    dm_motor_ctrl_send(&hcan1, &motor[Motor3]);
    dm_motor_ctrl_send(&hcan1, &motor[Motor4]);
    vTaskDelayUntil(&pretime, 2);                // 以500HZ周期发布位置控制
  }
};

/******************************************************************************
 * 函  数： ik_Touch_ManageTask
 * 功  能： 页面二及逆运动学解算任务
 * 参  数： 无
 * 返回值： 无
 * 备  注： 自行阅读代码
******************************************************************************/
void ik_Touch_ManageTask(void *params){
    vTaskDelay(10);
    g_yemian_num = 2;                                                      // 更新页面标志位
    vTaskDelete(xTouch_ManageTaskHandle[xTouch_ManageTaskHandleCount-1]);  // 杀死页面一
    // 初始化的变量
    int8_t volatile yingshe_x = 0;                                          // 映射x，映射为（-30，30）之间
    int8_t volatile yingshe_y = 0;                                          // 映射y，映射为（-30，30）之间
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;
    char effectpos[20] = {0};                                               // 存放动角台的xy位置
//    uint16_t num = 0;
    uint8_t status = 0;                                            // 用于判断触摸是否处于长按状态，0-不处于长按，1-处于长按，其可以直接跳过触摸检测操作按钮
//    char strnumTem[20]= {0};
    static char str[20] = {0};                                     // 新建数组，用于存放坐标字符串
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<页面二顶栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    LCD_Fill(0, 0, 240, 320, BLACK);                                                 // 整个背景填充颜色
    LCD_String(10, 8, "水 下 矢 量 推 进", 24, WHITE, BLACK);                        // 显示标题;
//    LCD_String(58, 40, "控 制 面 板", 24, YELLOW, BLACK);                            // 显示标题;
    LCD_String(10, 40, "逆 运 动 学 控 制", 24, YELLOW, BLACK);                        // 显示标题;
    LCD_Line(0, 37, 240, 37, GRAY);                                                  // 画个横线    
    LCD_Line(0, 38, 240, 38, GRAY);                                                  // 画个横线，加粗
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>页面二顶栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
  
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<螺旋桨电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 72, "螺旋桨电机状态：", 16, WHITE, BLACK); 
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                       // 判断pwm的值并显示电机状态为停止，正转和反转
    LCD_Rectangle(12, 70, 176, 90, GRAY);                                            // 绘制矩形框
    //按钮-显示
    LCD_BOTTON_SUB(22, 105, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
    //PWM标签显示
    LCD_String(80, 105-8, "PWM:", 16, WHITE, BLACK);
    //使能和强停按钮显示
    Judge_enable(g_rov_status, 181,72);                                             // 判断status的值并显示电机是否使能
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);        // 判断pwm的值并显示电机是否复位
    else LCD_String(181,105-8,"复位", 16, WHITE, RED);
    //pwm数值标签显示
    char strTem[100];                                                           
    sprintf(strTem, "%d", Rovmotor_PWM);                                          // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                            
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>螺旋桨电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<动平台运动圆圈<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    LCD_Circle(120, 200, 60, RED);                                               // 运动边界
    LCD_Circle(120, 200, 61, RED);
    LCD_Circle(120, 200, 62, MAGENTA);
    LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
    LCD_BOTTON_SUB(44, 200, BLACK, WHITE);                                      // ←
    LCD_BOTTON_SUB(120, 124, BLACK, WHITE);                                     // ↑
    LCD_BOTTON_ADD(196, 200, BLACK, WHITE);                                     // →
    LCD_BOTTON_ADD(120, 276, BLACK, WHITE);                                     // ↓
    
    LCD_BOTTON_SUB(62, 142, BLACK, WHITE);                                      // ↖
    LCD_BOTTON_SUB(62, 258, BLACK, WHITE);                                      // ↙
    LCD_BOTTON_ADD(178, 142, BLACK, WHITE);                                     // ↗
    LCD_BOTTON_ADD(178, 258, BLACK, WHITE);                                     // ↘
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>动平台运动圆圈>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<底边栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    Judge_transmit(g_transmit_status, 181, 310-8);                                   // 判断是否创建CAN发送任务按钮
    LCD_String(15, 310-8, "页面一", 16, BLACK, GRAY);                                // 创建一个页面一的任务，同时删除页面二自身的任务
    LCD_String(65, 310-8, "页面二", 16, WHITE, GREEN);                               // 创建一个页面二的任务，同时删除页面一自身的任务
    Judge_machine_mode(g_machine_mode, 115,310-8);                                   // 显示当前的控制模式，按下可以切换
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>底边栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    while (1){
      /** 1ms延时 **/
      vTaskDelay(1);                                                  
      /** 检查触摸屏是否按下，并进行相关处理 **/
      if ((XPT2046_IsPressed() && g_handup == 1) || status == 1)           // 检查触摸屏是否从抬手状态按下; 返回：0-未按下、1-按下
      {
        vTaskDelay(10);                                                  //rtos中改成vtaskdelay就行，就不用定时器再定时了
        if (XPT2046_IsPressed() || status == 1){
          g_handup = 0;                                                  // 确认触摸按下
          touch_x = XPT2046_GetX();                                      // 获取触摸点x位置
          touch_y = XPT2046_GetY();                                      // 获取触摸点y位置
          LCD_DrawPoint(touch_x, touch_y, GREEN);                        // 在按下的位置，画点; 用于测试
          sprintf(str, "X:%3d  Y:%3d", touch_x, touch_y);                // 格式化坐标字符串
          LCD_String(8, 5, str, 12, WHITE, BLACK);                       // 显示坐标字符串
          //num += 1;                                                    // 测试用num
          //sprintf(strnumTem, "%d", num);
          //LCD_String(120, 130-8, strnumTem, 16, WHITE, BLACK); 
          /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<螺旋桨电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
          if (181<touch_x && touch_x<213 && 72<touch_y && touch_y<88 && g_machine_mode==0){   // 螺旋桨电机使能按钮检测，开启或关闭
            if (g_rov_status == 0){g_rov_status = 1;HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);}
            else if (g_rov_status == 1) {g_rov_status = 0;HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_4);}
            Judge_enable(g_rov_status, 181,72);                                               // 判断status的值并显示电机是否使能
            }
          
          if (181<touch_x && touch_x<213 && 97<touch_y && touch_y<113 && g_machine_mode==0){  // 螺旋桨电机复位按钮检测
            Rovmotor_PWM = 750;
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
            sprintf(strTem, "%d", Rovmotor_PWM); 
            LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
            if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
            else LCD_String(181,105-8,"复位", 16, WHITE, RED);
            Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                        // 判断pwm的值并显示电机状态为停止，正转和反转
            }
          
          if (14<touch_x && touch_x<30 && 97<touch_y && touch_y<113 && g_machine_mode==0){    // 螺旋桨电机pwm-按钮检测
            Rovmotor_PWM = Rovmotor_PWM - 5;
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
            LCD_BOTTON_SUB(22, 105, BLACK, BLUE);                                             // 显示按钮被点击的动画-按钮变色
            Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
            if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
            else LCD_String(181,105-8,"复位", 16, WHITE, RED);
            sprintf(strTem, "%d", Rovmotor_PWM); 
            LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                                 // 更改显示的PWM值
            vTaskDelay(50);
            LCD_BOTTON_SUB(22, 105, BLACK, WHITE);                                            // 重新绘制按钮-按钮恢复
            }
          
          if (42<touch_x && touch_x<58 && 97<touch_y && touch_y<113 && g_machine_mode==0){    //螺旋桨电机pwm+按钮检测
            Rovmotor_PWM = Rovmotor_PWM + 5;
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
            LCD_BOTTON_ADD(50, 105, BLACK, BLUE);
            Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
            if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
            else LCD_String(181,105-8,"复位", 16, WHITE, RED);
            sprintf(strTem, "%d", Rovmotor_PWM); 
            LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
            vTaskDelay(50);
            LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
            }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>螺旋桨电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
          
          /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<动平台运动圆圈<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
          if (36<touch_x && touch_x<52 && 192<touch_y && touch_y<208 && g_machine_mode==0){   // 减小x方向的角度
            if (abs(yingshe_x-1) <= 30 && abs(yingshe_y)<=30){                                  // 大角度软件限位，防止出现意外
//            if ((float)(sqrt((yingshe_x-1)*(yingshe_x-1)+yingshe_y*yingshe_y))<30.0f){        // 小角度软件限位，防止出现意外
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_x--;                                                                // 减小动平台x方向位置
              yingshe_x= (int8_t)(g_moveplate_x -120);
              ReflashIk2Motor(yingshe_x,yingshe_y);                                           // 逆运动学更新
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);                       // 填充实心圈
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                     // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_SUB(44, 200, BLACK, BLUE);                                             // 显示按钮被点击的动画-按钮变色
            vTaskDelay(50);
            LCD_BOTTON_SUB(44, 200, BLACK, WHITE);                                            // 重新绘制按钮-按钮恢复
          }
          
          if (188<touch_x && touch_x<204 && 192<touch_y && touch_y<208 && g_machine_mode==0){ // 增大x方向的角度
            if (abs(yingshe_x+1) <= 30 && abs(yingshe_y)<=30){
//            if ((float)(sqrt((yingshe_x+1)*(yingshe_x+1)+yingshe_y*yingshe_y))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);              
              g_moveplate_x++;                                                                // 增加动平台x方向位置
              yingshe_x= (int8_t)(g_moveplate_x -120);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_ADD(196, 200, BLACK, BLUE);                                            // 显示按钮被点击的动画+按钮变色
            vTaskDelay(50);
            LCD_BOTTON_ADD(196, 200, BLACK, WHITE);                                           // 重新绘制按钮+按钮恢复
          }
          
          if (112<touch_x && touch_x<128 && 116<touch_y && touch_y<132 && g_machine_mode==0){ // 减小y方向的角度
            if (abs(yingshe_x) <= 30 && abs(yingshe_y-1)<=30){
//            if ((float)(sqrt(yingshe_x*yingshe_x+(yingshe_y-1)*(yingshe_y-1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_y--;                                                                // 减小动平台x方向位置
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_SUB(120, 124, BLACK, BLUE);                                            // 显示按钮被点击的动画-按钮变色
            vTaskDelay(50);
            LCD_BOTTON_SUB(120, 124, BLACK, WHITE);                                           // 重新绘制按钮-按钮恢复
          }
          
          if (112<touch_x && touch_x<128 && 268<touch_y && touch_y<284 && g_machine_mode==0){ // 增大y方向的角度
            if (abs(yingshe_x) <= 30 && abs(yingshe_y+1)<=30){
//            if ((float)(sqrt(yingshe_x*yingshe_x+(yingshe_y+1)*(yingshe_y+1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_y++;                                                                // 增加动平台y方向位置
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_ADD(120, 276, BLACK, BLUE);                                            // 显示按钮被点击的动画+按钮变色
            vTaskDelay(50);
            LCD_BOTTON_ADD(120, 276, BLACK, WHITE);                                           // 重新绘制按钮+按钮恢复
          }
          /*--------------------------------------------斜向运动--------------------------------------------------*/
          if (54<touch_x && touch_x<70 && 134<touch_y && touch_y<150 && g_machine_mode==0){ // 减小x方向，减小y方向的角度
            if (abs(yingshe_x-1) <= 30 && abs(yingshe_y-1)<=30){
//            if ((float)(sqrt((yingshe_x-1)*(yingshe_x-1)+(yingshe_y-1)*(yingshe_y-1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_x--;g_moveplate_y--;
              yingshe_x= (int8_t)(g_moveplate_x -120);
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_SUB(62, 142, BLACK, BLUE);                                           // 显示按钮被点击的动画-按钮变色
            vTaskDelay(50);
            LCD_BOTTON_SUB(62, 142, BLACK, WHITE);                                          // 重新绘制按钮-按钮恢复
          }
          
          if (170<touch_x && touch_x<186 && 134<touch_y && touch_y<150 && g_machine_mode==0){   // 增大x方向，减小y方向的角度
            if (abs(yingshe_x+1) <= 30 && abs(yingshe_y-1)<=30){
//            if ((float)(sqrt((yingshe_x+1)*(yingshe_x+1)+(yingshe_y-1)*(yingshe_y-1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_x++;g_moveplate_y--;                                                      
              yingshe_x= (int8_t)(g_moveplate_x -120);
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_ADD(178, 142, BLACK, BLUE);                                 // 显示按钮被点击的动画+按钮变色
            vTaskDelay(50);
            LCD_BOTTON_ADD(178, 142, BLACK, WHITE);                                // 重新绘制按钮+按钮恢复
          }
          
          if (54<touch_x && touch_x<70 && 250<touch_y && touch_y<266 && g_machine_mode==0){   // 减小x方向，增大y方向的角度
            if (abs(yingshe_x-1) <= 30 && abs(yingshe_y+1)<=30){
//            if ((float)(sqrt((yingshe_x-1)*(yingshe_x-1)+(yingshe_y+1)*(yingshe_y+1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_x--;g_moveplate_y++;   
              yingshe_x= (int8_t)(g_moveplate_x -120);            
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_SUB(62, 258, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
            vTaskDelay(50);
            LCD_BOTTON_SUB(62, 258, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
          }
          
          if (170<touch_x && touch_x<186 && 250<touch_y && touch_y<266 && g_machine_mode==0){   // 增大x方向，增大y方向的角度
            if (abs(yingshe_x+1) <= 30 && abs(yingshe_y+1)<=30){
//            if ((float)(sqrt((yingshe_x+1)*(yingshe_x+1)+(yingshe_y+1)*(yingshe_y+1)))<30.0f){
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, BLACK);
              g_moveplate_x++;g_moveplate_y++;               
              yingshe_x= (int8_t)(g_moveplate_x -120);
              yingshe_y= (int8_t)(g_moveplate_y -200);
              ReflashIk2Motor(yingshe_x,yingshe_y);
              LCD_Solid_Circle(g_moveplate_x, g_moveplate_y, 5, WHITE);
            }
            LCD_Fill(8, 280, 104, 292, BLACK);
            sprintf(effectpos, "末端X:%d度 Y:%d度", yingshe_x,yingshe_y);                         // 格式化坐标字符串
            LCD_String(8, 280, effectpos, 12, WHITE, BLACK);                                  // 显示动角台期望位姿
            LCD_BOTTON_ADD(178, 258, BLACK, BLUE);                                 // 显示按钮被点击的动画+按钮变色
            vTaskDelay(50);
            LCD_BOTTON_ADD(178, 258, BLACK, WHITE);                                // 重新绘制按钮+按钮恢复
          }
          /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>动平台运动圆圈>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
          
          /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<底边栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 302<touch_y && touch_y<318 && g_machine_mode==0){ // ********所有电机是否创建发送任务按钮检测*******
              if (g_transmit_status == 0){                                         // 创建电机控制任务
                g_transmit_status = 1;
                ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);
              }
              else if (g_transmit_status == 1){                                    // 删除电机发送控制任务
                g_transmit_status = 0;               
                vTaskDelete(xMotorTaskHandle);            
              }
              Judge_transmit(g_transmit_status, 181, 310-8); 
              }
            
            if (115<touch_x && touch_x<179 && 302<touch_y && touch_y<318){          // 改变推进器的控制状态，屏幕控制和手柄控制的切换
                if (g_machine_mode == 0){                                           // 进入手柄模式
                  g_machine_mode = 1;
                  MX_USB_HOST_Init();                                               // 创建手柄任务
                }
                else if (g_machine_mode == 1){
                  g_machine_mode = 0;                                               // 重新进入屏幕模式
                  vTaskDelete(hUsbHostFS.thread);                                   // 删除上述init创建的手柄任务
                }
                Judge_machine_mode(g_machine_mode, 115,310-8);             
              }
            
          if (15<touch_x && touch_x<63 && 302<touch_y && touch_y<318){              // 切换进入页面一
              if(g_transmit_status == 1){                                             // 如果开启了电机任务
                g_transmit_status = 0;                                            // 删除电机任务，安全退出页面一，退出也面前最好手动将电机回零
                Judge_transmit(g_transmit_status, 181, 310-8);
                vTaskDelete(xMotorTaskHandle);
              }
              if (g_machine_mode == 1){
                g_machine_mode = 0;                                               // 更换页面强行进入屏幕模式
                vTaskDelete(hUsbHostFS.thread);                                   // 删除上述init创建的手柄任务
                Judge_machine_mode(g_machine_mode, 115,310-8); 
                }
              g_handup = 1;
              status = 0;
              ret = xTaskCreate(Touch_ManageTask, "Touch_ManageTask", 256, NULL, osPriorityNormal, &xTouch_ManageTaskHandle[xTouch_ManageTaskHandleCount]); //创建页面一
              xTouch_ManageTaskHandleCount++;
              xTouch_ManageTaskHandle[xTouch_ManageTaskHandleCount] = NULL;

          }
          /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>底边栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
          vTaskDelay(50);                                                // 放在所有触摸检测函数的末尾，用于检测是否长按
          if (XPT2046_IsPressed()){                                      // 判断是否退出长按状态
            status = 1;
          }
          else {status = 0;g_handup = 1;}
        }
      }
  };
};

/******************************************************************************
 * 函  数： Touch_ManageTask
 * 功  能： 页面一中用于检测触摸和管理别的任务生死的任务
 * 参  数： 无
 * 返回值： 无
 * 备  注： 各行的意思已经全部解释
******************************************************************************/
void Touch_ManageTask(void *params){
    vTaskDelay(10);
    g_yemian_num = 1;                                                                // 更新页面标志位
    if (xIkTouchManageTaskHandleCount != 0){vTaskDelete(xIkTouchManageTaskHandle[xIkTouchManageTaskHandleCount-1]);}    // 杀死页面二
    // 页面一控制界面显示
    LCD_Fill(0, 0, 240, 320, BLACK);                                                 // 整个背景填充颜色
    LCD_String(10, 8, "水 下 矢 量 推 进", 24, WHITE, BLACK);                        // 显示标题;
    LCD_String(58, 40, "调 试 面 板", 24, YELLOW, BLACK);                            // 显示标题;
    LCD_Line(0, 37, 240, 37, GRAY);                                                  // 画个横线    
    LCD_Line(0, 38, 240, 38, GRAY);                                                  // 画个横线，加粗
    //LCD_String(18, 270, "基于混联机构的磁耦合水下矢量推进器", 12, WHITE, BLACK);     // 显示文字;

    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<螺旋桨电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 72, "螺旋桨电机状态：", 16, WHITE, BLACK); 
    Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                       // 判断pwm的值并显示电机状态为停止，正转和反转
    LCD_Rectangle(12, 70, 176, 90, GRAY);                                            // 绘制矩形框
    //按钮-显示
    LCD_BOTTON_SUB(22, 105, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
    //PWM标签显示
    LCD_String(80, 105-8, "PWM:", 16, WHITE, BLACK);
    //使能和强停按钮显示
    Judge_enable(g_rov_status, 181,72);                                             // 判断status的值并显示电机是否使能
    if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);        // 判断pwm的值并显示电机是否复位
    else LCD_String(181,105-8,"复位", 16, WHITE, RED);
    //pwm数值标签显示
    char strTem[100];                                                           
    sprintf(strTem, "%d", Rovmotor_PWM);                                          // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                            
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>螺旋桨电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM4310关节电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 118, "基电机状态：", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm4310_status, 110, 118);                                 // 判断反馈帧status的值并显示电机状态
    LCD_Rectangle(12, 116, 176, 136, GRAY);                                          // 绘制矩形框
    //使能,发送按钮显示
    Judge_enable(g_dm4310_status, 181,118);                                          // 判断status的值并显示电机是否使能
    LCD_String(181, 151-8, "存零", 16, BLACK, GREY);                                 // 创建存零点按钮
    //按钮-显示
    LCD_BOTTON_SUB(22, 151, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 151, BLACK, WHITE);
    //位置标签显示
    LCD_String(80, 151-8, "POS:", 16, WHITE, BLACK);
    //位置数值标签显示                                                        
    sprintf(strTem, "%3.1f", g_dm4310_real_pos*180/3.14f);                            // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 151-8, strTem, 16, WHITE, BLACK);  
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM4310关节电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机1栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 164, "关节1状态：", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_1_status, 110, 164);                                 // 判断反馈帧status的值并显示电机状态
    LCD_Rectangle(12, 162, 176, 182, GRAY);                                          // 绘制矩形框
    //使能,保存零点按钮显示
    Judge_enable(g_dm3910_1_status, 181,164);                                          // 判断status的值并显示电机是否使能
    LCD_String(181, 197-8, "存零", 16, BLACK, GREY);                                   // 创建存零点按钮
    //按钮-显示
    LCD_BOTTON_SUB(22, 197, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 197, BLACK, WHITE);
    //位置标签显示
    LCD_String(80, 197-8, "POS:", 16, WHITE, BLACK);
    //位置数值标签显示                                                        
    sprintf(strTem, "%3.1f", g_dm3910_1_real_pos*180/3.14f);                            // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 197-8, strTem, 16, WHITE, BLACK);  
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机1栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机2栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 210, "关节2状态：", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_2_status, 110, 210);                                 // 判断反馈帧status的值并显示电机状态
    LCD_Rectangle(12, 208, 176, 228, GRAY);                                          // 绘制矩形框
    //使能,保存零点按钮显示
    Judge_enable(g_dm3910_2_status, 181,210);                                          // 判断status的值并显示电机是否使能
    LCD_String(181, 243-8, "存零", 16, BLACK, GREY);                                   // 创建存零点按钮
    //按钮-显示
    LCD_BOTTON_SUB(22, 243, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 243, BLACK, WHITE);
    //位置标签显示
    LCD_String(80, 243-8, "POS:", 16, WHITE, BLACK);
    //位置数值标签显示                                                        
    sprintf(strTem, "%3.1f", g_dm3910_2_real_pos*180/3.14f);                            // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 243-8, strTem, 16, WHITE, BLACK);
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机2栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机3栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    //电机状态标签显示
    LCD_String(14, 256, "关节3状态：", 16, WHITE, BLACK); 
    Judge_DMmotor_status(g_dm3910_3_status, 110, 256);                                 // 判断反馈帧status的值并显示电机状态
    LCD_Rectangle(12, 254, 176, 274, GRAY);                                          // 绘制矩形框
    //使能,保存零点按钮显示
    Judge_enable(g_dm3910_3_status, 181,256);                                          // 判断status的值并显示电机是否使能
    LCD_String(181, 289-8, "存零", 16, BLACK, GREY);                                   // 创建存零点按钮
    //按钮-显示
    LCD_BOTTON_SUB(22, 289, BLACK, WHITE);
    //按钮+显示
    LCD_BOTTON_ADD(50, 289, BLACK, WHITE);
    //位置标签显示
    LCD_String(80, 289-8, "POS:", 16, WHITE, BLACK);
    //位置数值标签显示                                                        
    sprintf(strTem, "%3.1f", g_dm3910_3_real_pos*180/3.14f);                            // 使用sprintf进行字符串格式化，输出整形数据
    LCD_String(120, 289-8, strTem, 16, WHITE, BLACK);
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机3栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<底边控制栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
    Judge_transmit(g_transmit_status, 181, 310-8);                                   // 判断是否创建CAN发送任务按钮
    LCD_String(15, 310-8, "页面一", 16, WHITE, GREEN);                               // 创建一个页面一的任务，同时删除页面二自身的任务
    LCD_String(65, 310-8, "页面二", 16, BLACK, GRAY);                               // 创建一个页面二的任务，同时删除页面一自身的任务
    Judge_machine_mode(g_machine_mode, 115,310-8);                                  // 显示当前的控制模式，按下可以切换
    /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>底边控制栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    while (1)
    {
        uint16_t touch_x = 0;
        uint16_t touch_y = 0;
        /** 1ms延时 **/
        vTaskDelay(1);                                                  
        /** 检查触摸屏是否按下，并进行相关处理 **/
        if (XPT2046_IsPressed() && g_handup == 1)                          // 检查触摸屏是否从抬手状态按下; 返回：0-未按下、1-按下
        {
          vTaskDelay(10);                                                  //rtos中改成vtaskdelay就行，就不用定时器再定时了
          if (XPT2046_IsPressed()){
            g_handup = 0;                                                  // 确认触摸按下
            touch_x = XPT2046_GetX();                                      // 获取触摸点x位置
            touch_y = XPT2046_GetY();                                      // 获取触摸点y位置
            LCD_DrawPoint(touch_x, touch_y, GREEN);                        // 在按下的位置，画点; 用于测试
            static char str[20] = {0};                                     // 新建数组，用于存放坐标字符串
            sprintf(str, "X:%3d  Y:%3d", touch_x, touch_y);                // 格式化坐标字符串
            LCD_String(8, 5, str, 12, WHITE, BLACK);                       // 显示坐标字符串
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<螺旋桨电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 72<touch_y && touch_y<88 && g_machine_mode==0){   // 螺旋桨电机使能按钮检测，开启或关闭
              if (g_rov_status == 0){g_rov_status = 1;HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);}
              else if (g_rov_status == 1) {g_rov_status = 0;HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_4);}
              Judge_enable(g_rov_status, 181,72);                                               // 判断status的值并显示电机是否使能
              }
            
            if (181<touch_x && touch_x<213 && 97<touch_y && touch_y<113 && g_machine_mode==0){  // 螺旋桨电机复位按钮检测
              Rovmotor_PWM = 750;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
              if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
              else LCD_String(181,105-8,"复位", 16, WHITE, RED);
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                        // 判断pwm的值并显示电机状态为停止，正转和反转
              }
            
            if (14<touch_x && touch_x<30 && 97<touch_y && touch_y<113 && g_machine_mode==0){    // 螺旋桨电机pwm-按钮检测
              Rovmotor_PWM = Rovmotor_PWM - 5;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
              LCD_BOTTON_SUB(22, 105, BLACK, BLUE);                                             // 显示按钮被点击的动画-按钮变色
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
              if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
              else LCD_String(181,105-8,"复位", 16, WHITE, RED);
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK);                                 // 更改显示的PWM值
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 105, BLACK, WHITE);                                            // 重新绘制按钮-按钮恢复
              }
            
            if (42<touch_x && touch_x<58 && 97<touch_y && touch_y<113 && g_machine_mode==0){    //螺旋桨电机pwm+按钮检测
              Rovmotor_PWM = Rovmotor_PWM + 5;
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4,Rovmotor_PWM);                        // 更新PA3接口的PWM值
              LCD_BOTTON_ADD(50, 105, BLACK, BLUE);
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);
              if (Rovmotor_PWM != 750) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
              else LCD_String(181,105-8,"复位", 16, WHITE, RED);
              sprintf(strTem, "%d", Rovmotor_PWM); 
              LCD_String(120, 105-8, strTem, 16, WHITE, BLACK); 
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 105, BLACK, WHITE);
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>螺旋桨电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM4310关节电机栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 118<touch_y && touch_y<134 && g_machine_mode==0){  // 基座电机使能按钮检测
                if (g_dm4310_status == 0){                                                       // 使能DM4310电机
                  //g_dm4310_status = 1;
                  dm_motor_enable(&hcan1, &motor[Motor1]);
                  vTaskDelay(200);                                                               // 延时，防止出问题
                }
                else {                                                                           // 失能DM4310电机
                  //g_dm4310_status = 0;
                  dm_motor_disable(&hcan1, &motor[Motor1]);
                  vTaskDelay(200);                                                               // 延时，防止出问题
                }                                                      
                Judge_enable(g_dm4310_status, 181,118);                                          // 判断status的值并显示电机是否使能              
              }
            
            if (181<touch_x && touch_x<213 && 143<touch_y && touch_y<159 && g_machine_mode==0){   // 当前位置设为零点的按钮检测
                //补入设为零点的函数
                LCD_String(181, 151-8, "存零", 16, BLACK, BLUE);                     // 显示按钮被点击的动画-按钮变色
                save_pos_zero(&hcan1, motor[Motor1].id, motor[Motor1].ctrl.mode);    // 保存当前位置为该电机零点
                if (g_dm4310_real_pos>0)motor[Motor1].ctrl.vel_set 	= -3.0f;         // 因为电机有负载，需要有前馈的速度
                else motor[Motor1].ctrl.vel_set = 3.0f;                              // 如果位置是负的，则要给正速度增大电流，反之负速度
                g_dm4310_target_pos = 0.0f;
                motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
                vTaskDelay(50);                                                      // 等待一会
                LCD_String(181, 151-8, "存零", 16, BLACK, WHITE);                    // 重新绘制按钮-按钮恢复             
              }
            
            if (14<touch_x && touch_x<30 && 143<touch_y && touch_y<159 && g_machine_mode==0){    // 基座电机pos-按钮检测，这里改变的是target，pos显示的是real
//              motor[Motor1].ctrl.vel_set 	= -3.0f;                                  // 给前馈速度，已丢弃，因为给了较大的kp
              g_dm4310_target_pos = g_dm4310_target_pos - 0.1f;                     // 更新motor[Motor1]结构体中发送给电机的位置
              motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
              LCD_BOTTON_SUB(22, 151, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 151, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            
            if (43<touch_x && touch_x<58 && 143<touch_y && touch_y<159 && g_machine_mode==0){   // 基座电机pos+按钮检测，这里改变的是target，pos显示的是real
//              motor[Motor1].ctrl.vel_set 	= 3.0f;                                   // 给前馈速度
              g_dm4310_target_pos = g_dm4310_target_pos + 0.1f;                     // 更新motor[Motor1]结构体中发送给电机的位置
              motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
              LCD_BOTTON_ADD(50, 151, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 151, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM4310关节电机栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机1栏0x02<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 164<touch_y && touch_y<180 && g_machine_mode==0){ // DM3910电机1使能按钮检测
                if (g_dm3910_1_status == 0){                                         // 使能DM3519电机1
                  //g_dm3910_1_status = 1;
                  dm_motor_enable(&hcan1, &motor[Motor2]);
                  vTaskDelay(200);
                }
                else  {                                                              // 失能DM3519电机1
                  //g_dm3910_1_status = 0;                                       
                  dm_motor_disable(&hcan1, &motor[Motor2]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_1_status, 181,164);                              // 判断status的值并显示电机是否使能              
              }
            
            if (181<touch_x && touch_x<213 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // 当前位置设为零点的按钮检测
                //补入设为零点的函数
                LCD_String(181, 197-8, "存零", 16, BLACK, BLUE);                     // 显示按钮被点击的动画-按钮变色
                save_pos_zero(&hcan1,motor[Motor2].id, motor[Motor2].ctrl.mode);
                g_dm3910_1_target_pos = 0.0f;
                motor[Motor2].ctrl.pos_set 	= g_dm3910_1_target_pos;
                vTaskDelay(80);
                LCD_String(181, 197-8, "存零", 16, BLACK, WHITE);                    // 重新绘制按钮-按钮恢复             
              }
            
            if (14<touch_x && touch_x<30 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // 基座电机pos-按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_1_target_pos = g_dm3910_1_target_pos - 0.2*3591/187.0f;
              motor[Motor2].ctrl.pos_set = g_dm3910_1_target_pos;
              LCD_BOTTON_SUB(22, 197, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 197, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            
            if (43<touch_x && touch_x<58 && 189<touch_y && touch_y<205 && g_machine_mode==0){ // 基座电机pos+按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_1_target_pos = g_dm3910_1_target_pos + 0.2*3591/187.0f;
              motor[Motor2].ctrl.pos_set = g_dm3910_1_target_pos;
              LCD_BOTTON_ADD(50, 197, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 197, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机1栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机2栏0x03<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 210<touch_y && touch_y<226 && g_machine_mode==0){ // DM3910电机2使能按钮检测
                if (g_dm3910_2_status == 0){
                  dm_motor_enable(&hcan1,&motor[Motor3]);
                  vTaskDelay(200);
                }
                else {
                  dm_motor_disable(&hcan1,&motor[Motor3]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_2_status, 181,210);                            // 判断status的值并显示电机是否使能              
              }
            
            if (181<touch_x && touch_x<213 && 235<touch_y && touch_y<251 && g_machine_mode==0){ // 当前位置设为零点的按钮检测
                //补入设为零点的函数
                LCD_String(181, 243-8, "存零", 16, BLACK, BLUE);                     // 显示按钮被点击的动画-按钮变色
                save_pos_zero(&hcan1,motor[Motor3].id, motor[Motor3].ctrl.mode);
                g_dm3910_2_target_pos = 0.0f;
                motor[Motor3].ctrl.pos_set 	= g_dm3910_2_target_pos;
                vTaskDelay(80);
                LCD_String(181, 243-8, "存零", 16, BLACK, WHITE);                    // 重新绘制按钮-按钮恢复             
              }
            
            if (14<touch_x && touch_x<30 && 235<touch_y && touch_y<251 && g_machine_mode==0){  // 基座电机pos-按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_2_target_pos = g_dm3910_2_target_pos - 0.2*3591/187.0f;
              motor[Motor3].ctrl.pos_set = g_dm3910_2_target_pos;
              LCD_BOTTON_SUB(22, 243, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 243, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            
            if (43<touch_x && touch_x<58 && 235<touch_y && touch_y<251 && g_machine_mode==0){ // 基座电机pos+按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_2_target_pos = g_dm3910_2_target_pos + 0.2*3591/187.0f ;
              motor[Motor3].ctrl.pos_set = g_dm3910_2_target_pos;
              LCD_BOTTON_ADD(50, 243, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 243, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机2栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<DM3910关节电机3栏0x04<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/  
            if (181<touch_x && touch_x<213 && 256<touch_y && touch_y<272 && g_machine_mode==0){ // DM3910电机3使能按钮检测
                if (g_dm3910_3_status == 0){
                  dm_motor_enable(&hcan1,&motor[Motor4]);
                  vTaskDelay(200);
                }
                else {
                  dm_motor_disable(&hcan1,&motor[Motor4]);
                  vTaskDelay(200);
                }
                Judge_enable(g_dm3910_3_status, 181,256);                            // 判断status的值并显示电机是否使能              
              }
            
            if (181<touch_x && touch_x<213 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // 当前位置设为零点的按钮检测
                //补入设为零点的函数
                LCD_String(181, 289-8, "存零", 16, BLACK, BLUE);                     // 显示按钮被点击的动画-按钮变色
                save_pos_zero(&hcan1,motor[Motor4].id, motor[Motor4].ctrl.mode);
                g_dm3910_3_target_pos = 0.0f;
                motor[Motor4].ctrl.pos_set 	= g_dm3910_3_target_pos;
                vTaskDelay(80);
                LCD_String(181, 289-8, "存零", 16, BLACK, WHITE);                    // 重新绘制按钮-按钮恢复             
              }
            
            if (14<touch_x && touch_x<30 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // 基座电机pos-按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_3_target_pos = g_dm3910_3_target_pos - 0.2*3591/187.0f;
              motor[Motor4].ctrl.pos_set = g_dm3910_3_target_pos;
              LCD_BOTTON_SUB(22, 289, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 289, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            
            if (43<touch_x && touch_x<58 && 281<touch_y && touch_y<297 && g_machine_mode==0){ // 基座电机pos+按钮检测，这里改变的是target，pos显示的是real
              g_dm3910_3_target_pos = g_dm3910_3_target_pos + 0.2*3591/187.0f;
              motor[Motor4].ctrl.pos_set = g_dm3910_3_target_pos;
              LCD_BOTTON_ADD(50, 289, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_ADD(50, 289, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DM3910关节电机3栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
              
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<底边栏<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
            if (181<touch_x && touch_x<213 && 302<touch_y && touch_y<318 && g_machine_mode==0){ // ********所有电机是否创建发送任务按钮检测*******
              if (g_transmit_status == 0){                                         // 创建电机控制任务
                g_transmit_status = 1;
                ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);
              }
              else if (g_transmit_status == 1){                                    // 删除电机发送控制任务
                g_transmit_status = 0;               
                vTaskDelete(xMotorTaskHandle);            
              }
              Judge_transmit(g_transmit_status, 181, 310-8); 
              }
            
            if (115<touch_x && touch_x<179 && 302<touch_y && touch_y<318){          // 改变推进器的控制状态，屏幕控制和手柄控制的切换
                if (g_machine_mode == 0){                                           // 进入手柄模式
                  g_machine_mode = 1;
                  MX_USB_HOST_Init();                                               // 创建手柄任务
                }
                else if (g_machine_mode == 1){
                  g_machine_mode = 0;                                               // 重新进入屏幕模式
                  vTaskDelete(hUsbHostFS.thread);                                   // 删除上述init创建的手柄任务
                }
                Judge_machine_mode(g_machine_mode, 115,310-8);             
              }
            
            if(65<touch_x && touch_x<113 && 302<touch_y && touch_y<318){            // 删除当前的页面一主控任务，创建页面二的主控任务
                if(g_transmit_status == 1){                                         // 如果开启了电机任务
                  g_transmit_status = 0;                                            // 删除电机任务，安全退出页面一，退出也面前最好手动将电机回零
                  Judge_transmit(g_transmit_status, 181, 310-8);
                  vTaskDelete(xMotorTaskHandle);
                }
                if (g_machine_mode == 1){
                  g_machine_mode = 0;                                               // 更换页面强行进入屏幕模式
                  vTaskDelete(hUsbHostFS.thread);                                   // 删除上述init创建的手柄任务
                  Judge_machine_mode(g_machine_mode, 115,310-8); 
                }
                g_handup = 1;
                ret = xTaskCreate(ik_Touch_ManageTask, "ik_Touch_ManageTask", 256, NULL, osPriorityNormal, &xIkTouchManageTaskHandle[xIkTouchManageTaskHandleCount]); //创建页面二
                xIkTouchManageTaskHandleCount++;
                xIkTouchManageTaskHandle[xIkTouchManageTaskHandleCount] = NULL;
            }
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>底边栏>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
            } 
          }
      if (XPT2046_IsPressed() == 0) g_handup = 1;                      //抬手状态检测，消抖
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
  ret = xTaskCreate(CommunicationTask, "CommunicationTask", 128, NULL, osPriorityNormal, &xCommunicationTaskHandle);  // 用于通讯测试，闪灯判断系统运行没问题
  ret = xTaskCreate(Touch_ManageTask, "Touch_ManageTask", 256, NULL, osPriorityNormal, &xTouch_ManageTaskHandle[xTouch_ManageTaskHandleCount]);   // 用于页面一检测触摸和管理别的任务生死的任务
  xTouch_ManageTaskHandleCount = xTouch_ManageTaskHandleCount + 1;
  xTouch_ManageTaskHandle[xTouch_ManageTaskHandleCount] = NULL;
  //ret = xTaskCreate(ik_Touch_ManageTask, "ik_Touch_ManageTask", 128, NULL, osPriorityNormal, &xIkTouchManageTaskHandle);   // 用于页面二检测触摸和管理别的任务生死的任务,这里也是声明不创建
  //ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);                      // 创建电机发送任务，这里只是声明并不创建

}
