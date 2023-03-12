#include "mqtt_client.h"
#include "string.h"

#define KEEPALIVE 5000

void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id)
{
	mqtt_client->client_id = client_id;
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw);

	size_t len_client_id = strlen(mqtt_client->client_id);
	uint8_t fixed_header[] = {(1 << 4), 10 + len_client_id + 2};
	uint8_t variable_header[] = {0x00, 0x04, 0x4D, 0x51, 0x54, 0x54, 0x04, 0x02, 0x00, KEEPALIVE / 1000, 0x00, len_client_id};

	size_t len_fixed_header = sizeof(fixed_header);
	size_t len_variable_header = sizeof(variable_header);
	size_t len_packet = len_fixed_header + len_variable_header + len_client_id;


	uint8_t packet[len_packet];

	memcpy(packet, fixed_header, len_fixed_header);
	memcpy(packet + len_fixed_header, variable_header, len_variable_header);
	memcpy(packet + len_fixed_header + len_variable_header, mqtt_client->client_id, len_client_id);

	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet, len_packet);
}
