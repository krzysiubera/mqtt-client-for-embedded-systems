#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "tcp_connection_raw.h"

struct mqtt_client_t
{
	struct tcp_connection_raw_t tcp_connection_raw;
	struct mqtt_client_cb_info_t client_cb_info;
	const char* client_id;
};

void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id, msg_received_cb_t msg_received_cb);
void MQTTClient_connect(struct mqtt_client_t* mqtt_client);
void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg);
void MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic);


#endif
