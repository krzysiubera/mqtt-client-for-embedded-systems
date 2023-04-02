#include "tcp_connection_raw.h"
#include "lwip/ip.h"
#include "lwip.h"
#include "mqtt_cb_info.h"
#include "mqtt_deserialize.h"

#define TCP_CONNECTION_RAW_PORT 1883

void TCPConnectionRaw_wait_for_condition(bool* condition)
{
	while (!(*condition))
		TCPConnectionRaw_process_lwip_packets();
}

static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	struct mqtt_cb_info_t* cb_info = arg;
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
		uint8_t* mqtt_data = (uint8_t*) p->payload;

		uint32_t bytes_left = deserialize_mqtt_packet(mqtt_data, p->tot_len, cb_info);
		while (bytes_left != 0)
		{
			uint32_t offset = p->tot_len - bytes_left;
			bytes_left = deserialize_mqtt_packet(mqtt_data + offset, bytes_left, cb_info);
		}
	}
	else
	{
		tcp_close(pcb);
	}
	pbuf_free(p);
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

void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw, struct mqtt_cb_info_t* cb_info)
{
	tcp_connection_raw->pcb = tcp_new();
	tcp_connect(tcp_connection_raw->pcb, &tcp_connection_raw->broker_ip_addr, TCP_CONNECTION_RAW_PORT, tcp_connected_cb);
	tcp_arg(tcp_connection_raw->pcb, cb_info);
	tcp_err(tcp_connection_raw->pcb, tcp_error_cb);
	tcp_poll(tcp_connection_raw->pcb, tcp_poll_cb, 4);
	tcp_accept(tcp_connection_raw->pcb, tcp_connected_cb);
}

void TCPConnectionRaw_write(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* packet, size_t len_packet)
{
	tcp_write(tcp_connection_raw->pcb, (void*) packet, (uint16_t) len_packet, 1);
}

void TCPConnectionRaw_output(struct tcp_connection_raw_t* tcp_connection_raw)
{
	tcp_output(tcp_connection_raw->pcb);
}

void TCPConnectionRaw_close(struct tcp_connection_raw_t* tcp_connection_raw)
{
	tcp_close(tcp_connection_raw->pcb);
}

void TCPConnectionRaw_process_lwip_packets()
{
	MX_LWIP_Process();
}
