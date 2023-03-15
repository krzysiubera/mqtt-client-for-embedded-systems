#ifndef TCP_CONNECTION_RAW_H
#define TCP_CONNECTION_RAW_H

#include "lwip/tcp.h"
#include <stdbool.h>

struct tcp_connection_raw_t
{
	struct tcp_pcb* pcb;
	ip_addr_t broker_ip_addr;
	bool mqtt_connected;
};

void TCPConnectionRaw_init(struct tcp_connection_raw_t* tcp_connection_raw);
void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw);
void TCPConnectionRaw_write(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* packet, size_t len_packet);
void TCPConnectionRaw_wait_until_mqtt_connected(struct tcp_connection_raw_t* tcp_connection_raw);


#endif
