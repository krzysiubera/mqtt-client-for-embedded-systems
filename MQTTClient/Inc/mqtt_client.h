#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "tcp_connection_raw.h"

struct mqtt_client_t
{
	struct tcp_connection_raw_t tcp_connection_raw;
	char* device_id;
};

void MQTTClient_init(struct mqtt_client_t* mqtt_client, char* device_id);
void MQTTClient_connect(struct mqtt_client_t* mqtt_client);


#endif
