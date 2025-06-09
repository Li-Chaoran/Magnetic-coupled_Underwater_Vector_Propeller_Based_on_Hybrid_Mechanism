#include "bsp_W25Q128.h"




//W25Q系列芯片型号返回值
#define    W25Q80            0XEF13
#define    W25Q16            0XEF14
#define    W25Q32            0XEF15
#define    W25Q64            0XEF16
#define    W25Q128           0XEF17
#define    W25Q256           0XEF18
//#define  W25Qxx    65519    // 很多时候重新下载后读出的都是65519
#define    W25Q128_CS_HIGH    (W25Q128_CS_GPIO -> BSRR =  W25Q128_CS_PIN)
#define    W25Q128_CS_LOW     (W25Q128_CS_GPIO -> BSRR =  W25Q128_CS_PIN << 16)



xW25Q_TypeDef  xW25Q128;      // 声明全局结构体, 用于记录w25qxx信息





// 5_1 发送1字节,返回1字节
// SPI通信,只一个动作:向DR写入从设命令值,同步读出数据!写读组合,按从设时序图来. 作为主设,因为收发同步,连接收发送中断也不用开,未验证其它中断对其工作的影响.
static uint8_t  sendByte(uint8_t d)
{
    uint16_t retry = 0;

    while ((W25Q128_SPI ->SR & 2) == 0)      // 等待发送区为空
    {
        retry++;
        if (retry > 1000)    return 0;
    }
    W25Q128_SPI ->DR = d;

    retry = 0;
    while ((W25Q128_SPI->SR & 1) == 0)       // 等待接收完数据
    {
        retry++;
        if (retry > 1000)    return 0;
    }
    return W25Q128_SPI->DR ;
}



// 5_2 写使能
static void writeEnable()
{
    W25Q128_CS_LOW ;
    sendByte(0x6);                           // 命令: Write Enable : 06h
    W25Q128_CS_HIGH ;
}



// 5_3 等待空闲
static void WaitReady()
{
    W25Q128_CS_LOW ;

    sendByte(0x05);                          // 命令: Read Status Register : 05h
    while (sendByte(0xFF) & 1) {}            // 只要发送读状态寄存器指令，芯片就会持续向主机发送最新的状态寄存器内容 ，直到收到通信的停止信号。

    W25Q128_CS_HIGH ;
}



// 5_4 擦除一个扇区, 每扇区>150ms
static void eraseSector(uint32_t addr)
{
    if (xW25Q128.FlagInit == 0) return;      // 如果W25Qxx初始化失败，则跳过检测，防止卡死

    addr = addr * 4096;                      // 从第几扇区开始

    writeEnable();
    WaitReady();
    // 命令
    W25Q128_CS_LOW ;
    sendByte(0x20);                          // 命令: Sector Erase(4K) : 20h
    sendByte((uint8_t)(addr >> 16));
    sendByte((uint8_t)(addr >> 8));
    sendByte((uint8_t)addr);
    W25Q128_CS_HIGH ;

    WaitReady();
}



// 5_5 写扇区. 要分页写入
static void writeSector(uint32_t addr, uint8_t *p, uint16_t num)
{
    if (xW25Q128.FlagInit == 0) return;           // 如果W25Qxx初始化失败，则跳过检测，防止卡死

    uint16_t pageRemain = 256;                    // 重要，重要，重要：W25Qxx每个页命令最大写入字节数:256字节;

    for (char i = 0; i < 16; i++)                 // 扇区:4096bytes, 缓存页:256bytes, 写扇区要分16次页命令写入
    {
        writeEnable();                            // 写使能
        WaitReady();                              // 等待空闲

        W25Q128_CS_LOW ;                          // 低电平,开始
        sendByte(0x02);                           // 命令: page program : 02h , 每个写页命令最大缓存256字节
        sendByte((uint8_t)(addr >> 16));          // 地址
        sendByte((uint8_t)(addr >> 8));
        sendByte((uint8_t)addr);
        for (uint16_t i = 0; i < pageRemain; i++) // 发送写入的数据
            sendByte(p[i]);                       // 高电平, 结束
        W25Q128_CS_HIGH ;

        WaitReady();                              // 等待空闲

        p = p + pageRemain;                       // 缓存指针增加一页字节数
        addr = addr + pageRemain ;                // 写地址增加一页字节数
    }
}



// 读取芯片型号
static uint32_t readID(void)
{
    uint16_t Temp = 0;
    W25Q128_CS_LOW;
    sendByte(0x90);//发送读取ID命令
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    Temp |= sendByte(0xFF) << 8;
    Temp |= sendByte(0xFF);
    W25Q128_CS_HIGH;

    xW25Q128.FlagInit  = 1;
    switch (Temp)
    {
        case W25Q16:
            sprintf((char *)xW25Q128.type, "%s", "W25Q16");
            break;
        case W25Q32:
            sprintf((char *)xW25Q128.type, "%s", "W25Q32");
            break;
        case W25Q64:
            sprintf((char *)xW25Q128.type, "%s", "W25Q64");
            break;
        case W25Q128:
            sprintf((char *)xW25Q128.type, "%s", "W25Q128");
            break;
        case W25Q256:
            sprintf((char *)xW25Q128.type, "%s", "W25Q256");          // 注意:W25Q256的地址是4字节
            break;
        default:
            sprintf((char *)xW25Q128.type, "%s", "Flash设备失败 !!!");
            xW25Q128.FlagInit = 0;
            printf("读取到的错误型号数据：%d\r\n", Temp);
            break;
    }

    if (xW25Q128.FlagInit  == 1)
        printf("Flash存储 检测...        型号:%s\r", xW25Q128.type);
    else
        printf("数据存储检测：           型号读取错误，设备不可用!\r");

    return Temp;
}



// 检查字库样本的正确性
static void checkFlagGBKStorage(void)
{
    if (xW25Q128 .FlagInit == 0) return;                  // 如果W25Qxx初始化失败，则跳过检测，防止卡死

    printf("GBK字库 测试...          ");
    uint8_t sub = 0;
    uint8_t f = 0 ;

    for (uint32_t i = 0; i < 6128640; i = i + 1000000)
    {
        W25Q128_ReadData(GBK_STORAGE_ADDR + i, &f, 1);
        sub = sub + f;                                    // 80 , 0, 98, 79, 0, 1, 0
    }
    xW25Q128.FlagGBKStorage = (sub == 146 ? 1 : 0);       // 判断是否有字库,打开地址写保护, 防止字库被错误写入履盖

    if (xW25Q128.FlagGBKStorage == 1)
        printf("字库可用\r");                             // 标记字库可用
    else
        printf(" 错误，字库不可用!\r");
}



/******************************************************************************
 * 函  数： W25qx_Init
 * 功  能： 初始化W25Q128所需引脚、SPI
 * 参  数：
 * 返回值： 初始化结果，0:失败、1:成功
 ******************************************************************************/
uint8_t W25Q128_Init(void)
{  
    // CS引脚时钟 使能
    if (W25Q128_CS_GPIO == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    if (W25Q128_CS_GPIO == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    if (W25Q128_CS_GPIO == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    if (W25Q128_CS_GPIO == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    if (W25Q128_CS_GPIO == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    if (W25Q128_CS_GPIO == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    if (W25Q128_CS_GPIO == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    // SPI引脚时钟 使能
    if (W25Q128_SCK_GPIO == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    if (W25Q128_SCK_GPIO == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    if (W25Q128_SCK_GPIO == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    if (W25Q128_SCK_GPIO == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    if (W25Q128_SCK_GPIO == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    if (W25Q128_SCK_GPIO == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    if (W25Q128_SCK_GPIO == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    // SPI时钟 使能
    if (W25Q128_SPI == SPI1)  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    if (W25Q128_SPI == SPI2)  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    if (W25Q128_SPI == SPI3)  RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
    
#ifdef USE_HAL_DRIVER                                                // HAL库
    // 配置引脚: CS                                                  
    GPIO_InitTypeDef    GPIO_InitStruct = {0};                       // 声明初始化要用到的结构体
    GPIO_InitStruct.Pin   = W25Q128_CS_PIN;                          // 引脚
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;                     // 工作模式
    GPIO_InitStruct.Pull  = GPIO_PULLUP;                             // 上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;               // 引脚速率
    HAL_GPIO_Init(W25Q128_CS_GPIO, &GPIO_InitStruct);                // 初始化引脚工作模式
    W25Q128_CS_HIGH;                                                 // CS引脚拉高：停止信号
    // 配置 SPI引脚: SCK、MISO、MOSI
    GPIO_InitStruct.Pin   = W25Q128_SCK_PIN | W25Q128_MISO_PIN | W25Q128_MOSI_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;                         // 工作模式
    GPIO_InitStruct.Pull  = GPIO_PULLUP;                             // 上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;               // 引脚速率
    GPIO_InitStruct.Alternate = W25Q128_SPI_AFx;                     // 引脚复用功能
    HAL_GPIO_Init(W25Q128_SCK_GPIO, &GPIO_InitStruct);               // 初始化引脚工作模式
#endif                                                               
                                                                     
#ifdef USE_STDPERIPH_DRIVER                                          // 标准库
    // 配置引脚: CS                                                  
    GPIO_InitTypeDef    GPIO_InitStruct = {0};                       // 声明初始化要用到的结构体
    GPIO_InitStruct.GPIO_Pin = W25Q128_CS_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(W25Q128_CS_GPIO, &GPIO_InitStruct);
    W25Q128_CS_HIGH;                                                 // CS引脚拉高：停止信号 
    // 配置 SPI引脚: SCK、MISO、MOSI
    GPIO_InitStruct.GPIO_Pin = W25Q128_SCK_PIN | W25Q128_MISO_PIN | W25Q128_MOSI_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(W25Q128_SCK_GPIO, &GPIO_InitStruct);
    //设置引脚复用
    GPIO_PinAFConfig(W25Q128_SCK_GPIO,  W25Q128_SCK_PINSOURCE,  W25Q128_SCK_AF);
    GPIO_PinAFConfig(W25Q128_MISO_GPIO, W25Q128_MISO_PINSOURCE, W25Q128_MISO_AF);
    GPIO_PinAFConfig(W25Q128_MOSI_GPIO, W25Q128_MOSI_PINSOURCE, W25Q128_MOSI_AF);
#endif
    
    // 配置SPI工作模式
    W25Q128_SPI -> CR1  = 0x1 << 0;                           // CPHA:时钟相位,0x1=在第2个时钟边沿进行数据采样
    W25Q128_SPI -> CR1 |= 0x1 << 1;                           // CPOL:时钟极性,0x1=空闲状态时，SCK保持高电平
    W25Q128_SPI -> CR1 |= 0x1 << 2;                           // 主从模式:         1 = 主配置
    W25Q128_SPI -> CR1 |= 0x0 << 3;                           // 波特率控制[5:3]:  0 = fPCLK /2
    W25Q128_SPI -> CR1 |= 0x0 << 7;                           // 帧格式:           0 = 先发送MSB
    W25Q128_SPI -> CR1 |= 0x1 << 9;                           // 软件从器件管理 :  1 = 使能软件从器件管理(软件NSS)
    W25Q128_SPI -> CR1 |= 0x1 << 8;                           // 内部从器件选择,根据9位设置(失能内部NSS)
    W25Q128_SPI -> CR1 |= 0x0 << 11;                          // 数据帧格式,       0 = 8位
    W25Q128_SPI -> CR1 |= 0x1 << 6;                           // SPI使能           1 = 使能外设

    readID();                                                 // 读取芯片型号,判断通讯是否正常
    checkFlagGBKStorage();                                    // 检查字库

    if (xW25Q128.FlagInit)
        return 1;                                             // 初始化成功，返回:1

    return 0;                                                 // 初始化失败，返回:0
}



/******************************************************************************
 * 函  数： W25Q128_ReadData
 * 功  能： 读取数据
 * 参  数： uint32_t addr  数据在W25Q128内的地址
 *          uint8_t *pData 数据缓存地址
 *          uint16_t num   连续读取的字节数
 * 返回值： 无
 ******************************************************************************/
void W25Q128_ReadData(uint32_t addr, uint8_t *pData, uint16_t num)
{
    if (xW25Q128 .FlagInit == 0) return;  // 如果W25Qxx初始化失败，则跳过检测，防止卡死

    W25Q128_CS_LOW ;
    sendByte(0x03);                       // 发送读取命令 03h
    sendByte((uint8_t)(addr >> 16));
    sendByte((uint8_t)(addr >> 8));
    sendByte((uint8_t)addr);

    for (uint32_t i = 0; i < num; i++)
    {
        pData[i] = sendByte(0xFF);
    }

    W25Q128_CS_HIGH ;
}



/******************************************************************************
 * 函数名： W25Q128_WriteData
 * 功  能： 从addr处起，读取num个字节，存放到缓存p
 * 参  数： uint32_t  addr   写入地址         (W25Q128 只用3字节, W25Q256用4字节)
 *          uint8_t  *pData  要写入的数据存储区
 *          uint16_t  num    写入的字节数
 * 返  回： 无
 * 备  注： 最后更新_2020年12月15日
 ******************************************************************************/
uint8_t W25Q128_buffer[4096];                         // 开辟一段内存空间

void W25Q128_WriteData(uint32_t addr, uint8_t *pData, uint16_t num)
{
    if (xW25Q128.FlagInit == 0) return ;              // 如果w25qxx设备初始化失败，则跳过本函数，防止卡死

    // 字库段写保护, 防止字库被错误写入履盖
    if (((addr + num) > 0x00A00000) && (xW25Q128.FlagGBKStorage == 1))
    {
        printf("要写入的数据在字库数据存储区内，已跳过本次操作!!\r");
        return;
    }

    uint32_t  secPos      = addr / 4096;              // 扇区地址,第几个扇区
    uint16_t  secOff      = addr % 4096;              // 开始地始偏移字节数: 数据在扇区的第几字节存放
    uint16_t  secRemain   = 4096 - secOff;            // 扇区剩余空间字节数 ,用于判断够不够存放余下的数据
    uint8_t  *buf = W25Q128_buffer;                   // 原子哥代码,为什么不直接使用所声明的数组.

    //spiInit();                                      // 每次读写前，先重新配置SPI，避免多个设备共用一SPI时的配置不同
    if (num <= secRemain) secRemain = num;
    while (1)
    {
        W25Q128_ReadData(secPos * 4096, buf, 4096);   // 读取扇区内容到缓存

        eraseSector(secPos);                          // 擦扇区
        for (uint16_t i = 0; i < secRemain ; i++)     // 原始数据写入缓存
            buf[secOff + i] = pData[i];
        writeSector(secPos * 4096, buf, 4096);        // 缓存数据写入设备

        if (secRemain == num)                         // 已全部写入
            break;
        else
        {
            // 未写完
            pData = pData + secRemain ;               // 原始数据指针偏移
            secPos ++;                                // 新扇区
            secOff = 0;                               // 新偏移位,扇区内数据起始地址
            num = num - secRemain ;                   // 剩余未写字节数
            secRemain = (num > 4096) ? 4096 : num;    // 计算新扇区写入字节数
        }
    }
}



/******************************************************************************
 * 函数名： W25Q128_ReadFontData
 * 功  能： 从w25Q128的字库中读取出汉字字模数据
 * 参  数： uint8_t *pFont     汉字
 *          uint8_t  size      字体大小 12/16/24/32
 *          uint8_t *fontData  读取到的字模点阵数据
 * 返  回： 无
 * 备  注： 最后更新_2024年02月05日
 ******************************************************************************/
void W25Q128_ReadFontData(uint8_t *pFont, uint8_t size, uint8_t *fontData)
{
    uint8_t qh, ql;
    uint32_t foffset;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);                           // 计算汉字点阵大小，单位字节数

    qh = *pFont;                                                                          // 汉字GBK的第一个字节
    ql = *(++pFont);                                                                      // 汉字GBK的第二个字节

    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)                               // 不在字库内的汉字，将用填充显示整个位置
    {
        for (uint8_t i = 0; i < csize; i++) *fontData++ = 0x00;                           // 填充满格
        return;                                                                           // 返回
    }

    if (ql < 0x7f)                                                                        // 计算要提取的汉字在字库中的偏移位置
        ql -= 0x40;
    else
        ql -= 0x41;
    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * csize;                                     // 得到汉字在字库中的偏移位置

    switch (size)                                                                         // 按字体的不同，在不同字库读取字体点阵
    {
        case 12:
            W25Q128_ReadData(foffset + GBK_STORAGE_ADDR + 0x00000000, fontData, csize);     // 12号字体
            break;
        case 16:
            W25Q128_ReadData(foffset + GBK_STORAGE_ADDR + 0x0008c460, fontData, csize);   // 16号字体
            break;
        case 24:
            W25Q128_ReadData(foffset + GBK_STORAGE_ADDR + 0x001474E0, fontData, csize);   // 24号字体
            break;
        case 32:
            W25Q128_ReadData(foffset + GBK_STORAGE_ADDR + 0x002EC200, fontData, csize);   // 32号字体
            break;
    }
}

