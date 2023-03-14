#include "tcp_connection_raw.h"
#include "lwip/ip.h"
#include "mqtt_client.h"

#define TCP_CONNECTION_RAW_PORT 1883


static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	struct tcp_connection_raw_t* tcp_connection_raw = arg;
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
		uint8_t* mqtt_data = (uint8_t*) p->payload;
		enum mqtt_packet_type_t pkt_type = mqtt_data[0] & 0xF0;
		switch (pkt_type)
		{
		case MQTT_CONNACK_PACKET:
			if (mqtt_data[3] == MQTT_CONNECTION_ACCEPTED)
			{
				tcp_connection_raw->mqtt_connected = true;
			}
			break;
		default:
			break;
		}

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
	tcp_connection_raw->mqtt_connected = false;
}

void TCPConnectionRaw_connect(struct tcp_connection_raw_t* tcp_connection_raw)
{
	tcp_connection_raw->pcb = tcp_new();
	tcp_connect(tcp_connection_raw->pcb, &tcp_connection_raw->broker_ip_addr, TCP_CONNECTION_RAW_PORT, tcp_connected_cb);
	tcp_arg(tcp_connection_raw->pcb, tcp_connection_raw);
	tcp_err(tcp_connection_raw->pcb, tcp_error_cb);
	tcp_poll(tcp_connection_raw->pcb, tcp_poll_cb, 4);
	tcp_accept(tcp_connection_raw->pcb, tcp_connected_cb);
}

void TCPConnectionRaw_write(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* packet, size_t len_packet)
{
	tcp_write(tcp_connection_raw->pcb, (void*) packet, (uint16_t) len_packet, 1);
	tcp_output(tcp_connection_raw->pcb);
}
