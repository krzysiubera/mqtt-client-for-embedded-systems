#include "tcp_connection_raw.h"
#include "lwip/ip.h"

#define TCP_CONNECTION_RAW_PORT 1883


static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
	}
	else
	{
		tcp_close(pcb);
	}
	return ERR_OK;
}

static err_t tcp_connected_cb(void* arg, struct tcp_pcb* npcb, err_t err)
{
	tcp_recv(npcb, tcp_received_cb);
	return ERR_OK;
}

static void tcp_error_cb(void* arg, err_t err)
{

}

static err_t tcp_poll_cb(void* arg, struct tcp_pcb* tpcb)
{
	return ERR_OK;
}


void TCPConnectionRaw_init(struct tcp_connection_raw_t* tcp_connection_raw)
{
	IP4_ADDR(&tcp_connection_raw->broker_ip_addr, 192, 168, 1, 2);
}

void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw)
{
	tcp_connection_raw->pcb = tcp_new();
	tcp_connect(tcp_connection_raw->pcb, &tcp_connection_raw->broker_ip_addr, TCP_CONNECTION_RAW_PORT, tcp_connected_cb);
	tcp_arg(tcp_connection_raw->pcb, NULL);
	tcp_err(tcp_connection_raw->pcb, tcp_error_cb);
	tcp_poll(tcp_connection_raw->pcb, tcp_poll_cb, 4);
	tcp_accept(tcp_connection_raw->pcb, tcp_connected_cb);
}

void TCPConnectionRaw_write(struct tcp_connection_raw_t* tcp_connection_raw, char* packet)
{
	tcp_write(tcp_connection_raw->pcb, (void*) packet, sizeof(packet), 1);
	tcp_output(tcp_connection_raw->pcb);
}
