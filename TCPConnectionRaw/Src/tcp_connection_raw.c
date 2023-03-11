#include "tcp_connection_raw.h"
#include "lwip/ip.h"

#define TCP_CONNECTION_RAW_PORT 1883


static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	return ERR_OK;
}

static err_t tcp_connected_cb(void* arg, struct tcp_pcb* npcb, err_t err)
{
	tcp_recv(npcb, tcp_received_cb);
	return ERR_OK;
}


void TCPConnectionRaw_init(struct tcp_connection_raw_t* tcp_connection_raw)
{
	IP4_ADDR(&(tcp_connection_raw->broker_ip_addr), 192, 168, 1, 2);
}

void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw)
{
	tcp_connection_raw->pcb = tcp_new();
	tcp_connect(tcp_connection_raw->pcb,
				&tcp_connection_raw->broker_ip_addr,
				TCP_CONNECTION_RAW_PORT,
				tcp_connected_cb);
}
