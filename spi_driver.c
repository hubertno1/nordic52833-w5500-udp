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


static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  
static volatile bool spi_xfer_done;  


static spi_err_code_t spi_transfer_base(const uint8_t *tx_data, 
                                        uint8_t *rx_data, 
                                        uint16_t length, 
                                        bool need_rx)
{
    SPI_TRANSFER_PREPARE();

    ret_code_t err_code = nrf_drv_spi_transfer(&spi, 
                                                tx_data, 
                                                length,
                                                need_rx ? rx_data : NULL,
                                                need_rx ? length : 0);
    SPI_CHECK_TRANSFER(err_code);

    SPI_WAIT_DONE();
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
    NRF_LOG_INFO("SPI transfer completed.\r\n");
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
    spi_config.frequency = NRF_DRV_SPI_FREQ_1M;                       /* 1M，后面可以测试 8M 的速率是否可行 */

    ret_code_t err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    if (err_code != NRF_SUCCESS)
    {
        return SPI_ERROR_INIT;
    }
    
    nrf_gpio_cfg_output(SPI_SS_PIN);                /* 手动控制片选引脚 */
    spi_cs_disable();                               /* 初始禁用片选 */
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
