#include "tcp_connection_raw.h"
#include "lwip/ip.h"
#include "lwip.h"
#include "mqtt_packets.h"
#include "mqtt_cb_info.h"
#include "mqtt_rc.h"

#define TCP_CONNECTION_RAW_PORT 1883

void TCPConnectionRaw_wait_until_mqtt_connected(struct mqtt_cb_info_t* cb_info)
{
	while (!cb_info->mqtt_connected)
		MX_LWIP_Process();
}

void TCPConnectionRaw_wait_for_suback(struct mqtt_cb_info_t* cb_info)
{
	while (!cb_info->last_subscribe_success)
		MX_LWIP_Process();
}

void TCPConnectionRaw_wait_for_puback(struct mqtt_cb_info_t* cb_info)
{
	while (!cb_info->puback_received)
		MX_LWIP_Process();
}

void TCPConnectionRaw_wait_for_pubrec(struct mqtt_cb_info_t* cb_info)
{
	while (!cb_info->pubrec_received)
		MX_LWIP_Process();
}

void TCPConnectionRaw_wait_for_pubcomp(struct mqtt_cb_info_t* cb_info)
{
	while (!cb_info->pubcomp_received)
		MX_LWIP_Process();
}


static err_t tcp_received_cb(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
	struct mqtt_cb_info_t* cb_info = arg;
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
		uint8_t* mqtt_data = (uint8_t*) p->payload;
		enum mqtt_packet_type_t pkt_type = mqtt_data[0] & 0xF0;
		switch (pkt_type)
		{
		case MQTT_CONNACK_PACKET:
		{
			enum mqtt_connection_rc_t conn_rc = mqtt_data[3];
			if (conn_rc == MQTT_CONNECTION_ACCEPTED)
			{
				cb_info->mqtt_connected = true;
			}
			break;
		}
		case MQTT_SUBACK_PACKET:
		{
			enum mqtt_suback_rc_t suback_rc = mqtt_data[4];
			if (suback_rc == SUBACK_MAX_QOS_0)
			{
				cb_info->last_subscribe_success = true;
			}
			break;
		}
		case MQTT_PUBLISH_PACKET:
		{
			uint8_t* topic = mqtt_data + 4;
			uint16_t topic_len = (mqtt_data[2] << 8) | mqtt_data[3];
			uint8_t* data = mqtt_data + 2 + 2 + topic_len;
			uint32_t data_len = p->tot_len - (2 + 2 + topic_len);
			cb_info->msg_received_cb(topic, topic_len, data, data_len);
			break;
		}
		case MQTT_PUBACK_PACKET:
		{
			cb_info->puback_received = true;
			break;
		}
		case MQTT_PUBREC_PACKET:
		{
			cb_info->pubrec_received = true;
			break;
		}
		case MQTT_PUBCOMP_PACKET:
		{
			cb_info->pubcomp_received = true;
			break;
		}

		default:
			break;
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

void TCPConnectionRaw_write_and_output(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* packet, size_t len_packet)
{
	tcp_write(tcp_connection_raw->pcb, (void*) packet, (uint16_t) len_packet, 1);
	tcp_output(tcp_connection_raw->pcb);
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
