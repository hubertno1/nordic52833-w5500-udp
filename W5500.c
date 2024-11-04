/**********************************************************************************
 * 文件名  ：W5500.c
 * 描述    ：W5500 驱动函数库         
 * 库版本  ：ST_v3.5

 * 淘宝    ：http://yixindianzikeji.taobao.com/
**********************************************************************************/

#include "W5500.h"	
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "spi_driver.h"
#include "string.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


/***************----- 网络参数变量定义 -----***************/
unsigned char Gateway_IP[4];//网关IP地址 
unsigned char Sub_Mask[4];	//子网掩码 
unsigned char Phy_Addr[6];	//物理地址(MAC) 
unsigned char IP_Addr[4];	//本机IP地址 

unsigned char S0_Port[2];	//端口0的端口号(5000) 

unsigned char UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
unsigned char UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
unsigned char S0_Mode =3;	//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER	0x00	//TCP服务器模式
#define TCP_CLIENT	0x01	//TCP客户端模式 
#define UDP_MODE	0x02	//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
unsigned char S0_State =0;	//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据) 
#define S_INIT		0x01	//端口完成初始化 
#define S_CONN		0x02	//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
unsigned char S0_Data;		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE	 0x01	//端口接收到一个数据包 
#define S_TRANSMITOK 0x02	//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
unsigned char Rx_Buffer[4096];	//端口接收数据缓冲区 
unsigned char Tx_Buffer[4096];	//端口发送数据缓冲区 

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 通过SPI1向指定地址寄存器写1个字节数据
* 输入    : reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
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
* 函数名  : Write_W5500_2Byte
* 描述    : 通过SPI1向指定地址寄存器写2个字节数据
* 输入    : reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	spi_cs_enable();  //置W5500的SCS为低电平
		
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM2|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,2个字节数据长度,写数据,选择通用寄存器
	spi_write_short(dat);//写16位数据

	spi_cs_disable(); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	spi_cs_enable();  //置W5500的SCS为低电平	
		
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,N个字节数据长度,写数据,选择通用寄存器

	for(i=0;i<size;i++)//循环将缓冲区的size个字节数据写入W5500
	{
		spi_write_byte(*dat_ptr++);//写一个字节数据
	}

	spi_cs_disable(); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	spi_cs_enable();  //置W5500的SCS为低电平	
		
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM1|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,写数据,选择端口s的寄存器
	spi_write_byte(dat);//写1个字节数据

	spi_cs_disable(); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	spi_cs_enable();  //置W5500的SCS为低电平
			
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM2|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,写数据,选择端口s的寄存器
	spi_write_short(dat);//写16位数据

	spi_cs_disable(); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	spi_cs_enable();  //置W5500的SCS为低电平
			
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM4|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,4个字节数据长度,写数据,选择端口s的寄存器

	spi_write_byte(*dat_ptr++);//写第1个字节数据
	spi_write_byte(*dat_ptr++);//写第2个字节数据
	spi_write_byte(*dat_ptr++);//写第3个字节数据
	spi_write_byte(*dat_ptr++);//写第4个字节数据

	spi_cs_disable(); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读W5500指定地址寄存器的1个字节数据
* 输入    : reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;

	spi_cs_enable();  //置W5500的SCS为低电平
			
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM1|RWB_READ|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,读数据,选择通用寄存器

	spi_read_byte(&i);

	spi_cs_disable();//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读W5500指定端口寄存器的1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	spi_cs_enable();  //置W5500的SCS为低电平
			
	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM1|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,读数据,选择端口s的寄存器

	spi_read_byte(&i);

	spi_cs_disable();//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的2个字节数据(16位)
* 说明    : 无
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	uint16_t value;

	spi_cs_enable();  //置W5500的SCS为低电平

	spi_write_short(reg);//通过SPI1写16位寄存器地址
	spi_write_byte(FDM2|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,读数据,选择端口s的寄存器
	spi_read_short(&value);
	
	spi_cs_disable();//置W5500的SCS为高电平

	return value;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : 从W5500接收数据缓冲区中读取数据
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针
* 输出    : 无
* 返回值  : 读取到的数据长度,rx_size个字节
* 说明    : 无
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	// 1. 检查并获取可读数据大小
	uint16_t rx_size = Read_W5500_SOCK_2Byte(s, Sn_RX_RSR);
	if (rx_size == 0)
	{
		return 0;
	}
	rx_size = (rx_size > 1460) ? 1460 : rx_size;			/* 限制单次读取大小 （MTU限制）*/
	
	// 2. 获取并计算读指针的位置
	uint16_t read_ptr = Read_W5500_SOCK_2Byte(s, Sn_RX_RD);
	uint16_t original_ptr = read_ptr;						/* 保留原始指针位置 */
	read_ptr &= (S_RX_SIZE -1);								/* 实际的物理地址 */
	
	// 3. 准备 spi 读取数据
	spi_cs_enable();
	spi_write_short(read_ptr);
	spi_write_byte(VDM | RWB_READ | (s * 0x20 + 0x18));
	
	// 4. 读取数据
	uint16_t bytes_read = 0; 								/* 已读取的字节数 */
	uint8_t temp_byte = 0;									/* 临时存储的字节 */
	
	if (read_ptr + rx_size < S_RX_SIZE)		/* 4.1 如果是连续读取 */					
	{
		while(bytes_read < rx_size)
		{
			spi_read_byte(&temp_byte);
			dat_ptr[bytes_read++] = temp_byte;
		}
	}
	else	/* 4.2 如果是跨越缓冲区边界 */
	{
		// 先读取到缓冲区末尾
        uint16_t first_part_size = S_RX_SIZE - read_ptr;
        while(bytes_read < first_part_size) 
		{
            spi_read_byte(&temp_byte);
            dat_ptr[bytes_read++] = temp_byte;
        }
		
		// 然后从缓冲区起始位置继续读取
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
	
	// 5. 完成 spi 操作
	spi_cs_disable();
	
	// 6. 更新读指针并通知W5500
	uint16_t new_ptr = original_ptr + rx_size;
    Write_W5500_SOCK_2Byte(s, Sn_RX_RD, new_ptr);
    Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);
	
	return rx_size;

}

/*******************************************************************************
* 函数名  : Write_SOCK_Data_Buffer
* 描述    : 将数据写入W5500的数据发送缓冲区
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针,size:待写入数据的长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	// 1. 设置UDP目标地址和端口
    Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);
    Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256+UDP_DPORT[1]);
    
    // 2. 获取并计算写指针位置
    uint16_t write_ptr = Read_W5500_SOCK_2Byte(s, Sn_TX_WR);
    uint16_t original_ptr = write_ptr;                    // 保存原始写指针
    write_ptr &= (S_TX_SIZE-1);                          // 计算实际物理地址
    
    // 3. 准备SPI写入
    spi_cs_enable();
    spi_write_short(write_ptr);
    spi_write_byte(VDM|RWB_WRITE|(s*0x20+0x10));
    
    // 4. 写入数据
    if(write_ptr + size < S_TX_SIZE)    // 4.1 如果是连续写入
    {
        for(uint16_t i = 0; i < size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
    }
    else    // 4.2 如果跨越缓冲区边界
    {
        // 先写入到缓冲区末尾
        uint16_t first_part_size = S_TX_SIZE - write_ptr;
        for(uint16_t i = 0; i < first_part_size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
        
        // 从缓冲区起始位置继续写入
        spi_cs_disable();
        spi_cs_enable();
        spi_write_short(0x0000);
        spi_write_byte(VDM|RWB_WRITE|(s*0x20+0x10));
        
        // 写入剩余数据
        for(uint16_t i = first_part_size; i < size; i++)
        {
            spi_write_byte(*dat_ptr++);
        }
    }
    
    // 5. 完成SPI操作
    spi_cs_disable();
    
    // 6. 更新写指针并通知W5500发送
    uint16_t new_ptr = original_ptr + size;
    Write_W5500_SOCK_2Byte(s, Sn_TX_WR, new_ptr);
    Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);
			
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	spi_cs_disable();
	nrf_gpio_pin_clear(W5500_RST_PIN);//复位引脚拉低
	nrf_delay_ms(50);
	nrf_gpio_pin_set(W5500_RST_PIN);//复位引脚拉高
	nrf_delay_ms(500);
	
	while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//等待以太网连接完成
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void W5500_Init(void)
{

	Write_W5500_1Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
	nrf_delay_ms(10);//延时10ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义 
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);
			
	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	Write_W5500_nByte(SUBR,Sub_Mask,4);		
	
	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	Write_W5500_nByte(SHAR,Phy_Addr,6);		

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	Write_W5500_nByte(SIPR,IP_Addr,4);		
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册

	/* 尝试分配给 socket0 更多的缓冲区 16KB接收 + 16KB发送 */
	Write_W5500_SOCK_1Byte(0, Sn_RXBUF_SIZE, 0x08);	
	Write_W5500_SOCK_1Byte(0, Sn_TXBUF_SIZE, 0x08);
	/* socket 相关中断 */
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_RECV);
	Write_W5500_1Byte(IMR, IM_IR7);
	Write_W5500_1Byte(SIMR, S0_IMR); 


	//设置重试时间，默认为2000(200ms) 
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	Write_W5500_2Byte(RTR, 0x07d0);

	//设置重试次数为10次，默认为8次 
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	Write_W5500_1Byte(RCR,10);

}


/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~7)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	//设置分片长度，参考W5500数据手册，该值可以不修改	
	Write_W5500_SOCK_2Byte(0, Sn_MSSR, 1460);//最大分片字节数=1460(0x5b4)

	//设置端口0的端口号
	Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port[0]*256+S0_Port[1]);		
			
}


/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定Socket(0~7)为UDP模式
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需要建立连接
*			该程序只调用一次，就使W5500设置为UDP模式
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//设置Socket为UDP模式*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//打开Socket*/
	nrf_delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//如果Socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	else
		return TRUE;

	//至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
	//因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	//如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}

/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	/* sir对应着SIR，是总的Socket中断寄存器，每一位对应着一个socket的中断状态*/
	/* ir对应着Sn_IR，是具体某一个socket的中断类型寄存器，每一位对应着每一种中断类型*/
	/* 注意：我们在前面配置了Sn_IR，只让部分中断生效；也就是说，被屏蔽的中断会被记录 */
	/* 但是不会通知INT引脚产生中断 */
	uint8_t sir, ir;
	
	do {
		// 处理所有pending的中断
		sir = Read_W5500_1Byte(SIR);
		
		// 遍历所有Socket
		for(int i = 0; i < 8; i++) 
		{
			if(sir & (1 << i)) 
			{
				// 读取和清除该Socket的中断
				ir = Read_W5500_SOCK_1Byte(i, Sn_IR);
				Write_W5500_SOCK_1Byte(i, Sn_IR, ir);
				
				// 处理Socket0的特定功能
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
	} while(Read_W5500_1Byte(SIR) != 0);  // 确认是否还有未处理的中断
	
}

/*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : W5500初始货配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 初始化W5500寄存器函数,检查网关服务器,指定Socket(0~7)初始化,初始化端口0
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();		//初始化W5500寄存器函数
	Socket_Init(0);		//指定Socket(0~7)初始化,初始化端口0
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 0;//加载网关参数
	Gateway_IP[1] = 0;
	Gateway_IP[2] = 0;
	Gateway_IP[3] = 0;

	Sub_Mask[0]=255;//加载子网掩码
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//加载物理地址
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=192;//加载本机IP地址
	IP_Addr[1]=168;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	Phy_Addr[4]=IP_Addr[2];
	Phy_Addr[5]=IP_Addr[3];
	
	S0_Port[0] = 0x13;//加载端口0的端口号5000 
	S0_Port[1] = 0x88;

}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//端口0初始化配置
	{
		if(Socket_UDP(0)==TRUE)
			S0_State=S_INIT;
		else
			S0_State=0;
	}
}
/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	// 从socket接收缓冲区读取数据
	size=Read_SOCK_Data_Buffer(s, Rx_Buffer);

	if (size < 8)
	{
		return;
	}
	
	// 提取目的IP地址和端口号
	UDP_DIPR[0] = Rx_Buffer[0];
	UDP_DIPR[1] = Rx_Buffer[1];
	UDP_DIPR[2] = Rx_Buffer[2];
	UDP_DIPR[3] = Rx_Buffer[3];

	UDP_DPORT[0] = Rx_Buffer[4];
	UDP_DPORT[1] = Rx_Buffer[5];
	

	// 将接收到的数据复制到发送缓冲区
	memcpy(Tx_Buffer, Rx_Buffer+8, size-8);

	// 将处理后的数据写入W5500的socket发送缓冲区			
	Write_SOCK_Data_Buffer(s, Tx_Buffer, size-8);

}
