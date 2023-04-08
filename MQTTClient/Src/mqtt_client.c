#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_cb_info.h"
#include "mqtt_serialize.h"

#define CLEAN_SESSION 1

static const uint16_t keepalive_sec = 5000;
static const uint32_t keepalive_ms = (uint32_t)keepalive_sec * 1000;
static char* protocol_name = "MQTT";
static uint8_t protocol_version = 0x04;

static void update_packet_id(uint16_t* last_packet_id)
{
	if ((*last_packet_id) == 65535)
	{
		(*last_packet_id) = 1;
		return;
	}
	(*last_packet_id)++;
}

void MQTTClient_init(struct mqtt_client_t* mqtt_client,
					 msg_received_cb_t msg_received_cb,
		             elapsed_time_cb_t elapsed_time_cb,
					 struct mqtt_client_connect_opts_t* conn_opts)
{
	mqtt_client->elapsed_time_cb = elapsed_time_cb;
	mqtt_client->last_activity = 0;
	mqtt_client->conn_opts = conn_opts;
	mqtt_client->last_packet_id = 1;

	MQTTCbInfo_init(&mqtt_client->cb_info, msg_received_cb);
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	if (mqtt_client->cb_info.mqtt_connected)
		return;

	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw, &mqtt_client->cb_info);

	uint16_t client_id_len = strlen(mqtt_client->conn_opts->client_id);
	uint16_t will_topic_len = (mqtt_client->conn_opts->will_topic != NULL) ? strlen(mqtt_client->conn_opts->will_topic) : 0;
	uint16_t will_msg_len = (mqtt_client->conn_opts->will_msg != NULL) ? strlen(mqtt_client->conn_opts->will_msg) : 0;
	uint16_t username_len = (mqtt_client->conn_opts->username != NULL) ? strlen(mqtt_client->conn_opts->username) : 0;
	uint16_t password_len = (mqtt_client->conn_opts->password != NULL) ? strlen(mqtt_client->conn_opts->password) : 0;

	uint32_t remaining_len = 10 + 2 + client_id_len;
	remaining_len += ((will_topic_len > 0) ? (2 + will_topic_len) : 0);
	remaining_len += ((will_msg_len > 0) ? (2 + will_msg_len) : 0);
	remaining_len += ((username_len > 0) ? (2 + username_len) : 0);
	remaining_len += ((password_len > 0) ? (2 + password_len) : 0);

	uint8_t connect_flags = ((username_len > 0) << 7) |((password_len > 0) << 6) | (mqtt_client->conn_opts->will_retain << 5) |
			                (mqtt_client->conn_opts->will_qos << 4) | ((will_msg_len > 0) << 2) | (CLEAN_SESSION << 1);

	// fix around: https://stackoverflow.com/questions/3025050/error-initializer-element-is-not-constant-when-trying-to-initialize-variable-w
	uint16_t ka = (uint16_t) keepalive_sec;
	uint8_t ctrl_field = (uint8_t) MQTT_CONNECT_PACKET;
	serialize_fixed_header(&mqtt_client->tcp_connection_raw, ctrl_field, remaining_len);
	serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) protocol_name, strlen(protocol_name));
	serialize_u8(&mqtt_client->tcp_connection_raw, &protocol_version);
	serialize_u8(&mqtt_client->tcp_connection_raw, &connect_flags);
	serialize_u16(&mqtt_client->tcp_connection_raw, &ka);

	// write payload
	serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->client_id, client_id_len);
	if (will_topic_len > 0)
		serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_topic, will_topic_len);
	if (will_msg_len > 0)
		serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_msg, will_msg_len);
	if (username_len > 0)
		serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->username, username_len);
	if (password_len > 0)
		serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->password, password_len);

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	TCPConnectionRaw_wait_for_condition(&mqtt_client->cb_info.mqtt_connected);
}

void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg, uint8_t qos, bool retain)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint32_t remaining_len = 2 + strlen(topic) + strlen(msg);
	if (qos != 0)
		remaining_len += 2;

	uint8_t ctrl_field = (MQTT_PUBLISH_PACKET | (0 << 3) | (qos << 1) | retain);
	serialize_fixed_header(&mqtt_client->tcp_connection_raw, ctrl_field, remaining_len);

	serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) topic, strlen(topic));
	if (qos != 0)
		serialize_u16(&mqtt_client->tcp_connection_raw, &mqtt_client->last_packet_id);

	serialize_buffer(&mqtt_client->tcp_connection_raw, (uint8_t*) msg, strlen(msg));

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	mqtt_client->cb_info.last_packet_id = mqtt_client->last_packet_id;

	if (qos == 0)
	{
		// do nothing - fire and forget
	}
	else if (qos == 1)
	{
		TCPConnectionRaw_wait_for_condition(&mqtt_client->cb_info.puback_received);
		mqtt_client->cb_info.puback_received = false;

		update_packet_id(&mqtt_client->last_packet_id);
	}
	else
	{
		TCPConnectionRaw_wait_for_condition(&mqtt_client->cb_info.pubrec_received);
		mqtt_client->cb_info.pubrec_received = false;

		uint8_t ctrl_field = (MQTT_PUBREL_PACKET | 0x02);
		serialize_fixed_header(&mqtt_client->tcp_connection_raw, ctrl_field, 2);
		serialize_u16(&mqtt_client->tcp_connection_raw, &mqtt_client->last_packet_id);

		TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

		TCPConnectionRaw_wait_for_condition(&mqtt_client->cb_info.pubcomp_received);
		mqtt_client->cb_info.pubcomp_received = false;

		update_packet_id(&mqtt_client->last_packet_id);
	}
}

void MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic, uint8_t qos)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint32_t remaining_len = 2 + 2 + strlen(topic) + 1;
	uint8_t ctrl_field = (MQTT_SUBSCRIBE_PACKET | 0x02);
	serialize_fixed_header(&mqtt_client->tcp_connection_raw, ctrl_field, remaining_len);

	serialize_u16(&mqtt_client->tcp_connection_raw, &mqtt_client->last_packet_id);
	serialize_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) topic, strlen(topic));
	serialize_u8(&mqtt_client->tcp_connection_raw, &qos);
	mqtt_client->cb_info.last_qos_subscribed = qos;

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	TCPConnectionRaw_wait_for_condition(&mqtt_client->cb_info.suback_received);
	mqtt_client->cb_info.suback_received = false;

	update_packet_id(&mqtt_client->last_packet_id);
}

void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client)
{
	uint32_t current_time = mqtt_client->elapsed_time_cb();
	if ((current_time - mqtt_client->last_activity >= keepalive_ms) && (mqtt_client->cb_info.mqtt_connected))
	{
		serialize_fixed_header(&mqtt_client->tcp_connection_raw, MQTT_PINGREQ_PACKET, 0);
		TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	}
}

void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	serialize_fixed_header(&mqtt_client->tcp_connection_raw, MQTT_DISCONNECT_PACKET, 0);
	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	TCPConnectionRaw_close(&mqtt_client->tcp_connection_raw);
	mqtt_client->cb_info.mqtt_connected = false;
}

void MQTTClient_loop(struct mqtt_client_t* mqtt_client)
{
	MQTTClient_keepalive(mqtt_client);
	TCPConnectionRaw_process_lwip_packets();
}
