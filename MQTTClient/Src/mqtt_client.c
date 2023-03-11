#include "mqtt_client.h"

void MQTTClient_init(struct mqtt_client_t* mqtt_client, char* device_id)
{
	mqtt_client->device_id = device_id;
	TCPConnectionRaw_init(&mqtt_client->tcp_connection_raw);
}

void MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	TCPConnectionRaw_connect(&mqtt_client->tcp_connection_raw);
}
