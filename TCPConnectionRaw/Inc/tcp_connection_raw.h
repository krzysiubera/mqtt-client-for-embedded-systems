#ifndef TCP_CONNECTION_RAW_H
#define TCP_CONNECTION_RAW_H

#include "lwip/tcp.h"

struct tcp_connection_raw_t
{
	struct tcp_pcb* pcb;
	ip_addr_t broker_ip_addr;
};

void TCPConnectionRaw_init(struct tcp_connection_raw_t* tcp_connection_raw);
void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw);
void TCPConnectionRaw_write(struct tcp_connection_raw_t* tcp_connection_raw, char* packet);


#endif
