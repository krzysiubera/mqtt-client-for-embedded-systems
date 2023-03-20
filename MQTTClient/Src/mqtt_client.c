#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_cb_info.h"

#define MQTT_PROTOCOL_VERSION 0x04
#define CLEAN_SESSION 1
#define FIXED_HEADER_LEN 2

static const uint16_t keepalive_sec = 30;
static const uint32_t keepalive_ms = (uint32_t)(keepalive_sec) * 1000UL;
static uint16_t packet_id;

static uint16_t generate_packet_id()
{
	packet_id++;
	return packet_id;
}

static void send_utf8_encoded_str(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* msg, uint16_t len)
{
	uint8_t str_len_encoded[2];
	str_len_encoded[0] = (len >> 8) & 0xFF;
	str_len_encoded[1] = (len & 0xFF);
	TCPConnectionRaw_write(tcp_connection_raw, str_len_encoded, 2);
	TCPConnectionRaw_write(tcp_connection_raw, msg, len);
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

	bool is_will_present = (mqtt_client->conn_opts->will_topic != NULL);
	bool is_username_present = (mqtt_client->conn_opts->username != NULL);
	bool is_password_present = false;
	if (is_username_present)
		is_password_present = (mqtt_client->conn_opts->password != NULL);

	uint16_t client_id_len = strlen(mqtt_client->conn_opts->client_id);
	uint16_t will_topic_len = 0;
	uint16_t will_msg_len = 0;
	if (is_will_present)
	{
		will_topic_len = strlen(mqtt_client->conn_opts->will_topic);
		will_msg_len = strlen(mqtt_client->conn_opts->will_msg);
	}
	uint16_t username_len = 0;
	if (is_username_present)
		username_len = strlen(mqtt_client->conn_opts->username);
	uint16_t password_len = 0;
	if (is_password_present)
		password_len = strlen(mqtt_client->conn_opts->password);


	uint8_t remaining_len = 10 + 2 + client_id_len;
	if (is_will_present)
		remaining_len += (2 + will_topic_len + 2 + will_msg_len);
	if (is_username_present)
		remaining_len += (2 + username_len);
	if (is_password_present)
		remaining_len += (2 + password_len);


	uint8_t connect_flags = (is_username_present << 7) |
						    (is_password_present << 6) |
							(mqtt_client->conn_opts->will_retain << 5) |
			                (mqtt_client->conn_opts->will_qos << 4) |
							(is_will_present << 2) |
							(CLEAN_SESSION << 1);
	uint8_t header[12] = {
			MQTT_CONNECT_PACKET,
			remaining_len,
			0x00,
			0x04,
			'M',
			'Q',
			'T',
			'T',
			MQTT_PROTOCOL_VERSION,
			connect_flags,
			(keepalive_sec >> 8) & 0xFF,
			keepalive_sec & 0xFF
	};
	// write header
	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, header, sizeof(header));

	// write payload
	send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->client_id, client_id_len);
	if (is_will_present)
	{
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_topic, will_topic_len);
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->will_msg, will_msg_len);
	}
	if (is_username_present)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->username, username_len);
	if (is_password_present)
		send_utf8_encoded_str(&mqtt_client->tcp_connection_raw, (uint8_t*) mqtt_client->conn_opts->password, password_len);

	TCPConnectionRaw_output(&mqtt_client->tcp_connection_raw);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	TCPConnectionRaw_wait_until_mqtt_connected(&mqtt_client->cb_info);
}

void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint8_t topic_len = strlen(topic);
	uint8_t msg_len = strlen(msg);

	uint8_t remaining_len = FIXED_HEADER_LEN + topic_len + msg_len;
	uint8_t fixed_header[FIXED_HEADER_LEN] = {MQTT_PUBLISH_PACKET, remaining_len};

	uint8_t topic_len_encoded[] = {0x00, topic_len};
	uint8_t packet[FIXED_HEADER_LEN + remaining_len];

	memcpy(packet, fixed_header, FIXED_HEADER_LEN);
	memcpy(packet + FIXED_HEADER_LEN, topic_len_encoded, 2);
	memcpy(packet + FIXED_HEADER_LEN + 2, topic, topic_len);
	memcpy(packet + FIXED_HEADER_LEN + 2 + topic_len, msg, msg_len);

	TCPConnectionRaw_write_and_output(&mqtt_client->tcp_connection_raw, packet, FIXED_HEADER_LEN + remaining_len);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
}

void MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic)
{
	if (!mqtt_client->cb_info.mqtt_connected)
		return;

	uint8_t topic_len = strlen(topic);
	uint8_t remaining_len = 2 + 2 + topic_len + 1;   // msg_identifier + topic_len + topic + qos
	uint8_t fixed_header[FIXED_HEADER_LEN] = {(MQTT_SUBSCRIBE_PACKET | 2), remaining_len};

	uint16_t current_packet_id = generate_packet_id();
	uint8_t packet_id_encoded[2] = {(current_packet_id >> 8) & 0xFF, current_packet_id & 0xFF};
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
	mqtt_client->cb_info.last_subscribe_success = false;
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
