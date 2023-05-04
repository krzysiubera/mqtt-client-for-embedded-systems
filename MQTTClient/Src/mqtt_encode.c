#include "mqtt_encode.h"
#include <stdint.h>
#include <string.h>
#include "mqtt_packets.h"
#include "mqtt_send.h"

#define CLEAN_SESSION 1
static char* protocol_name = "MQTT";
static uint8_t protocol_version = 0x04;

static uint32_t get_connect_packet_len(struct mqtt_client_connect_opts_t* conn_opts)
{
	uint32_t len = 10 + 2 + strlen(conn_opts->client_id);
	if (conn_opts->will_topic)
		len += (2 + strlen(conn_opts->will_topic));
	if (conn_opts->will_msg)
		len += (2 + strlen(conn_opts->will_msg));
	if (conn_opts->username)
		len += (2 + strlen(conn_opts->username));
	if (conn_opts->password)
		len += (2 + strlen(conn_opts->password));
	return len;
}

static uint8_t get_connect_flags(struct mqtt_client_connect_opts_t* conn_opts)
{
	return ((conn_opts->username ? 1 : 0) << 7) |
			((conn_opts->password ? 1 : 0) << 6) |
			(conn_opts->will_retain << 5) |
            (conn_opts->will_qos << 4) |
			((conn_opts->will_msg ? 1 : 0) << 2) |
			(CLEAN_SESSION << 1);
}

static uint32_t get_publish_packet_len(char* topic, char* msg, uint8_t qos)
{
	uint32_t remaining_len = 2 + strlen(topic) + strlen(msg);
	if (qos != 0)
		remaining_len += 2;
	return remaining_len;
}

static uint32_t get_subscribe_packet_len(char* topic)
{
	return 2 + 2 + strlen(topic) + 1;
}

static uint16_t get_packet_id(uint16_t* last_packet_id)
{
	if ((*last_packet_id) == 65535)
	{
		(*last_packet_id) = 0;
	}
	(*last_packet_id)++;
	return (*last_packet_id);
}

void encode_mqtt_connect_msg(struct tcp_pcb* pcb, struct mqtt_client_connect_opts_t* conn_opts)
{
	uint8_t ctrl_field = (uint8_t) MQTT_CONNECT_PACKET;
	uint32_t remaining_len = get_connect_packet_len(conn_opts);
	uint8_t connect_flags = get_connect_flags(conn_opts);

	send_fixed_header(pcb, ctrl_field, remaining_len);
	send_utf8_encoded_str(pcb, (uint8_t*) protocol_name, strlen(protocol_name));
	send_u8(pcb, &protocol_version);
	send_u8(pcb, &connect_flags);

	uint16_t keepalive_seconds = conn_opts->keepalive_ms / 1000;
	send_u16(pcb, &keepalive_seconds);

	send_utf8_encoded_str(pcb, (uint8_t*) conn_opts->client_id, strlen(conn_opts->client_id));
	if (conn_opts->will_topic)
		send_utf8_encoded_str(pcb, (uint8_t*) conn_opts->will_topic, strlen(conn_opts->will_topic));
	if (conn_opts->will_msg)
		send_utf8_encoded_str(pcb, (uint8_t*) conn_opts->will_msg, strlen(conn_opts->will_msg));
	if (conn_opts->username)
		send_utf8_encoded_str(pcb, (uint8_t*) conn_opts->username, strlen(conn_opts->username));
	if (conn_opts->password)
		send_utf8_encoded_str(pcb, (uint8_t*) conn_opts->password, strlen(conn_opts->password));
}

void encode_mqtt_publish_msg(struct tcp_pcb* pcb, char* topic, char* msg, uint8_t qos, bool retain, uint16_t* last_packet_id)
{
	uint32_t remaining_len = get_publish_packet_len(topic, msg, qos);
	uint8_t ctrl_field = (MQTT_PUBLISH_PACKET | (0 << 3) | (qos << 1) | retain);
	send_fixed_header(pcb, ctrl_field, remaining_len);

	send_utf8_encoded_str(pcb, (uint8_t*) topic, strlen(topic));
	uint16_t current_packet_id = 0;
	if (qos != 0)
	{
		current_packet_id = get_packet_id(last_packet_id);
		send_u16(pcb, &current_packet_id);
	}
	send_buffer(pcb, (uint8_t*) msg, strlen(msg));
}

void encode_mqtt_subscribe_msg(struct tcp_pcb* pcb, char* topic, uint8_t qos, uint16_t* last_packet_id)
{
	uint32_t remaining_len = get_subscribe_packet_len(topic);
	uint8_t ctrl_field = (MQTT_SUBSCRIBE_PACKET | 0x02);
	send_fixed_header(pcb, ctrl_field, remaining_len);

	uint16_t current_packet_id = get_packet_id(last_packet_id);
	send_u16(pcb, &current_packet_id);
	send_utf8_encoded_str(pcb, (uint8_t*) topic, strlen(topic));
	send_u8(pcb, &qos);
}

void encode_mqtt_pingreq_msg(struct tcp_pcb* pcb)
{
	send_fixed_header(pcb, MQTT_PINGREQ_PACKET, 0);
}

void encode_mqtt_disconnect_msg(struct tcp_pcb* pcb)
{
	send_fixed_header(pcb, MQTT_DISCONNECT_PACKET, 0);
}

void encode_mqtt_pubrel_msg(struct tcp_pcb* pcb, uint16_t* packet_id)
{
	send_fixed_header(pcb, (MQTT_PUBREL_PACKET | 0x02), 2);
	send_u16(pcb, packet_id);
}

void encode_mqtt_puback_msg(struct tcp_pcb* pcb, uint16_t* packet_id)
{
	send_fixed_header(pcb, MQTT_PUBACK_PACKET, 2);
	send_u16(pcb, packet_id);
}

void encode_mqtt_pubrec_msg(struct tcp_pcb* pcb, uint16_t* packet_id)
{
	send_fixed_header(pcb, MQTT_PUBREC_PACKET, 2);
	send_u16(pcb, packet_id);
}

void encode_mqtt_pubcomp_msg(struct tcp_pcb* pcb, uint16_t* packet_id)
{
	send_fixed_header(pcb, MQTT_PUBCOMP_PACKET, 2);
	send_u16(pcb, packet_id);
}