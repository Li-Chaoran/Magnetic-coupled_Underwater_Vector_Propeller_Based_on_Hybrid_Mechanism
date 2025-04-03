#include "driver_timer.h"

#include "stm32f4xx_hal.h"
#include "cmsis_version.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
//#define USE_SYSTICK

extern TIM_HandleTypeDef        htim4;

/**********************************************************************
 * 函数名称： udelay
 * 功能描述： us级别的延时函数
 * 输入参数： us - 延时多少us
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void udelay(int us)
{
	uint32_t told = __HAL_TIM_GET_COUNTER(&htim6); // timer6的计数值
	uint32_t tnow;
	uint32_t load =  __HAL_TIM_GET_AUTORELOAD(&htim6); // timer6的auto-reload值
	
	/* LOAD+1个时钟对应1ms
	 * n us对应 n*(load+1)/1000个时钟
   */
	uint32_t ticks = us*(load+1)/1000;
	
	uint32_t cnt = 0;
	
	while (1)
	{
		tnow = __HAL_TIM_GET_COUNTER(&htim6);
		if (tnow >= told)
			cnt += tnow - told;
		else
			cnt += load + 1 - told + tnow;
		
		told = tnow;
		if (cnt >= ticks)
			break;
	}	
}

/**********************************************************************
 * 函数名称： mdelay
 * 功能描述： ms级别的延时函数
 * 输入参数： ms - 延时多少ms
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void mdelay(int ms)
{
	for (int i = 0; i < ms; i++)
		udelay(1000);
}

/**********************************************************************
 * 函数名称： system_get_ns
 * 功能描述： 获得系统时间(单位ns)
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 系统时间(单位ns)
 ***********************************************************************/
uint64_t system_get_ns(void)
{
	uint64_t ns = HAL_GetTick(); /* ms */
	ns = ns*1000000;

	uint32_t tnow = __HAL_TIM_GET_COUNTER(&htim6); // timer6的计数值
	uint32_t load = __HAL_TIM_GET_AUTORELOAD(&htim6); // timer6的auto-reload值
	
	uint64_t cnt;
	
	cnt = tnow; /* 没有考虑tnow等于0的情况 */
	
	ns += cnt * 1000000 / (load+1) ;
	return ns;
}



