#ifndef __SPI_DRIVER_H
#define __SPI_DRIVER_H

#include <stdint.h>
#include <stddef.h>


typedef enum {
    SPI_SUCCESS         = 0x00,
    SPI_ERROR_INIT      = 0x01,
    SPI_ERROR_NULL      = 0x02,
    SPI_ERROR_MEMORY    = 0x03,
    SPI_ERROR_TRANSFER  = 0x04,
} spi_err_code_t;


#define SPI_WAIT_DONE()                 \
    do {                                \
        while (!spi_xfer_done)          \
        {                               \
            ;                           \
        }                               \
    } while(0)                          

#define SPI_CHECK_TRANSFER(err_code)   \
    do {                               \
        if (err_code != NRF_SUCCESS) { \
            return SPI_ERROR_TRANSFER; \
        }                              \
    } while(0)

#define SPI_CHECK_NULL(ptr)           \
    do {                              \
        if (!(ptr)) {                 \
            return SPI_ERROR_NULL;    \
        }                             \
    } while(0)

#define SPI_TRANSFER_PREPARE()        \
    do {                              \
        spi_xfer_done = false;        \
    } while(0)


spi_err_code_t spi_init(void);
spi_err_code_t spi_cs_enable(void);
spi_err_code_t spi_cs_disable(void);

spi_err_code_t spi_write_byte(uint8_t byte);
spi_err_code_t spi_write_short(uint16_t data);
spi_err_code_t spi_write_bytes(const uint8_t *data, uint16_t length);

spi_err_code_t spi_read_byte(uint8_t *rx_data);
spi_err_code_t spi_read_bytes(uint8_t *data, uint16_t length);

spi_err_code_t spi_read_write(uint8_t *tx_data, uint8_t *rx_data, uint16_t length);

#endif /* __SPI_DRIVER_H */
