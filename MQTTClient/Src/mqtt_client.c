#include "mqtt_client.h"
#include "string.h"

#define KEEPALIVE 5000

void MQTTClient_init(struct mqtt_client_t* mqtt_client, char* device_id)
{
	mqtt_client->device_id = device_id;
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw);

	size_t len_device_id = strlen(mqtt_client->device_id);
	uint8_t fixed_header[] = {(1 << 4), 10 + len_device_id + 2};
	uint8_t variable_header[] = {0x00, 0x04, 0x4D, 0x51, 0x54, 0x54, 0x04, 0x02, 0x00, KEEPALIVE / 1000, 0x00, len_device_id};

	char packet[sizeof(fixed_header) + sizeof(variable_header) + len_device_id];
	memset(packet, 0, sizeof(packet));

	size_t len_fixed_header = sizeof(fixed_header);
	size_t len_variable_header = sizeof(variable_header);

	memcpy(packet, fixed_header, len_fixed_header);
	memcpy(packet + len_fixed_header, variable_header, len_variable_header);
	memcpy(packet + len_fixed_header + len_variable_header, mqtt_client->device_id, len_device_id);

	TCPConnectionRaw_write(&mqtt_client->tcp_connection_raw, packet);
}
