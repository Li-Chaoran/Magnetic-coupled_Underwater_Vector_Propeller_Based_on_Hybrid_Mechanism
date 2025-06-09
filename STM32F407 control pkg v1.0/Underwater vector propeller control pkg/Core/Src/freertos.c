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


/* Private typedef -----------------------------------------------------------*/
extern void MX_USB_HOST_Process(void);

/* 声明任务句柄 */
TaskHandle_t xCommunicationTaskHandle = NULL;
TaskHandle_t xTouch_ManageTaskHandle = NULL;
TaskHandle_t xMotorTaskHandle = NULL;
BaseType_t ret;  // 接收判断任务创建是否成功的变量
osThreadId defaultTaskHandle;

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
 * 函  数： Touch_ManageTask
 * 功  能： 用于检测触摸和管理别的任务生死的任务
 * 参  数： 无
 * 返回值： 无
 * 备  注： 各行的意思已经全部解释
******************************************************************************/
void Touch_ManageTask(void *params){
//首页界面显示
    LCD_Fill(0, 0, 240, 320, WHITE);                                                // 整个背景填充颜色
    extern const unsigned char yanda_152_60[ ];                                     // 图片数组声明; 存放于font.h文件中。这里仅作示范，不建议使用, 太吃资源了。
    LCD_Image(45,  150,  152, 60, yanda_152_60);                                    // 图片显示：x坐标、Y坐标、图片宽度、图片高度，图片数组地址; 注意，这里的图片尺寸是指真实尺寸
    LCD_String(10, 285, "如果发现坐标不对，按下左1按键进入校准", 12, BLACK, WHITE);   // 显示文字;
    LCD_String(80, 80, "作者：林楚然", 12, YELLOW, WHITE);     // 显示文字;
    LCD_String(68, 100, "指导老师：叶妮佳", 12, YELLOW, WHITE);     // 显示文字;
    LCD_String(35, 8, "基于混联机构的", 24, BLACK, WHITE);     // 显示文字;
    LCD_String(40, 40, "磁耦合水下矢量推进器", 16, DARKBLUE, WHITE);     // 显示文字;
    vTaskDelay(3000);
    
    // 控制界面显示
    LCD_Fill(0, 0, 240, 320, BLACK);                                                 // 整个背景填充颜色
    LCD_String(10, 8, "水 下 矢 量 推 进", 24, WHITE, BLACK);                        // 显示标题;
    LCD_String(58, 40, "控 制 面 板", 24, YELLOW, BLACK);                            // 显示标题;
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
              if (Rovmotor_PWM != 75) LCD_String(181,105-8,"复位", 16, BLACK, GREY);
              else LCD_String(181,105-8,"复位", 16, WHITE, RED);
              Judge_rovmotor_pwm(Rovmotor_PWM, 140, 72);                                        // 判断pwm的值并显示电机状态为停止，正转和反转
              }
            
            if (14<touch_x && touch_x<30 && 97<touch_y && touch_y<113 && g_machine_mode==0){    // 螺旋桨电机pwm-按钮检测
              Rovmotor_PWM = Rovmotor_PWM - 50;
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
              Rovmotor_PWM = Rovmotor_PWM + 50;
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
              motor[Motor1].ctrl.vel_set 	= -3.0f;                                  // 给前馈速度
              g_dm4310_target_pos = g_dm4310_target_pos - 0.2f;                     // 更新motor[Motor1]结构体中发送给电机的位置
              motor[Motor1].ctrl.pos_set 	= g_dm4310_target_pos;
              LCD_BOTTON_SUB(22, 151, BLACK, BLUE);                                 // 显示按钮被点击的动画-按钮变色
              vTaskDelay(50);
              LCD_BOTTON_SUB(22, 151, BLACK, WHITE);                                // 重新绘制按钮-按钮恢复
              }
            
            if (43<touch_x && touch_x<58 && 143<touch_y && touch_y<159 && g_machine_mode==0){   // 基座电机pos+按钮检测，这里改变的是target，pos显示的是real
              motor[Motor1].ctrl.vel_set 	= 3.0f;                                   // 给前馈速度
              g_dm4310_target_pos = g_dm4310_target_pos + 0.2f;                     // 更新motor[Motor1]结构体中发送给电机的位置
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
  ret = xTaskCreate(Touch_ManageTask, "Touch_ManageTask", 128, NULL, osPriorityNormal, &xTouch_ManageTaskHandle);   // 用于检测触摸和管理别的任务生死的任务
  //ret = xTaskCreate(Motor_Task, "Motor_Task", 128, NULL, osPriorityNormal, &xMotorTaskHandle);                      // 创建电机发送任务，这里只是声明并不创建

}
