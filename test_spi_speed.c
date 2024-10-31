#include "test_spi_speed.h"
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
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "test_spi_speed.h"

#define TEST_PIN  NRF_GPIO_PIN_MAP(0,13)

void test_spi_speed(void)
{  
    nrf_gpio_cfg_output(TEST_PIN);
    spi_init();
    while(1)
    {
    // µ¥×Ö½Ú´«Êä²âÊÔ
    nrf_gpio_pin_set(TEST_PIN);
    spi_cs_enable();
    spi_write_byte(0x55);
    spi_cs_disable();
    nrf_gpio_pin_clear(TEST_PIN);
		
	//nrf_delay_ms(100);
    }
	
}

