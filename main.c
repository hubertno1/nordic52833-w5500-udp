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

#define W5500_INT_PIN  NRF_GPIO_PIN_MAP(0,14)			// ����w5500���ж������� nrf52833 dk ��p0.14�� 

// w5500�жϱ�־λ�����w5500�жϴ������ñ�־λ�ᱻ��Ϊtrue����ѭ��һ����⵽�ñ�־λΪtrue���ͻ����̸�λ����жϱ�־λ��������w5500���ж��¼�
// ����жϱ�־λ����Ϊvolatile���������ڱ��������Ż���Ϊ������ܻ���Ϊ�˱�����ֵһֱ��false, �Ӷ���Ӱ����ѭ����w5500�жϵĴ���
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
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);		// ����dk��w5500���ӵ��ж�����Ϊ�½��ش���
    in_config.pull = NRF_GPIO_PIN_PULLUP;	// �����������裬��ֹ�ж��������յ��²��ȶ��������󴥷��жϻ򲻴����ж�
	nrf_drv_gpiote_in_init(W5500_INT_PIN, &in_config, w5500_itr_handler);		// ��ʼ���ж�����, ���ж�����1. ����Ϊ�½��ش�����2. �����������裬3. �����жϴ����� 4. ����������õĶ�Ӧ���ж�����
    nrf_drv_gpiote_in_event_enable(W5500_INT_PIN, true);	// ʹ���ж����ŵ��жϹ��ܣ������ʹ�ܣ���ʹ�ж����Ŵ������жϣ�Ҳ���ᴥ���жϴ�����

	spi_init();

	W5500_Hardware_Reset();		//Ӳ����λW5500
	Load_Net_Parameters();		//װ���������
	W5500_Initialization();		//W5500��ʼ������



	while (1)
	{
		W5500_Socket_Set();//W5500�˿ڳ�ʼ������

		if (w5500_itr_flag)
		{
			w5500_itr_flag = false;
			W5500_Interrupt_Process();//W5500�жϴ��������
			
			if((S0_Data & S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
			{
				S0_Data&=~S_RECEIVE;

				Process_Socket_Data(0);//W5500���ղ����ͽ��յ�������
			}
			
		
		}

	}

}
