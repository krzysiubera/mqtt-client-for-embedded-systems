#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_cb_info.h"

#define KEEPALIVE_SEC 10
#define FIXED_HEADER_LEN 2

static uint16_t packet_id;

static uint16_t generate_packet_id()
{
	packet_id++;
	return packet_id;
}

void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id)
{
	mqtt_client->client_id = client_id;
	mqtt_client->client_cb_info.mqtt_connected = false;
	mqtt_client->client_cb_info.last_subscribe_success = false;
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw, &mqtt_client->client_cb_info);

	size_t len_client_id = strlen(mqtt_client->client_id);
	uint8_t fixed_header[FIXED_HEADER_LEN] = {MQTT_CONNECT_PACKET, 10 + len_client_id + 2};
	uint8_t variable_header[] = {0x00, 0x04, 0x4D, 0x51, 0x54, 0x54, 0x04, 0x02, 0x00, KEEPALIVE_SEC, 0x00, len_client_id};

	size_t len_variable_header = sizeof(variable_header);
	size_t len_packet = FIXED_HEADER_LEN + len_variable_header + len_client_id;

	uint8_t packet[len_packet];

	memcpy(packet, fixed_header, FIXED_HEADER_LEN);
	memcpy(packet + FIXED_HEADER_LEN, variable_header, len_variable_header);
	memcpy(packet + FIXED_HEADER_LEN + len_variable_header, mqtt_client->client_id, len_client_id);

	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet, len_packet);
	TCPConnectionRaw_wait_until_mqtt_connected(&mqtt_client->client_cb_info);
}

void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg)
{
	if (!mqtt_client->client_cb_info.mqtt_connected)
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


	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet, FIXED_HEADER_LEN + remaining_len);
}

void MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic)
{
	if (!mqtt_client->client_cb_info.mqtt_connected)
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
	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet, FIXED_HEADER_LEN + remaining_len);

	TCPConnectionRaw_wait_for_suback(&mqtt_client->client_cb_info);
	mqtt_client->client_cb_info.last_subscribe_success = false;
}
