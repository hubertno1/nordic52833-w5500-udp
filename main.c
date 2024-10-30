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


// �������ڲ����� GPIO ����
#define TIMING_PIN  NRF_GPIO_PIN_MAP(0,13) // ѡ��һ�����õ�����

int main(void)
{
	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
	NRF_LOG_DEFAULT_BACKENDS_INIT();

	// ��ʼ�����ڲ����� GPIO ����
    nrf_gpio_cfg_output(TIMING_PIN);
    nrf_gpio_pin_clear(TIMING_PIN); // ȷ�����ų�ʼΪ�͵�ƽ

	spi_init();

	Load_Net_Parameters();		//װ���������
	W5500_Hardware_Reset();		//Ӳ����λW5500
	W5500_Initialization();		//W5500��ʼ������

	while (1)
	{
		// nrf_gpio_pin_set(TIMING_PIN);	
		
		W5500_Socket_Set();//W5500�˿ڳ�ʼ������

		W5500_Interrupt_Process();//W5500�жϴ��������


		if((S0_Data & S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
		{
			S0_Data&=~S_RECEIVE;

			Process_Socket_Data(0);//W5500���ղ����ͽ��յ�������
			// nrf_gpio_pin_clear(TIMING_PIN);	
			//nrf_delay_ms(10);
			

		}

	}

}
