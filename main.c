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

/*******************************************************************************
* ������  : W5500_Initialization
* ����    : W5500��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʼ��W5500�Ĵ�������,������ط�����,ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();		//��ʼ��W5500�Ĵ�������
	Detect_Gateway();	//������ط����� 
	Socket_Init(0);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
}

/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢�����ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 192;//�������ز���
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 100;

	Sub_Mask[0]=255;//������������
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//���������ַ
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=192;//���ر���IP��ַ
	IP_Addr[1]=168;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	Phy_Addr[4]=IP_Addr[2];
	Phy_Addr[5]=IP_Addr[3];
	
	S0_Port[0] = 0x13;//���ض˿�0�Ķ˿ں�5000 
	S0_Port[1] = 0x88;

	S0_Mode=UDP_MODE;//���ض˿�0 ��socket 0���Ĺ���ģʽ,UDPģʽ
}

/*******************************************************************************
* ������  : W5500_Socket_Set
* ����    : W5500�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//�˿�0��ʼ������
	{
		if(S0_Mode==TCP_SERVER)//TCP������ģʽ 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP�ͻ���ģʽ 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDPģʽ 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}
/*******************************************************************************
* ������  : Process_Socket_Data
* ����    : W5500���ղ����ͽ��յ�������
* ����    : s:�˿ں�
* ���    : ��
* ����ֵ  : ��
* ˵��    : �������ȵ���S_rx_process()��W5500�Ķ˿ڽ������ݻ�������ȡ����,
*			Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���
*			������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
*			�������ݡ�
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	// ��socket���ջ�������ȡ����
	size=Read_SOCK_Data_Buffer(s, Rx_Buffer);

	// ��ȡĿ��IP��ַ�Ͷ˿ں�
	UDP_DIPR[0] = Rx_Buffer[0];
	UDP_DIPR[1] = Rx_Buffer[1];
	UDP_DIPR[2] = Rx_Buffer[2];
	UDP_DIPR[3] = Rx_Buffer[3];

	UDP_DPORT[0] = Rx_Buffer[4];
	UDP_DPORT[1] = Rx_Buffer[5];

	// �����յ������ݸ��Ƶ����ͻ�����
	memcpy(Tx_Buffer, Rx_Buffer+8, size-8);
	// ������������д��W5500��socket���ͻ�����			
	Write_SOCK_Data_Buffer(s, Tx_Buffer, size-8);
}


/**
 * @brief Work-around for transmitting 1 byte with SPIM.
 *
 * @param spim: The SPIM instance that is in use.
 * @param ppi_channel: An unused PPI channel that will be used by the workaround.
 * @param gpiote_channel: An unused GPIOTE channel that will be used by the workaround.
 * 
 * @warning Must not be used when transmitting multiple bytes.
 * @warning After this workaround is used, the user must reset the PPI channel and the
 GPIOTE channel before attempting to transmit multiple bytes.
 */
void setup_workaround_for_ftpan_58(NRF_SPIM_Type * spim, uint32_t ppi_channel, uint32_t gpiote_channel)
{
	 // Create an event when SCK toggles.
	 NRF_GPIOTE->CONFIG[gpiote_channel] = (
	 GPIOTE_CONFIG_MODE_Event <<
	 GPIOTE_CONFIG_MODE_Pos
	 ) | (
	 spim->PSEL.SCK <<
	 GPIOTE_CONFIG_PSEL_Pos
	 ) | (
	 GPIOTE_CONFIG_POLARITY_Toggle <<
	 GPIOTE_CONFIG_POLARITY_Pos
	 );
	 // Stop the spim instance when SCK toggles.
	 NRF_PPI->CH[ppi_channel].EEP = (uint32_t)&NRF_GPIOTE->EVENTS_IN[gpiote_channel];
	 NRF_PPI->CH[ppi_channel].TEP = (uint32_t)&spim->TASKS_STOP;
	 NRF_PPI->CHENSET = 1U << ppi_channel;
	 // The spim instance cannot be stopped mid-byte, so it will finish
	 // transmitting the first byte and then stop. Effectively ensuring
	 // that only 1 byte is transmitted.
}
void spi_ppi_set(void)
{
	setup_workaround_for_ftpan_58(NRF_SPIM0, 6, 0);
}



int main(void)
{
	
		bsp_board_init(BSP_INIT_LEDS);

		APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
		NRF_LOG_DEFAULT_BACKENDS_INIT();	
		
		spi_init();
		Load_Net_Parameters();		//װ���������	
		W5500_Hardware_Reset();		//Ӳ����λW5500
		W5500_Initialization();		//W5500��ʼ������
	
		while (1)
    	{	
				W5500_Socket_Set();//W5500�˿ڳ�ʼ������
				

				W5500_Interrupt_Process();//W5500�жϴ��������

				if((S0_Data & S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
				{
					S0_Data&=~S_RECEIVE;
					Process_Socket_Data(0);//W5500���ղ����ͽ��յ�������
				}
	
		
    	}

}
