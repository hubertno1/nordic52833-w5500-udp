#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "spi_driver.h"
#include "spi_config.h"
#include "w5500_config.h"
#include "w5500.h"

#define W5500_INT_PIN  NRF_GPIO_PIN_MAP(0,14)			// 配置w5500的中断引脚在 nrf52833 dk 的p0.14上 

// w5500中断标志位。如果w5500中断触发，该标志位会被置为true，主循环一旦检测到该标志位为true，就会立刻复位这个中断标志位。并处理w5500的中断事件
// 这个中断标志位必须为volatile，否则由于编译器的优化行为，其可能会认为此变量的值一直是false, 从而会影响主循环对w5500中断的处理
volatile bool w5500_itr_flag = false;					


void w5500_itr_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	w5500_itr_flag = true;
}


int main(void)
{
	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
	NRF_LOG_DEFAULT_BACKENDS_INIT();

	nrf_drv_gpiote_init();			
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);		// 配置dk与w5500连接的中断引脚为下降沿触发
    in_config.pull = NRF_GPIO_PIN_PULLUP;	// 配置上拉电阻，防止中断引脚悬空导致不稳定而导致误触发中断或不触发中断
	nrf_drv_gpiote_in_init(W5500_INT_PIN, &in_config, w5500_itr_handler);		// 初始化中断引脚, 将中断引脚1. 设置为下降沿触发，2. 设置上拉电阻，3. 设置中断处理函数 4. 设置这个配置的对应的中断引脚
    nrf_drv_gpiote_in_event_enable(W5500_INT_PIN, true);	// 使能中断引脚的中断功能，如果不使能，即使中断引脚触发了中断，也不会触发中断处理函数

	spi_init();

	W5500_Hardware_Reset();		//硬件复位W5500
	Load_Net_Parameters();		//装载网络参数
	W5500_Initialization();		//W5500初始化配置



	while (1)
	{
		W5500_Socket_Set();//W5500端口初始化配置

		if (w5500_itr_flag)
		{
			w5500_itr_flag = false;
			W5500_Interrupt_Process();//W5500中断处理程序框架
			
			if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
			{
				S0_Data&=~S_RECEIVE;

				Process_Socket_Data(0);//W5500接收并发送接收到的数据
			}
			
		
		}

	}

}
