#include "bsp_XPT2046.h"            // 头文件
#include "bsp_W25Q128.h"            // W25Q128的驱动文件，触摸屏的校准数据存放位置：0x00A0000-100，即字库前100字节                                    
#include "string.h"                 // C语言的标准函数库，用于字符处理函数
#include "stdio.h"                  // C语言的标准函数库，用于printf等函数
                                    



/******************************* 声明相关的静态变量 ***************************/
typedef    struct                   // 触摸屏坐标结构体
{                                   
    uint8_t EN;                     // 触摸检测开关, 0_关闭, 1_打开; 关闭和打开, 是指代码上的检测与否, 不用时可关闭, 减少芯片资源消耗
                                    
    uint16_t lcdX;                  // 当前按下的LCD坐标值(
    uint16_t lcdY;                  
                                    
    uint16_t adcX;                  // 保存最后读取的触摸屏X方向的ADC值，已用平均值滤波
    uint16_t adcY;                  
                                    
    uint16_t lcdWidth;              // 用于记录LCD实际的宽度像素; 在XPT2046_Init裡賦值
    uint16_t lcdHeight;             // 用于记录LCD实际的高度像素; 在XPT2046_Init裡賦值
                                    
    float xfac;                     // 触摸屏与LCD的坐标比例系数,  xfac=(float)(20-320)/(t1x-t2x);
    float yfac;                     
    short xoff;                     // 像素点偏移值, xoff=(320-xfac*(t1x+t2x))/2;
    short yoff;                     
                                    
    uint8_t  dir;                   // 显示方向, 0-竖屏, 1_横屏
    uint32_t dataAddr;              // 触摸数据在内部FLASH中的存放地址
} xXPT2046_TypeDey;

static xXPT2046_TypeDey xXPT2046;   // 用于存放XPT2046的各种信息，全局变量，在h中extern





/******************************* 宏定义 ***************************/
#define XPT2046_CHANNEL_X   0x90         // 控制字：检测通道Y+电压值    
#define XPT2046_CHANNEL_Y   0xD0         // 控制字：检测通道X+电压值

#define  IRQ_READ           ( XPT2046_IRQ_GPIO -> IDR & XPT2046_IRQ_PIN )                  // 触摸信号引脚，闲时高电平，按下时低电平
#define  CS_HIGH            ( XPT2046_CS_GPIO  -> BSRR  = XPT2046_CS_PIN )
#define  CS_LOW             ( XPT2046_CS_GPIO  -> BSRR  = XPT2046_CS_PIN << 16 )
#define  CLK_HIGH           ( XPT2046_CLK_GPIO -> BSRR  = XPT2046_CLK_PIN)
#define  CLK_LOW            ( XPT2046_CLK_GPIO -> BSRR  = XPT2046_CLK_PIN << 16 )
#define  MOSI_1             ( XPT2046_MOSI_GPIO-> BSRR  = XPT2046_MOSI_PIN )
#define  MOSI_0             ( XPT2046_MOSI_GPIO-> BSRR  = XPT2046_MOSI_PIN << 16 )
#define  MISO               ( (( XPT2046_MISO_GPIO -> IDR) & XPT2046_MISO_PIN ) ? 1 : 0 )





/******************************* 声明本地函数 ***************************/
static void      delayUS(uint32_t ulCount);       // 粗略的延时函数, 为方便移植, 不使用外部延时函数
static void      sendCMD(uint8_t cmd);            // 发送命令字
static uint16_t  receiveData(void);               // 读取返回值
static int16_t   readADC_X(void);                 // 读取X的ADC值
static int16_t   readADC_Y(void);                 // 读取Y的ADC值
static uint8_t   readAdcXY(void);                 // 读取X、Y的ADC值并滤波，存放到全局结构体变量xXPT2046中






// 本地US粗略延时函数，减少移植时对外部文件依赖；
static void delayUS(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++)
    {
        uint8_t uc = 12;     //设置值为12，大约延1微秒
        while (uc --);       //延1微秒
    }
}



// 写入命令字
// Cmd ：0x90_通道Y+的选择控制字, 0xd0_通道X+的选择控制字
// XPT2046时序要求：CLK闲时低电平，上升沿采样数据，下降沿改变数据
// 注意：发送命令字后，返回的数据，与命令字，是同一组数据内(CS低、高电平期间)
static void sendCMD(uint8_t cmd)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        ((cmd >> (7 - i)) & 0x01) ? MOSI_1 : MOSI_0; // 高位先行
        delayUS(1);
        CLK_HIGH  ;
        delayUS(1);
        CLK_LOW   ;
    }
}



// 读取返回数据(紧跟sendCMD函数后面);
// 返回读取到的数据
static uint16_t receiveData(void)
{
    uint16_t usBuf = 0;

    CLK_HIGH;              // 给一个时钟，清除BUSY；这个时序是跟在发送命令字后面的，AD转换需要大约6US
    delayUS(5);
    CLK_LOW ;
    delayUS(5);

    for (uint8_t i = 0; i < 12; i++)
    {
        usBuf = usBuf << 1;
        CLK_HIGH;
        delayUS(1);
        usBuf |= MISO ;    // 高位先行
        CLK_LOW;
        delayUS(1);
    }

    return usBuf;
}



// 选择一个模拟通道后，启动ADC，并返回ADC采样结果
// 0x90 :通道Y+的选择控制字
// 0xd0 :通道X+的选择控制字
static int16_t readADC_X(void)
{
    if (xXPT2046.dir == 0)    sendCMD(XPT2046_CHANNEL_Y);
    if (xXPT2046.dir == 1)    sendCMD(XPT2046_CHANNEL_X);
    return  receiveData();
}



static int16_t readADC_Y(void)
{
    if (xXPT2046.dir == 0)    sendCMD(XPT2046_CHANNEL_X);
    if (xXPT2046.dir == 1)    sendCMD(XPT2046_CHANNEL_Y);
    return  receiveData();
}



/******************************************************************************
 * 函数名： readAdcXY
 * 功  能： 获取触摸屏按下时X、Y的ADC值，并滤波
 * 参  数：
 * 返  回： 1  获取成功，已存放到传入的结构体中
 *          0  失败
 ******************************************************************************/
static uint8_t readAdcXY()
{
    static uint8_t  cnt = 0;
    static uint16_t xSum = 0, ySum = 0;
    static int16_t  xyArray [2] [10] = {{0}, {0}};    // 临时二维数组，用于存放坐标X、Y的10次采样
    int32_t  xMin, xMax, yMin, yMax;                  // 存储采样中的最小值、最大值;　采样多次后，去头去尾求平均值

    cnt = 0;
    xSum = 0;
    ySum = 0;
    memset(xyArray, 0, 20);
    xMin = 0;
    xMax = 0;
    yMin = 0;
    yMax = 0;

    while ((IRQ_READ == 0) && (cnt < 4))              // 多次采集; 条件：TP_INT_IN信号为低(屏幕初按下), 且 cnt<10
    {
        xyArray[0] [cnt] = readADC_X();
        xyArray[1] [cnt] = readADC_Y();
        cnt ++;
    }

    // 开始求平均值
    xMax = xMin = xyArray [0] [0];                              // 筛选等会要去掉的最小值、最大值
    yMax = yMin = xyArray [1] [0];
    for (uint8_t i = 1; i < cnt; i++)
    {
        if (xyArray[0] [i] < xMin)    xMin = xyArray [0] [i];   // 求x的10次采样最小ADC值
        if (xyArray[0] [i] > xMax)    xMax = xyArray [0] [i];   // 求x的10次采样最大ADC值

        if (xyArray[1] [i] < yMin)    yMin = xyArray [1] [i];   // 求y的10次采样最小ADC值
        if (xyArray[1] [i] > yMax)    yMax = xyArray [1] [i];   // 求y的10次采样最小ADC值
    }
    // 去除最小值和最大值之后求平均值
    for (uint8_t i = 0; i < cnt; i++)
    {
        xSum = xSum + xyArray[0][i];
        ySum = ySum + xyArray[1][i];
    }
    xXPT2046.adcX = (xSum - xMin - xMax) >> 1;  // 去除最小值和最大值之后，除2
    xXPT2046.adcY = (ySum - yMin - yMax) >> 1;  // 去除最小值和最大值之后，除2

    return 1;
}



// 把电压值换算成对应的LCD坐标值
static void adcXYToLcdXY(void)
{
    static int16_t lcdX = 0;
    static int16_t lcdY = 0;
    // 计算比例系数
    lcdX = xXPT2046.adcX * xXPT2046.xfac + xXPT2046.xoff ;
    lcdY = xXPT2046.adcY * xXPT2046.yfac + xXPT2046.yoff ;
    // 限制坐标值范围
    if (lcdX < 0)  lcdX = 0;
    if (lcdX > xXPT2046.lcdWidth)  lcdX = xXPT2046.lcdWidth;
    if (lcdY < 0)  lcdY = 0;
    if (lcdY > xXPT2046.lcdHeight)  lcdY = xXPT2046.lcdHeight;
    // 经换算, 及限值后的坐标值, 转存到结构体, 随时可调用
    xXPT2046.lcdX = lcdX;
    xXPT2046.lcdY = lcdY;
}



/******************************************************************************
 * 函数名： writeDcorrectingData
 * 功  能： 写入校正数据
 * 参  数： float xfac   x轴比例因子
 *          float yfac   y轴比例因子
 *          short xoff   x轴偏移像素
 *          short yoff   y轴偏移像素
 * 返  回： 0_写入成功，非0_写入失败
 ******************************************************************************/
static uint8_t writeCorrectingData(float xfac, float yfac, short xoff, short yoff)
{
    uint32_t addr = 0;
    if (xXPT2046.dir == 0)
        addr = xXPT2046.dataAddr;
    if (xXPT2046.dir == 1)
        addr = xXPT2046.dataAddr + 20;

    uint8_t err = 0;
    uint16_t flag = 'O' | ('K' << 8);
    W25Q128_WriteData(addr + 0, (uint8_t *)&flag, 2);
    W25Q128_WriteData(addr + 2, (uint8_t *)&xfac, 4);
    W25Q128_WriteData(addr + 6, (uint8_t *)&xoff, 4);
    W25Q128_WriteData(addr + 10, (uint8_t *)&yfac, 4);
    W25Q128_WriteData(addr + 14, (uint8_t *)&yoff, 4);
    return err;
}



/******************************************************************************
 * 函数名： XPT2046_Init
 * 功  能： 初始化
 * 参  数： uint16_t lcdWidth     LCD宽度像素
 *          uint16_t lcdHeight    LCD宽度像素
 *          uint8_t dir           显示方向    0-正竖屏，3-倒竖屏，5-正横屏, 6-倒横屏
 * 返  回：
 ******************************************************************************/
void XPT2046_Init(uint16_t lcdWidth, uint16_t lcdHeight, uint8_t dir)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    xXPT2046.dataAddr  = 0x00A00000 - 100 ;                      // 校准数据的存放在W25Q128的位置; 0x00A00000是字库的起始位置，把校准数据存放在它的前面。

    xXPT2046.lcdWidth  = lcdWidth;
    xXPT2046.lcdHeight = lcdHeight;
    xXPT2046.dir = dir;

#ifdef USE_HAL_DRIVER                                           // HAL库 配置
    // 使能GPIO端口
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN ;  // 使能GPIOA、B、C、D、时钟
                                        
    /// 初始化 CS引脚
    GPIO_InitStruct.Pin   = XPT2046_CS_PIN;                      // 引脚编号
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;                 // 引脚工作模式：推挽输出
    GPIO_InitStruct.Pull  = GPIO_PULLUP;                         // 内部上下拉：上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;           // 引脚翻转速率：最快
    HAL_GPIO_Init(XPT2046_CS_GPIO, &GPIO_InitStruct);            // 初始化

    CS_HIGH;                                                     // 拉高片选，防止误操作

    GPIO_InitStruct.Pin   = XPT2046_CLK_PIN;                     // CLK
    HAL_GPIO_Init(XPT2046_CLK_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = XPT2046_MOSI_PIN;                    // MOSI
    HAL_GPIO_Init(XPT2046_MOSI_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = XPT2046_MISO_PIN;                    // MISO
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;                     // 上拉输入
    HAL_GPIO_Init(XPT2046_MISO_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.Pin  = XPT2046_IRQ_PIN;                      // IRQ信号引脚
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                      // 触摸屏触摸信号指示引脚，不使用中断
    HAL_GPIO_Init(XPT2046_IRQ_GPIO, &GPIO_InitStruct);
#endif

#ifdef USE_STDPERIPH_DRIVER                                      // 标准库 配置
    // 使能时钟
    RCC_AHB1PeriphClockCmd(XPT2046_IRQ_PORT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(XPT2046_CS_PORT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(XPT2046_CLK_PORT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(XPT2046_MOSI_PORT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(XPT2046_MISO_PORT_CLK, ENABLE);
    // 配置CS引脚工作模式
    GPIO_InitStruct.GPIO_Pin   = XPT2046_CS_PIN;                 // 选择要控制的GPIO引脚
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;                  // 引脚模式：输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;                  // 输出类型：推挽输出
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;                   // 上下拉：  上拉模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;                // 引脚速率：2MHz
    GPIO_Init(XPT2046_CS_GPIO, &GPIO_InitStruct);                // 调用库函数，使用上面的配置初始化GPIO
    CS_HIGH;                                                     // 拉高片选，防止误操作
    // CLK                                                       
    GPIO_InitStruct.GPIO_Pin   = XPT2046_CLK_PIN;                // CLK
    GPIO_Init(XPT2046_CLK_GPIO, &GPIO_InitStruct);               
    // MOSI                                                      
    GPIO_InitStruct.GPIO_Pin   = XPT2046_MOSI_PIN;               // MOSI
    GPIO_Init(XPT2046_MOSI_GPIO, &GPIO_InitStruct);              
    // MISO, 上拉输入                                            
    GPIO_InitStruct.GPIO_Pin   = XPT2046_MISO_PIN;               
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;                   // MISO, 上拉输入
    GPIO_Init(XPT2046_MISO_GPIO, &GPIO_InitStruct);              
    // IRQ                                                       
    GPIO_InitStruct.GPIO_Pin  = XPT2046_IRQ_PIN;                 
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;                    // 触摸屏触摸信号指示引脚，不使用中断
    GPIO_Init(XPT2046_IRQ_GPIO, &GPIO_InitStruct);
#endif

    CLK_LOW ;                                                    // XPT2046时序要求：CLK 闲时低电平，上升沿采样数据，下降沿改变数据
    MOSI_0;                                                      // XPT2046时序要求：MOSI闲时低电平
    CS_LOW;                                                      // 拉低片选，使XTP2046开始通信

    // 通过读取旧数据, 检查是否已校准
    uint8_t flashDATA[20] = {0};                                 // 用于存放读取的校准数据
    uint32_t addr = 0;                                           // 校准数据的地址
    if (xXPT2046.dir == 0)                                       // 竖屏
        addr = xXPT2046.dataAddr + 0;                            // 数据地址，偏移：0 个字节
    if (xXPT2046.dir == 1)                                       // 横屏
        addr = xXPT2046.dataAddr + 20;                           // 数据地址，偏移：20个字节

    W25Q128_ReadData(addr, flashDATA, 20);                       // 读取数据

    if ((flashDATA[0] == 'O') && (flashDATA[1] == 'K'))          // 判断是否存有已校准的数据
    {
        memcpy(&xXPT2046.xfac, flashDATA + 2, 4);
        xXPT2046.xoff = *(short *)(flashDATA + 6);
        memcpy(&xXPT2046.yfac, flashDATA + 10, 4);
        xXPT2046.yoff = *(short *)(flashDATA + 14);
#if 0
        printf("xfac=%f\r", xXPT2046.xfac);
        printf("xoff=%d\r", xXPT2046.xoff);
        printf("yfac=%f\r", xXPT2046.yfac);
        printf("yoff=%d\r", xXPT2046.yoff);
#endif
    }
    else                                                         // 没有读取到旧的校准数据
    {
#if 1
        if (xXPT2046.dir == 0)                                   // 竖屏
            writeCorrectingData(0.0675f, 0.091f, -18, -19);      // 预先写入竖屏的校正数据，以减少生产时手动校正操作
        else                                                     // 横屏
            writeCorrectingData(0.09184f, 0.077956f, 338, -45);  // 预先写入横屏的校正数据，以减少生产时手动校正操作
        delayUS(10000);                                          // 稍作延时
        SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;                // 软复位，重新运行
#else
        XPT2046_ReCalibration();                                 // 手动重新校准
#endif
    }

    XPT2046_Cmd(ENABLE);                                         // 打开触摸检测
}



/******************************************************************************
 * 函数名： XPT2046_ReCalibration
 * 功  能： 重新校准触摸屏,
 *          并把校准的数据存入内部FLASH, 以方便下次调用
 * 参  数： 无
 * 返  回： 0_校准成功
 *          1_校准失败
 ******************************************************************************/
uint8_t  XPT2046_ReCalibration(void)
{
    uint16_t pixelOff = 30;   // 偏移像素,用来画十字
    uint16_t adcX1, adcX2, adcX3, adcX4, adcY1, adcY2, adcY3, adcY4; // 记录校准过程中的坐标值
    float xfac = 0;
    float yfac = 0;
    short xoff = 0;
    short yoff = 0;
    uint16_t crossX = 0;      // 用于画十字线
    uint16_t crossY = 0;      // 用于画十字线
    char strTemp[30];
    uint16_t lcdWidth  = xXPT2046.lcdWidth;
    uint16_t lcdHeight = xXPT2046.lcdHeight;

    printf("\r\r触摸屏开始重新校准....\r");
    printf("请使用笔尖，依次点击黄色光标!\r\r");
    LCD_Fill(0, 0, lcdWidth, lcdHeight, BLACK);
    LCD_String(20, 90,  "Please use the pen , ", 16, WHITE, BLACK);
    LCD_String(20, 115, "and click on the reticle !", 16, WHITE, BLACK);

    // 左上角
    crossX = pixelOff;
    crossY = pixelOff;
    xXPT2046.adcX = 0;
    xXPT2046.adcY = 0;
    XPT2046_Cmd(ENABLE);                                              // 打开触屏检测(代码)
    LCD_Cross(crossX, crossY, 20, YELLOW);                            // 画十字
    while ((readAdcXY() == 0) || (xXPT2046.adcX == 0));               // 等待按下
    XPT2046_Cmd(DISABLE);                                             // 关闭触屏检测(代码)
    adcX1 = xXPT2046 .adcX;                                           // 记录下取得的adc值
    adcY1 = xXPT2046.adcY;                                            // 记录下取得的adc值
    LCD_Cross(crossX, crossY, 20, BLACK);                             // 抹去十字
    sprintf(strTemp, "X:%d", adcX1);
    LCD_String(crossX - 12, crossY - 16, strTemp, 12, YELLOW, BLACK); // 提示
    sprintf(strTemp, "Y:%d", adcY1);
    LCD_String(crossX - 12, crossY, strTemp, 12, YELLOW, BLACK);      // 提示
    delayUS(800000);

    // 右上角
    crossX = lcdWidth - pixelOff;
    crossY = pixelOff;
    xXPT2046.adcX = 0;
    xXPT2046.adcY = 0;
    XPT2046_Cmd(ENABLE);                                              // 打开触屏检测(代码)
    LCD_Cross(crossX, crossY, 20, YELLOW);                            // 画十字
    while ((readAdcXY() == 0) || (xXPT2046.adcX == 0));               // 等待按下
    XPT2046_Cmd(DISABLE);                                             // 关闭触屏检测(代码)
    adcX2 = xXPT2046 .adcX;                                           // 记录下取得的adc值
    adcY2 = xXPT2046.adcY;                                            // 记录下取得的adc值
    LCD_Cross(crossX, crossY, 20, BLACK);                             // 抹去十字
    sprintf(strTemp, "X:%d", adcX2);
    LCD_String(crossX - 12, crossY - 16, strTemp, 12, YELLOW, BLACK); // 提示
    sprintf(strTemp, "Y:%d", adcY2);
    LCD_String(crossX - 12, crossY, strTemp, 12, YELLOW, BLACK);      // 提示
    delayUS(800000);

    // 左下角
    crossX = pixelOff;
    crossY = lcdHeight - pixelOff;
    xXPT2046.adcX = 0;
    xXPT2046.adcY = 0;
    XPT2046_Cmd(ENABLE);                                              // 打开触屏检测(代码)
    LCD_Cross(crossX, crossY, 20, YELLOW);                            // 画十字
    while ((readAdcXY() == 0) || (xXPT2046.adcX == 0));               // 等待按下
    XPT2046_Cmd(DISABLE);                                             // 关闭触屏检测(代码)
    adcX3 = xXPT2046 .adcX;                                           // 记录下取得的adc值
    adcY3 = xXPT2046.adcY;                                            // 记录下取得的adc值
    LCD_Cross(crossX, crossY, 20, BLACK);                             // 抹去十字
    sprintf(strTemp, "X:%d", adcX3);
    LCD_String(crossX - 12, crossY - 16, strTemp, 12, YELLOW, BLACK); // 提示
    sprintf(strTemp, "Y:%d", adcY3);
    LCD_String(crossX - 12, crossY, strTemp, 12, YELLOW, BLACK);      // 提示
    delayUS(800000);

    // 右下角
    crossX = lcdWidth - pixelOff;
    crossY = lcdHeight - pixelOff;
    xXPT2046.adcX = 0;
    xXPT2046.adcY = 0;
    XPT2046_Cmd(ENABLE);                                              // 打开触屏检测(代码)
    LCD_Cross(crossX, crossY, 20, YELLOW);                            // 画十字
    while ((readAdcXY() == 0) || (xXPT2046.adcX == 0));               // 等待按下
    XPT2046_Cmd(DISABLE);                                             // 关闭触屏检测(代码)
    adcX4 = xXPT2046 .adcX;                                           // 记录下取得的adc值
    adcY4 = xXPT2046.adcY;                                            // 记录下取得的adc值
    LCD_Cross(crossX, crossY, 20, BLACK);                             // 抹去十字
    sprintf(strTemp, "X:%d", adcX4);
    LCD_String(crossX - 12, crossY - 16, strTemp, 12, YELLOW, BLACK); // 提示
    sprintf(strTemp, "Y:%d", adcY4);
    LCD_String(crossX - 12, crossY, strTemp, 12, YELLOW, BLACK);      // 提示
    delayUS(400000);

    //timeCNT=0;
    // 取adcX和adcY的平均值; 如果不取平均值, 在对角画两个十字线即可
    adcX1 = (adcX1 + adcX3) / 2;
    adcX2 = (adcX2 + adcX4) / 2;

    adcY1 = (adcY1 + adcY2) / 2;
    adcY2 = (adcY3 + adcY4) / 2;

    xfac = (float)(pixelOff - (lcdWidth - pixelOff)) / (adcX1 - adcX2);   // 触摸屏与LCD的坐标比例系数,  xfac=(float)(20-320)/(t1x-t2x);
    yfac = (float)(pixelOff - (lcdHeight - pixelOff)) / (adcY1 - adcY2);
    xoff = (lcdWidth - xfac * (adcX1 + adcX2)) / 2;                       // 像素点偏移值, xoff=(320-xfac*(t1x+t2x))/2;
    yoff = (lcdHeight - yfac * (adcY1 + adcY2)) / 2;

    // 存入FLASH中
    writeCorrectingData(xfac, yfac, xoff, yoff);

    xXPT2046.xfac = xfac;
    xXPT2046.xoff = xoff;
    xXPT2046.yfac = yfac;
    xXPT2046.yoff = yoff;

    printf(">>>校准完成! 比例系数、偏移值已存入内部FLASH, 地址:0x%X\r\r", xXPT2046.dataAddr);

    SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;  // 软复位，重新运行
    return 0;
}



/******************************************************************************
 * 函数名： XPT2046_Cmd
 * 功  能： 触摸检测开关
 *          由于检测比较耗时, 在不使用触屏的状态下, 可以关闭检测以节省芯片资源
 *          此开关状态, 只作用于XPT2046_TouchHandler();
 * 参  数： 0_关闭触摸屏的检测,以节省资源
 *          1_打开触摸检测
 * 返  回：
 ******************************************************************************/
void XPT2046_Cmd(uint8_t status)
{
    if (status != 0)
    {
        xXPT2046 .EN = 1;
    }
    else
    {
        xXPT2046.EN = 0;
    }
}



/******************************************************************************
 * 函数名： XPT2046_IsPressed
 * 功  能： 判断触摸屏是否有按下
 * 参  数： 无
 * 返  回： 0-未按下、1-按下中
 ******************************************************************************/
uint8_t XPT2046_IsPressed(void)
{
    static uint8_t status = 0;

    if (xXPT2046.EN == 0)             // 触摸检测开关; 初始化后默认是开启的; 可以手动关闭，以实现一些操作后，然后再手动开启
        return 0;

    status = IRQ_READ ? 0 : 1 ;       // 引脚闲时为高电平，按下时为低电平
    if (status == 1)                  // 如果已按下
    {
        readAdcXY();                  // 获取XPT2046的按下位置(电压值)
        adcXYToLcdXY();               // 换算成显示屏的坐标值; 换算后的XY值，可通过：XPT2046_GetX()与XPT2046_GetY()获取;
        uint16_t x1 = xXPT2046.lcdX;
        uint16_t y1 = xXPT2046.lcdY;

        readAdcXY();                  // 获取XPT2046的按下位置(电压值)
        adcXYToLcdXY();               // 换算成显示屏的坐标值; 换算后的XY值，可通过：XPT2046_GetX()与XPT2046_GetY()获取;
        uint16_t x2 = xXPT2046.lcdX;
        uint16_t y2 = xXPT2046.lcdY;

        // 计算采值差
        uint8_t x, y;
        if (x1 > x2)
            x = x1 - x2;
        else
            x = x2 - x1;

        if (y1 > y2)
            y = y1 - y2;
        else
            y = y2 - y1;

        if (x > 3 || y > 3)
            return 0;

        // 计算平均值
        xXPT2046.lcdX = (x1 + x2) >> 1;
        xXPT2046.lcdY = (y1 + y2) >> 1;
    }    
    
    return  status ;      // 返回：0-未按下、1-按下中             
}



/******************************************************************************
 * 函数名： XPT2046_GetX
 * 功  能： 获取按下位置的坐标值 (X)
 * 参  数： 无
 * 返  回： 坐标值 (X)
 ******************************************************************************/
uint16_t  XPT2046_GetX(void)
{
    return xXPT2046.lcdX;
}



/******************************************************************************
 * 函数名： XPT2046_GetY
 * 功  能： 获取按下位置的坐标值 (Y)
 * 参  数： 无
 * 返  回： 坐标值 (Y)
 ******************************************************************************/
uint16_t  XPT2046_GetY(void)
{
    return xXPT2046.lcdY;
}



/******************************************************************************
 * 函数名： XPT2046_TouchDown
 * 功  能： 触摸按下时的处理
 *          空白函数, 用户自行编写代码
 * 参  数：
 * 返  回：
 ******************************************************************************/
void XPT2046_TouchDown(void)
{
//    // 示例在触摸点画一个颜色点
//    LCD_DrawPoint(xXPT2046.lcdX, xXPT2046.lcdY, YELLOW);
//    // 示例，把坐标显示在LCD
//    static char strTem[20];
//    sprintf(strTem, "%3d  %3d", xXPT2046.lcdX, xXPT2046.lcdY);
//    LCD_String(180, 290, strTem, 12, WHITE, BLACK);
}



/******************************************************************************
 * 函数名： XPT2046_TouchUp
 * 功  能： 触摸按下时的处理
 *          空白函数, 用户自行编写代码
 * 参  数：
 * 返  回：
 ******************************************************************************/
void XPT2046_TouchUp(void)
{


}

