#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"

#define KEEPALIVE_SEC 10
#define FIXED_HEADER_LEN 2

void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id)
{
	mqtt_client->client_id = client_id;
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw);

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
}

void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg)
{
	uint8_t topic_len = strlen(topic);
	uint8_t msg_len = strlen(msg);
	uint8_t remaining_len = 2 + topic_len + 2 + msg_len;
	uint8_t fixed_header[FIXED_HEADER_LEN] = {MQTT_PUBLISH_PACKET, remaining_len};

	uint8_t packet_len = 2 + remaining_len;
	uint8_t packet[packet_len];

	uint8_t topic_len_encoded[] = {0x00, topic_len};
	uint8_t msg_len_encoded[] = {0x00, msg_len};

	memcpy(packet, fixed_header, FIXED_HEADER_LEN);
	memcpy(packet + FIXED_HEADER_LEN, topic_len_encoded, 2);
	memcpy(packet + FIXED_HEADER_LEN + 2, topic, topic_len);
	memcpy(packet + FIXED_HEADER_LEN + 2 + topic_len, msg_len_encoded, 2);
	memcpy(packet + FIXED_HEADER_LEN + 2 + topic_len + 2, msg, msg_len);

	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet, packet_len);
}
