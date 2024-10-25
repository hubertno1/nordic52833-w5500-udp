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


int main(void)
{
		APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
		NRF_LOG_DEFAULT_BACKENDS_INIT();	
		
		spi_init();

		Load_Net_Parameters();		//装载网络参数
		W5500_Hardware_Reset();		//硬件复位W5500
		W5500_Initialization();		//W5500初始化配置
	
		while (1)
    	{	
			W5500_Socket_Set();//W5500端口初始化配置


			W5500_Interrupt_Process();//W5500中断处理程序框架

			if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
			{
				S0_Data&=~S_RECEIVE;
				Process_Socket_Data(0);//W5500接收并发送接收到的数据
			}
	
    	}

}
