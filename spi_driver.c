#include "spi_driver.h"
#include "spi_config.h"
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "nrf_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sdk_config.h"
#include <stddef.h>
#include <nrf_delay.h>
#include <nrfx_ppi.h>
#include <nrf_gpio.h>

// 定义时间引脚 (TIME_PIN) 为输出引脚
#define TIME_PIN  NRF_GPIO_PIN_MAP(0,17)  // 假设在P0.17上

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  
static volatile bool spi_xfer_done;  
// static uint32_t ppi_channel;

static spi_err_code_t spi_transfer_base(const uint8_t *tx_data, 
                                        uint8_t *rx_data, 
                                        uint16_t length, 
                                        bool need_rx)
{
    
    //nrf_gpio_pin_clear(TIME_PIN);
    //nrf_delay_ms(1);
    SPI_TRANSFER_PREPARE();
    //nrf_delay_ms(1);  
    //nrf_gpio_pin_set(TIME_PIN);

		
    ret_code_t err_code = nrf_drv_spi_transfer(&spi, 
                                                tx_data, 
                                                length,
                                                need_rx ? rx_data : NULL,
                                                need_rx ? length : 0);
    SPI_CHECK_TRANSFER(err_code);
    nrf_gpio_pin_clear(TIME_PIN);
    SPI_WAIT_DONE();
    // for (int i = 0; i < 10; i++)
    // {
    //     __NOP();
    // }
	nrf_gpio_pin_set(TIME_PIN);
    return SPI_SUCCESS;
}

spi_err_code_t spi_cs_enable(void)
{
    nrf_gpio_pin_clear(SPI_SS_PIN);
    return SPI_SUCCESS;
}

spi_err_code_t spi_cs_disable(void)
{
    nrf_gpio_pin_set(SPI_SS_PIN);
    return SPI_SUCCESS;
}

static void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
    spi_xfer_done = true;
}

/**
 * @brief 配置和初始化 spi
 * 
 * @return spi_err_code_t 
 */
spi_err_code_t spi_init(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;                   /* 禁用 spi 驱动库自动控制片选，手动控制片选信号，为了更好控制 W5500 的时序 */
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    spi_config.mode = NRF_DRV_SPI_MODE_0;                             /* 配置 spi 模式为 0 */
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;                       /* 1M，后面可以测试 8M 的速率是否可行 */

    ret_code_t err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    if (err_code != NRF_SUCCESS)
    {
        return SPI_ERROR_INIT;
    }
    
    nrf_gpio_cfg_output(SPI_SS_PIN);                /* 手动控制片选引脚 */
    spi_cs_disable();                               /* 初始禁用片选 */
    
    //nrfx_ppi_channel_alloc(&ppi_channel);           /* 分配一个 PPI 通道 */
    
        // 获取任务地址
    //uint32_t spi_start_task_addr = nrfx_ppi_task_addr_get(NRF_DRV_SPI_EVENT_START);
    // 获取事件地址
    //uint32_t spi_done_event_addr = nrfx_ppi_event_addr_get(NRF_DRV_SPI_EVENT_DONE);

    // 将 SPI 任务和事件连接：SPI 事件完成后自动触发下一个 SPI 任务
    //nrfx_ppi_channel_assign(ppi_channel,
                            //spi_start_task_addr,   // 获取 SPI 任务的地址
                            //spi_done_event_addr);  // 获取 SPI 事件的地址
    
    return SPI_SUCCESS;
}

spi_err_code_t spi_write_byte(uint8_t byte)
{
    return spi_transfer_base(&byte, NULL, 1, false);
}

spi_err_code_t spi_write_short(uint16_t data)
{
    /* big endian */
    uint8_t tx_data[2] = {
        (data >> 8) & 0xFF,    // high byte
        data & 0xFF            // low 
    };
    return spi_transfer_base(tx_data, NULL, 2, false);
}

spi_err_code_t spi_write_bytes(const uint8_t *data, uint16_t length)
{
    SPI_CHECK_NULL(data);
    return spi_transfer_base(data, NULL, length, false);
}

spi_err_code_t spi_read_byte(uint8_t *rx_data)
{
    SPI_CHECK_NULL(rx_data);
    uint8_t tx_dummy = 0xFF;
    return spi_transfer_base(&tx_dummy, rx_data, 1, true);
}

spi_err_code_t spi_read_short(uint16_t *rx_data)
{
    SPI_CHECK_NULL(rx_data);
    uint8_t rx_bytes[2];
    uint8_t tx_dummy[2] = {0xFF, 0xFF};

    spi_err_code_t err = spi_transfer_base(tx_dummy, rx_bytes, 2, true);
    if (err == SPI_SUCCESS)
    {
        *rx_data = (rx_bytes[0] << 8) | rx_bytes[1];
    }
    return err;
}

spi_err_code_t spi_read_bytes(uint8_t *data, uint16_t length)
{
    SPI_CHECK_NULL(data);

    uint8_t *tx_dummy = malloc(length);
    if (!tx_dummy) 
    {
        return SPI_ERROR_MEMORY;
    }
    memset(tx_dummy, 0xFF, length);

    spi_err_code_t ret = spi_transfer_base(tx_dummy, data, length, true);
    free(tx_dummy);
    return ret;
}

spi_err_code_t spi_read_write(uint8_t *tx_data, uint8_t *rx_data, uint16_t length)
{
    SPI_CHECK_NULL(tx_data);
    SPI_CHECK_NULL(rx_data);

    return spi_transfer_base(tx_data, rx_data, (uint16_t)length, true);
}
