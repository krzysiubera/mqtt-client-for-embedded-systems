#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "tcp_connection_raw.h"

struct mqtt_client_t
{
	struct tcp_connection_raw_t tcp_connection_raw;
	const char* client_id;
};

enum mqtt_packet_type_t
{
	MQTT_CONNECT_PACKET = 1,
	MQTT_CONNACK_PACKET = 2,
	MQTT_PUBLISH_PACKET = 3,
	MQTT_PUBACK_PACKET = 4,
	MQTT_PUBREC_PACKET = 5,
	MQTT_PUBREL_PACKET = 6,
	MQTT_PUBCOMP_PACKET = 7,
	MQTT_SUBSCRIBE_PACKET = 8,
	MQTT_SUBACK_PACKET = 9,
	MQTT_UNSUBSCRIBE_PACKET = 10,
	MQTT_UNSUBACK_PACKET = 11,
	MQTT_PINGREQ_PACKET = 12,
	MQTT_PINGRESP_PACKET = 13,
	MQTT_DISCONNECT_PACKET = 14
};


void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id);
void MQTTClient_connect(struct mqtt_client_t* mqtt_client);


#endif
