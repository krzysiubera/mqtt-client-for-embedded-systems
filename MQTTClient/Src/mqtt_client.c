#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_cb_info.h"

#define MQTT_PROTOCOL_VERSION 0x04
#define CLEAN_SESSION 1
#define FIXED_HEADER_LEN 2

static const uint16_t keepalive_sec = 30;
static const uint32_t keepalive_ms = (uint32_t)(keepalive_sec) * 1000UL;

static uint16_t packet_id = 0;


static void send_utf8_encoded_str(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* msg, uint16_t len)
{
	uint8_t str_len_encoded[2];
	str_len_encoded[0] = (len >> 8) & 0xFF;
	str_len_encoded[1] = (len & 0xFF);
	TCPConnectionRaw_write(tcp_connection_raw, str_len_encoded, 2);
	TCPConnectionRaw_write(tcp_connection_raw, msg, len);
}

static void send_u16(struct tcp_connection_raw_t* tcp_connection_raw, uint16_t len)
{
	uint8_t u16_as_bytes[2];
	u16_as_bytes[0] = (len >> 8) & 0xFF;
	u16_as_bytes[1] = (len & 0xFF);
	TCPConnectionRaw_write(tcp_connection_raw, u16_as_bytes, 2);
}


void MQTTClient_init(struct mqtt_client_t* mqtt_client,
					 msg_received_cb_t msg_received_cb,
		             elapsed_time_cb_t elapsed_time_cb,
					 struct mqtt_client_connect_opts_t* conn_opts)
{
	mqtt_client->elapsed_time_cb = elapsed_time_cb;
	mqtt_client->last_activity = 0;
	mqtt_client->conn_opts = conn_opts;

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

	uint8_t remaining_len = 10 + 2 + client_id_len;
	remaining_len += ((will_topic_len > 0) ? (2 + will_topic_len) : 0);
	remaining_len += ((will_msg_len > 0) ? (2 + will_msg_len) : 0);
	remaining_len += ((username_len > 0) ? (2 + username_len) : 0);
	remaining_len += ((password_len > 0) ? (2 + password_len) : 0);

	uint8_t connect_flags = ((username_len > 0) << 7) |((password_len > 0) << 6) | (mqtt_client->conn_opts->will_retain << 5) |
			                (mqtt_client->conn_opts->will_qos << 4) | ((will_msg_len > 0) << 2) | (CLEAN_SESSION << 1);

	uint8_t header[12] = {MQTT_CONNECT_PACKET, remaining_len, 0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_PROTOCOL_VERSION, connect_flags,
			             (keepalive_sec >> 8) & 0xFF, keepalive_sec & 0xFF};

	// write header
	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, header, sizeof(header));

	// write payload
	send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->client_id, client_id_len);
	if (will_topic_len > 0)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_topic, will_topic_len);
	if (will_msg_len > 0)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_msg, will_msg_len);
	if (username_len > 0)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->username, username_len);
	if (password_len > 0)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->password, password_len);

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	TCPConnectionRaw_wait_until_mqtt_connected(&mqtt_client->cb_info);
}

void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg, enum mqtt_qos_t qos, bool retain)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint8_t remaining_len = 2 + strlen(topic) + 2 + strlen(msg);
	uint8_t fixed_header[2] = {(MQTT_PUBLISH_PACKET | (0 << 3) | (qos << 1) | retain), remaining_len};
	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, fixed_header, 2);

	send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) topic, strlen(topic));
	send_u16(&mqtt_client->tcp_connection_raw, packet_id);

	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, (uint8_t*) msg, strlen(msg));

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	if (qos == MQTT_QOS_0)
	{
		// do nothing - fire and forget
	}
	else if (qos == MQTT_QOS_1)
	{
		TCPConnectionRaw_wait_for_puback(&mqtt_client->cb_info);
		mqtt_client->cb_info.puback_received = false;
	}
	else
	{
		TCPConnectionRaw_wait_for_pubrec(&mqtt_client->cb_info);
		mqtt_client->cb_info.pubrec_received = false;

		// send pubrel
		uint8_t pubrel_pkt[4] = {(MQTT_PUBREL_PACKET | 0x02), 0x02, (packet_id >> 8) & 0xFF, (packet_id) & 0xFF};
		TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, pubrel_pkt, 4);
		TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

		TCPConnectionRaw_wait_for_pubcomp(&mqtt_client->cb_info);
		mqtt_client->cb_info.pubcomp_received = false;
	}

	packet_id++;
}

void MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint8_t topic_len = strlen(topic);
	uint8_t remaining_len = 2 + 2 + topic_len + 1;   // msg_identifier + topic_len + topic + qos
	uint8_t fixed_header[FIXED_HEADER_LEN] = {(MQTT_SUBSCRIBE_PACKET | 2), remaining_len};

	uint8_t packet_id_encoded[2] = {(packet_id >> 8) & 0xFF, packet_id & 0xFF};
	uint8_t topic_len_encoded[2] = {0x00, topic_len};
	uint8_t qos = 0;

	uint8_t packet[FIXED_HEADER_LEN + remaining_len];
	memcpy(packet, fixed_header, FIXED_HEADER_LEN);
	memcpy(packet + FIXED_HEADER_LEN, packet_id_encoded, 2);
	memcpy(packet + FIXED_HEADER_LEN + 2, topic_len_encoded, 2);
	memcpy(packet + FIXED_HEADER_LEN + 2 + 2, topic, topic_len);
	memcpy(packet + FIXED_HEADER_LEN + 2 + 2 + topic_len, &qos, 1);

	TCPConnectionRaw_write_and_output(&mqtt_client->tcp_connection_raw, packet, FIXED_HEADER_LEN + remaining_len);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	TCPConnectionRaw_wait_for_suback(&mqtt_client->cb_info);
	mqtt_client->cb_info.suback_received = false;

	packet_id++;
}

void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client)
{
	uint32_t current_time = mqtt_client->elapsed_time_cb();
	if ((current_time - mqtt_client->last_activity >= keepalive_ms) && (mqtt_client->cb_info.mqtt_connected))
	{
		uint8_t pingreq_msg[] = {MQTT_PINGREQ_PACKET, 0x00};
		TCPConnectionRaw_write_and_output(&mqtt_client->tcp_connection_raw, pingreq_msg, 2);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	}
}

void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint8_t disconnect_msg[] = {MQTT_DISCONNECT_PACKET, 0x00};
	TCPConnectionRaw_write_and_output(&mqtt_client->tcp_connection_raw, disconnect_msg, 2);
	TCPConnectionRaw_close(&mqtt_client->tcp_connection_raw);
	mqtt_client->cb_info.mqtt_connected = false;

}
