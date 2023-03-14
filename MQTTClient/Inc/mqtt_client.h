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
	MQTT_CONNECT_PACKET = (1 << 4),
	MQTT_CONNACK_PACKET = (2 << 4),
	MQTT_PUBLISH_PACKET = (3 << 4),
	MQTT_PUBACK_PACKET = (4 << 4),
	MQTT_PUBREC_PACKET = (5 << 4),
	MQTT_PUBREL_PACKET = (6 << 4),
	MQTT_PUBCOMP_PACKET = (7 << 4),
	MQTT_SUBSCRIBE_PACKET = (8 << 4),
	MQTT_SUBACK_PACKET = (9 << 4),
	MQTT_UNSUBSCRIBE_PACKET = (10 << 4),
	MQTT_UNSUBACK_PACKET = (11 << 4),
	MQTT_PINGREQ_PACKET = (12 << 4),
	MQTT_PINGRESP_PACKET = (13 << 4),
	MQTT_DISCONNECT_PACKET = (14 << 4)
};

enum mqtt_connection_rc_t
{
	MQTT_CONNECTION_ACCEPTED = 0,
	MQTT_UNACCEPTABLE_PROTOCOL_VERSION = 1,
	MQTT_IDENTIFIER_REJECTED = 2,
	MQTT_SERVER_UNAVAILABLE = 3,
	MQTT_BAD_CREDENTIALS = 4,
	MQTT_NOT_AUTHORIZED = 5
};


void MQTTClient_init(struct mqtt_client_t* mqtt_client, const char* client_id);
void MQTTClient_connect(struct mqtt_client_t* mqtt_client);
void MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg);


#endif
