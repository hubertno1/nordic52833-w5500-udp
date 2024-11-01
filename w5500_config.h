#ifndef __W5500_CONFIG_H_
#define __W5500_CONFIG_H_

#include <stdint.h>
#include "w5500.h"

typedef struct {
    uint8_t gateway_ip[4];      // Gateway IP
    uint8_t subnet_mask[4];     // Subnet mask
    uint8_t mac_addr[6];        // MAC address
    uint8_t local_ip[4];        // Local IP
} w5500_network_t;

typedef enum {
    SOCKET_MODE_CLOSE = 0,      // Socket closed
    SOCKET_MODE_TCP_SERVER,     // TCP server mode
    SOCKET_MODE_TCP_CLIENT,     // TCP client mode
    SOCKET_MODE_UDP,            // UDP mode
} socket_mode_t;

typedef enum {
    SOCKSTAT_CLOSED = 0,        // Socket closed
    SOCKSTAT_INIT,              // Socket initialized
    SOCKSTAT_LISTEN,            // Socket listening
    SOCKSTAT_ESTABLISHED,       // Socket established
    SOCKSTAT_UDP,               // Socket in UDP mode
} socket_state_t;

typedef enum {
    SOCKDATA_NONE = 0,          // No data
    SOCKDATA_RECEIVED,          // Data received
    SOCKDATA_SENT,              // Data sent
} socket_data_status_t;

typedef struct {
    uint8_t id;                 // Socket ID (0~7)
    socket_mode_t mode;         // Working mode
    socket_state_t state;       // Socket state
    socket_data_status_t status;  // Data status
    uint16_t local_port;        // Local port
    uint8_t remote_ip[4];       // Remote IP (for UDP)
    uint16_t remote_port;       // Remote port (for UDP)
    uint8_t *p_rxbuf;           // Pointer to receive buffer
    uint8_t *p_txbuf;           // Pointer to transmit buffer
    uint16_t rxbuf_size;        // Receive buffer size
    uint16_t txbuf_size;        // Transmit buffer size
} w5500_socket_t;

typedef void (*w5500_reset_func_t)(void);
typedef void (*w5500_init_func_t)(void);
typedef void (*w5500_set_network_func_t)(void);
typedef void (*w5500_socket_open_func_t)(uint8_t);
typedef void (*w5500_socket_close_func_t)(uint8_t);

typedef struct {
    w5500_network_t network;                    // Network parameters
    w5500_socket_t  sockets[8];                 // Socket array
    w5500_reset_func_t reset;                   // Hardware reset function
    w5500_init_func_t init;                     // Initialization function
    w5500_set_network_func_t set_network;       // Set network parameters
    w5500_socket_open_func_t socket_open;       // Open socket
    w5500_socket_close_func_t socket_close;     // Close socket
    // To be supplemented, other operation functions
} w5500_dev_t;

#endif
