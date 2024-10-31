#include "boards.h"
#include "spi_driver.h"
#include "spi_config.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_gpio.h"
#include <stddef.h>
#include "lpback_test.h"
#include "w5500_config.h"
#include "w5500.h"
#include "nrf_drv_timer.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "test_spi_speed.h"



#define TIMING_PIN  NRF_GPIO_PIN_MAP(0,13) 
#define W5500_INT_PIN  NRF_GPIO_PIN_MAP(0,14) 

volatile uint32_t w5500_int = 0;

volatile bool w5500_int_flag = false;

void w5500_int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	w5500_int++;
	w5500_int_flag = true;
}


int main(void)
{
	ret_code_t err_code;


	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
	NRF_LOG_DEFAULT_BACKENDS_INIT();

	// 初始化 GPIOTE 模块
    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

	// 配置 W5500 INT 引脚为输入，上拉，下降沿中断（低电平有效）
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(W5500_INT_PIN, &in_config, w5500_int_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(W5500_INT_PIN, true);

	// 初始化用于测量的 GPIO 引脚
    nrf_gpio_cfg_output(TIMING_PIN);
    nrf_gpio_pin_clear(TIMING_PIN); // 确保引脚初始为低电平

	spi_init();

	Load_Net_Parameters();		//装载网络参数
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_Initialization();		//W5500初始化配置

	while (1)
	{
		// nrf_gpio_pin_set(TIMING_PIN);	
		
		W5500_Socket_Set();//W5500端口初始化配置

		if (w5500_int_flag)
		{
			w5500_int_flag = false;
			W5500_Interrupt_Process();//W5500中断处理程序框架
		
			//  打印中断次数
            // NRF_LOG_INFO("Interrupt count: %d", w5500_int);
            // NRF_LOG_FLUSH();
			
			if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
			{
				S0_Data&=~S_RECEIVE;

				Process_Socket_Data(0);//W5500接收并发送接收到的数据
				// nrf_gpio_pin_clear(TIMING_PIN);	
				//nrf_delay_ms(10);
				
				NRF_LOG_FLUSH();
			}
			
			// 添加发送完成处理
			if((S0_Data & S_TRANSMITOK) == S_TRANSMITOK)
			{
				S0_Data &= ~S_TRANSMITOK;
				
				// 处理发送完成事件
				// 例如：更新发送状态、发送下一包数据等
			}
		
		}

	}

}

