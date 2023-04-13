#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include "tcp_connection_raw.h"

enum mqtt_client_err_t
{
	MQTT_SUCCESS = 0,
	MQTT_NOT_CONNECTED = 1,
	MQTT_ALREADY_CONNECTED = 2,
	MQTT_SUBSCRIBE_FAILURE = 3,
	MQTT_PUBLISH_FAILURE = 4,
	MQTT_CONNECT_FAILURE = 5,
	MQTT_INVALID_MSG = 6
};


struct mqtt_client_connect_opts_t
{
	char* client_id;			// required - must not be null
	char* username;				// can be null
	char* password;				// can be null, if username is null, then password must be null as well

	char* will_topic;			// can be null
	char* will_msg;				// if will_topic is not null, then will_msg must not be null
	uint8_t will_qos;			// must be 0 if will_topic is null
	bool will_retain;			// must be false if will_topic is null
	uint16_t keepalive_ms;		// required
};


struct mqtt_client_t
{
	struct mqtt_cb_info_t cb_info;
	struct mqtt_client_connect_opts_t* conn_opts;
	uint16_t last_packet_id;
	bool mqtt_connected;
	struct tcp_connection_raw_t tcp_connection_raw;
};

void MQTTClient_init(struct mqtt_client_t* mqtt_client,
					 msg_received_cb_t msg_received_cb,
		             elapsed_time_cb_t elapsed_time_cb,
					 struct mqtt_client_connect_opts_t* conn_opts);
enum mqtt_client_err_t MQTTClient_connect(struct mqtt_client_t* mqtt_client);
enum mqtt_client_err_t MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg, uint8_t qos, bool retain);
enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic, uint8_t qos);
void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client);
void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client);
void MQTTClient_loop(struct mqtt_client_t* mqtt_client);


#endif
