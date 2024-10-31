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

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(W5500_INT_PIN, &in_config, w5500_int_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(W5500_INT_PIN, true);

	spi_init();

	Load_Net_Parameters();		//装载网络参数
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_Initialization();		//W5500初始化配置

	while (1)
	{
		W5500_Socket_Set();//W5500端口初始化配置

		if (w5500_int_flag)
		{
			w5500_int_flag = false;
			W5500_Interrupt_Process();//W5500中断处理程序框架
			
			if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
			{
				S0_Data&=~S_RECEIVE;

				Process_Socket_Data(0);//W5500接收并发送接收到的数据
			}
			
			// 添加发送完成处理
			if((S0_Data & S_TRANSMITOK) == S_TRANSMITOK)
			{
				S0_Data &= ~S_TRANSMITOK;
			}
		
		}

	}

}

