#include "tcp_connection_raw.h"
#include "lwip/ip.h"
#include "lwip.h"
#include "mqtt_receive.h"

#define TCP_CONNECTION_RAW_PORT 1883

static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	struct mqtt_client_t* mqtt_client = arg;
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
		uint8_t* payload = (uint8_t*) p->payload;

		int32_t bytes_left = get_mqtt_packet(payload, p->tot_len, mqtt_client);
		if (bytes_left == MQTT_INVALID_MSG_LEN)
		{
			tcp_close(mqtt_client->pcb);
			mqtt_client->mqtt_connected = false;
		}

		while (bytes_left != 0)
		{
			uint32_t offset = p->tot_len - (uint32_t) bytes_left;
			bytes_left = get_mqtt_packet(payload + offset, (uint32_t) bytes_left, mqtt_client);
			if (bytes_left == MQTT_INVALID_MSG_LEN)
			{
				tcp_close(mqtt_client->pcb);
				mqtt_client->mqtt_connected = false;
			}
		}
	}
	else
	{
		tcp_close(pcb);
		mqtt_client->mqtt_connected = false;
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
	TCPHandler_process_lwip_packets();
	return ERR_OK;
}

void TCPHandler_process_lwip_packets()
{
	MX_LWIP_Process();
}

void TCPHandler_set_ip_address(ip_addr_t* broker_ip_addr)
{
	IP4_ADDR(broker_ip_addr, 192, 168, 1, 2);
}

struct tcp_pcb* TCPHandler_get_pcb()
{
	return tcp_new();
}

enum mqtt_client_err_t TCPHandler_connect(struct mqtt_client_t* mqtt_client)
{
	err_t err = tcp_connect(mqtt_client->pcb, &mqtt_client->broker_ip_addr, TCP_CONNECTION_RAW_PORT, tcp_connected_cb);
	if (err != ERR_OK)
		return MQTT_TCP_CONNECT_FAILURE;

	tcp_arg(mqtt_client->pcb, mqtt_client);
	tcp_err(mqtt_client->pcb, tcp_error_cb);
	tcp_poll(mqtt_client->pcb, tcp_poll_cb, 1);
	tcp_accept(mqtt_client->pcb, tcp_connected_cb);
	return MQTT_SUCCESS;
}

void TCPHandler_write(struct tcp_pcb* pcb, uint8_t* packet, uint16_t len_packet)
{
	tcp_write(pcb, (void*) packet, len_packet, 1);
}

void TCPHandler_output(struct tcp_pcb* pcb)
{
	tcp_output(pcb);
}

void TCPHandler_close(struct tcp_pcb* pcb)
{
	tcp_close(pcb);
}
