#include <stddef.h>
#include "nrf_gpio.h"
#include "test_spi_speed.h"
#include "spi_driver.h"
#include "spi_config.h"


#define TEST_PIN  NRF_GPIO_PIN_MAP(0,13)            /* TEST PIN 用来测试一次完整的spi传输字节需要多少时间 */


void test_spi_speed(void)
{  
    nrf_gpio_cfg_output(TEST_PIN);
    spi_init();

    while(1)
    {
        nrf_gpio_pin_set(TEST_PIN);

        spi_cs_enable();
        spi_write_byte(0x55);
        spi_cs_disable();

        nrf_gpio_pin_clear(TEST_PIN);	
    }
	
}
