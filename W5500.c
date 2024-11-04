/**********************************************************************************
 * �ļ���  ��W5500.c
 * ����    ��W5500 ����������         
 * ��汾  ��ST_v3.5

 * �Ա�    ��http://yixindianzikeji.taobao.com/
**********************************************************************************/

#include "W5500.h"	
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "spi_driver.h"
#include "string.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


/***************----- ��������������� -----***************/
unsigned char Gateway_IP[4];//����IP��ַ 
unsigned char Sub_Mask[4];	//�������� 
unsigned char Phy_Addr[6];	//�����ַ(MAC) 
unsigned char IP_Addr[4];	//����IP��ַ 

unsigned char S0_Port[2];	//�˿�0�Ķ˿ں�(5000) 

unsigned char UDP_DIPR[4];	//UDP(�㲥)ģʽ,Ŀ������IP��ַ
unsigned char UDP_DPORT[2];	//UDP(�㲥)ģʽ,Ŀ�������˿ں�

/***************----- �˿ڵ�����ģʽ -----***************/
unsigned char S0_Mode =3;	//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
#define TCP_SERVER	0x00	//TCP������ģʽ
#define TCP_CLIENT	0x01	//TCP�ͻ���ģʽ 
#define UDP_MODE	0x02	//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
unsigned char S0_State =0;	//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������) 
#define S_INIT		0x01	//�˿���ɳ�ʼ�� 
#define S_CONN		0x02	//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
unsigned char S0_Data;		//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
#define S_RECEIVE	 0x01	//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK 0x02	//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
unsigned char Rx_Buffer[4096];	//�˿ڽ������ݻ����� 
unsigned char Tx_Buffer[4096];	//�˿ڷ������ݻ����� 

/*******************************************************************************
* ������  : Write_W5500_1Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	
	spi_cs_enable();
	spi_write_short(reg);
	spi_write_byte(FDM1|RWB_WRITE|COMMON_R);
	spi_write_byte(dat);
	spi_cs_disable();

}

/*******************************************************************************
* ������  : Write_W5500_2Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д2���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ
		
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM2|RWB_WRITE|COMMON_R);//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	spi_write_short(dat);//д16λ����

	spi_cs_disable(); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_nByte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���дn���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,*dat_ptr:��д�����ݻ�����ָ��,size:��д������ݳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ	
		
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(VDM|RWB_WRITE|COMMON_R);//ͨ��SPI1д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���

	for(i=0;i<size;i++)//ѭ������������size���ֽ�����д��W5500
	{
		spi_write_byte(*dat_ptr++);//дһ���ֽ�����
	}

	spi_cs_disable(); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_1Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ	
		
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM1|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	spi_write_byte(dat);//д1���ֽ�����

	spi_cs_disable(); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_2Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ
			
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM2|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	spi_write_short(dat);//д16λ����

	spi_cs_disable(); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_4Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д4���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,*dat_ptr:��д���4���ֽڻ�����ָ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ
			
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM4|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,4���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

	spi_write_byte(*dat_ptr++);//д��1���ֽ�����
	spi_write_byte(*dat_ptr++);//д��2���ֽ�����
	spi_write_byte(*dat_ptr++);//д��3���ֽ�����
	spi_write_byte(*dat_ptr++);//д��4���ֽ�����

	spi_cs_disable(); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Read_W5500_1Byte
* ����    : ��W5500ָ����ַ�Ĵ�����1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;

	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ
			
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM1|RWB_READ|COMMON_R);//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���

	spi_read_byte(&i);

	spi_cs_disable();//��W5500��SCSΪ�ߵ�ƽ
	return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_1Byte
* ����    : ��W5500ָ���˿ڼĴ�����1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ
			
	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM1|RWB_READ|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

	spi_read_byte(&i);

	spi_cs_disable();//��W5500��SCSΪ�ߵ�ƽ
	return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_2Byte
* ����    : ��W5500ָ���˿ڼĴ�����2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����2���ֽ�����(16λ)
* ˵��    : ��
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	uint16_t value;

	spi_cs_enable();  //��W5500��SCSΪ�͵�ƽ

	spi_write_short(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
	spi_write_byte(FDM2|RWB_READ|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���
	spi_read_short(&value);
	
	spi_cs_disable();//��W5500��SCSΪ�ߵ�ƽ

	return value;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_SOCK_Data_Buffer
* ����    : ��W5500�������ݻ������ж�ȡ����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��
* ���    : ��
* ����ֵ  : ��ȡ�������ݳ���,rx_size���ֽ�
* ˵��    : ��
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	// 1. ��鲢��ȡ�ɶ����ݴ�С
	uint16_t rx_size = Read_W5500_SOCK_2Byte(s, Sn_RX_RSR);
	if (rx_size == 0)
	{
		return 0;
	}
	rx_size = (rx_size > 1460) ? 1460 : rx_size;			/* ���Ƶ��ζ�ȡ��С ��MTU���ƣ�*/
	
	// 2. ��ȡ�������ָ���λ��
	uint16_t read_ptr = Read_W5500_SOCK_2Byte(s, Sn_RX_RD);
	uint16_t original_ptr = read_ptr;						/* ����ԭʼָ��λ�� */
	read_ptr &= (S_RX_SIZE -1);								/* ʵ�ʵ������ַ */
	
	// 3. ׼�� spi ��ȡ����
	spi_cs_enable();
	spi_write_short(read_ptr);
	spi_write_byte(VDM | RWB_READ | (s * 0x20 + 0x18));
	
	// 4. ��ȡ����
	uint16_t bytes_read = 0; 								/* �Ѷ�ȡ���ֽ��� */
	uint8_t temp_byte = 0;									/* ��ʱ�洢���ֽ� */
	
	if (read_ptr + rx_size < S_RX_SIZE)		/* 4.1 �����������ȡ */					
	{
		while(bytes_read < rx_size)
		{
			spi_read_byte(&temp_byte);
			dat_ptr[bytes_read++] = temp_byte;
		}
	}
	else	/* 4.2 ����ǿ�Խ�������߽� */
	{
		// �ȶ�ȡ��������ĩβ
        uint16_t first_part_size = S_RX_SIZE - read_ptr;
        while(bytes_read < first_part_size) 
		{
            spi_read_byte(&temp_byte);
            dat_ptr[bytes_read++] = temp_byte;
        }
		
		// Ȼ��ӻ�������ʼλ�ü�����ȡ
        spi_cs_disable();
        spi_cs_enable();
        spi_write_short(0x0000);
        spi_write_byte(VDM|RWB_READ|(s*0x20+0x18));

        while(bytes_read < rx_size) 
		{
            spi_read_byte(&temp_byte);
            dat_ptr[bytes_read++] = temp_byte;
        }
		
	}
	
	// 5. ��� spi ����
	spi_cs_disable();
	
	// 6. ���¶�ָ�벢֪ͨW5500
	uint16_t new_ptr = original_ptr + rx_size;
    Write_W5500_SOCK_2Byte(s, Sn_RX_RD, new_ptr);
    Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);
	
	return rx_size;

}

/*******************************************************************************
* ������  : Write_SOCK_Data_Buffer
* ����    : ������д��W5500�����ݷ��ͻ�����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��,size:��д�����ݵĳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	// 1. ����UDPĿ���ַ�Ͷ˿�
    Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);
    Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256+UDP_DPORT[1]);
    
    // 2. ��ȡ������дָ��λ��
    uint16_t write_ptr = Read_W5500_SOCK_2Byte(s, Sn_TX_WR);
    uint16_t original_ptr = write_ptr;                    // ����ԭʼдָ��
    write_ptr &= (S_TX_SIZE-1);                          // ����ʵ�������ַ
    
    // 3. ׼��SPIд��
    spi_cs_enable();
    spi_write_short(write_ptr);
    spi_write_byte(VDM|RWB_WRITE|(s*0x20+0x10));
    
    // 4. д������
    if(write_ptr + size < S_TX_SIZE)    // 4.1 ���������д��
    {
        for(uint16_t i = 0; i < size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
    }
    else    // 4.2 �����Խ�������߽�
    {
        // ��д�뵽������ĩβ
        uint16_t first_part_size = S_TX_SIZE - write_ptr;
        for(uint16_t i = 0; i < first_part_size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
        
        // �ӻ�������ʼλ�ü���д��
        spi_cs_disable();
        spi_cs_enable();
        spi_write_short(0x0000);
        spi_write_byte(VDM|RWB_WRITE|(s*0x20+0x10));
        
        // д��ʣ������
        for(uint16_t i = first_part_size; i < size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
    }
    
    // 5. ���SPI����
    spi_cs_disable();
    
    // 6. ����дָ�벢֪ͨW5500����
    uint16_t new_ptr = original_ptr + size;
    Write_W5500_SOCK_2Byte(s, Sn_TX_WR, new_ptr);
    Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);
			
}

/*******************************************************************************
* ������  : W5500_Hardware_Reset
* ����    : Ӳ����λW5500
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : W5500�ĸ�λ���ű��ֵ͵�ƽ����500us����,������ΧW5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	spi_cs_disable();
	nrf_gpio_pin_clear(W5500_RST_PIN);//��λ��������
	nrf_delay_ms(50);
	nrf_gpio_pin_set(W5500_RST_PIN);//��λ��������
	nrf_delay_ms(500);
	
	while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//�ȴ���̫���������
}

/*******************************************************************************
* ������  : W5500_Init
* ����    : ��ʼ��W5500�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5500֮ǰ���ȶ�W5500��ʼ��
*******************************************************************************/
void W5500_Init(void)
{

	Write_W5500_1Byte(MR, RST);//�����λW5500,��1��Ч,��λ���Զ���0
	nrf_delay_ms(10);//��ʱ10ms,�Լ�����ú���

	//��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ����� 
	//ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);
			
	//������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
	//��������������������
	Write_W5500_nByte(SUBR,Sub_Mask,4);		
	
	//���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
	//�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	//����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
	Write_W5500_nByte(SHAR,Phy_Addr,6);		

	//���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
	//ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
	Write_W5500_nByte(SIPR,IP_Addr,4);		
	
	//���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�

	/* ���Է���� socket0 ����Ļ����� 16KB���� + 16KB���� */
	Write_W5500_SOCK_1Byte(0, Sn_RXBUF_SIZE, 0x08);	
	Write_W5500_SOCK_1Byte(0, Sn_TXBUF_SIZE, 0x08);
	/* socket ����ж� */
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_RECV);
	Write_W5500_1Byte(IMR, IM_IR7);
	Write_W5500_1Byte(SIMR, S0_IMR); 


	//��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	Write_W5500_2Byte(RTR, 0x07d0);

	//�������Դ���Ϊ10�Σ�Ĭ��Ϊ8�� 
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	Write_W5500_1Byte(RCR,10);

}


/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~7)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	//���÷�Ƭ���ȣ��ο�W5500�����ֲᣬ��ֵ���Բ��޸�	
	Write_W5500_SOCK_2Byte(0, Sn_MSSR, 1460);//����Ƭ�ֽ���=1460(0x5b4)

	//���ö˿�0�Ķ˿ں�
	Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port[0]*256+S0_Port[1]);		
			
}


/*******************************************************************************
* ������  : Socket_UDP
* ����    : ����ָ��Socket(0~7)ΪUDPģʽ
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ���Socket������UDPģʽ,���øó���,��UDPģʽ��,Socketͨ�Ų���Ҫ��������
*			�ó���ֻ����һ�Σ���ʹW5500����ΪUDPģʽ
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//����SocketΪUDPģʽ*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//��Socket*/
	nrf_delay_ms(5);//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//���Socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	else
		return TRUE;

	//���������Socket�Ĵ򿪺�UDPģʽ����,������ģʽ��������Ҫ��Զ��������������
	//��ΪSocket����Ҫ��������,�����ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	//���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶���,�����й�����û�иı�,��ôҲ��������������
}

/*******************************************************************************
* ������  : W5500_Interrupt_Process
* ����    : W5500�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	/* sir��Ӧ��SIR�����ܵ�Socket�жϼĴ�����ÿһλ��Ӧ��һ��socket���ж�״̬*/
	/* ir��Ӧ��Sn_IR���Ǿ���ĳһ��socket���ж����ͼĴ�����ÿһλ��Ӧ��ÿһ���ж�����*/
	/* ע�⣺������ǰ��������Sn_IR��ֻ�ò����ж���Ч��Ҳ����˵�������ε��жϻᱻ��¼ */
	/* ���ǲ���֪ͨINT���Ų����ж� */
	uint8_t sir, ir;
	
	do {
		// ��������pending���ж�
		sir = Read_W5500_1Byte(SIR);
		
		// ��������Socket
		for(int i = 0; i < 8; i++) 
		{
			if(sir & (1 << i)) 
			{
				// ��ȡ�������Socket���ж�
				ir = Read_W5500_SOCK_1Byte(i, Sn_IR);
				Write_W5500_SOCK_1Byte(i, Sn_IR, ir);
				
				// ����Socket0���ض�����
				if(i == 0) 
				{
					if(ir & IR_RECV) 
					{
						S0_Data |= S_RECEIVE;
					}
					if(ir & IR_TIMEOUT) 
					{
						Write_W5500_SOCK_1Byte(0, Sn_CR, CLOSE);
						S0_State = 0;
					}
				}
			}
		}
	} while(Read_W5500_1Byte(SIR) != 0);  // ȷ���Ƿ���δ������ж�
	
}

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
	Gateway_IP[0] = 0;//�������ز���
	Gateway_IP[1] = 0;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 0;

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
		if(Socket_UDP(0)==TRUE)
			S0_State=S_INIT;
		else
			S0_State=0;
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

	if (size < 8)
	{
		return;
	}
	
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
