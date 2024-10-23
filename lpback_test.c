#include "spi_driver.h"
#include "spi_config.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_gpio.h"
#include <stddef.h>

int loopback_test(void)
{

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    spi_init();

    NRF_LOG_INFO("SPI Loopback Test Start");
    NRF_LOG_FLUSH();

    uint8_t test_data_tx = 0xAB;
    uint8_t test_data_rx = 0x00;

    spi_cs_enable();
    spi_read_write(&test_data_tx, &test_data_rx, 1);
    spi_cs_disable();

    if (test_data_tx == test_data_rx)
    {
        NRF_LOG_INFO("SPI Loopback Test Passed. Sent: 0x%02X, Received: 0x%02X", test_data_tx, test_data_rx);
    }
    else
    {
        NRF_LOG_ERROR("SPI Loopback Test Failed. Sent: 0x%02X, Received: 0x%02X", test_data_tx, test_data_rx);
    }

    NRF_LOG_FLUSH();

    while (1)
    {
		;
    }
}
