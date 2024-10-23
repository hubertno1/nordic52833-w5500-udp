#ifndef __W5500_CONFIG_H_
#define __W5500_CONFIG_H_

#include <stdint.h>
#include "w5500.h"

typedef struct {
    uint8_t gateway_ip[4];      // ����ip
    uint8_t subnet_mask[4];     // ��������
    uint8_t mac_addr[6];        // MAC addr
    uint8_t local_ip[4];        // ���� ip
} w5500_network_config_t;

typedef struct {
    uint8_t port[2];        // �˿ں�
    uint8_t mode;           // ����ģʽ
    uint8_t state;          // socket״̬
} w5500_socket_config_t;

typedef struct {
    w5500_network_config_t network;
    w5500_socket_config_t  sockets[8];
    void (*reset) (void);
    void (*init) (void);
} w5500_dev_t;



#endif
