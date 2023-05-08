#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include "mqtt_packets.h"
#include "mqtt_req_queue.h"

typedef void (*on_msg_received_cb_t)(struct mqtt_publish_resp_t* publish_resp);
typedef uint32_t (*elapsed_time_cb_t)();

enum mqtt_client_err_t
{
	MQTT_SUCCESS = 0,
	MQTT_NOT_CONNECTED = 1,
	MQTT_ALREADY_CONNECTED = 2,
	MQTT_TCP_CONNECT_FAILURE = 3,
	MQTT_INVALID_MSG = 4,
	MQTT_MEMORY_ERR = 5,
	MQTT_TIMEOUT_ON_CONNECT = 6,
	MQTT_CONNECTION_REFUSED_BY_BROKER = 7
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
	uint32_t keepalive_ms;		// required - max allowed value - 65 535 000
};


struct mqtt_client_t
{
	struct mqtt_client_connect_opts_t* conn_opts;
	uint16_t last_packet_id;
	bool mqtt_connected;

	struct tcp_pcb* pcb;

	struct mqtt_connack_resp_t connack_resp;
	bool connack_resp_available;
	on_msg_received_cb_t on_msg_received_cb;
	uint32_t last_activity;
	elapsed_time_cb_t elapsed_time_cb;
	uint32_t timeout_on_connect_response_ms;

	struct mqtt_req_queue_t req_queue;
};

void MQTTClient_init(struct mqtt_client_t* mqtt_client,
					 on_msg_received_cb_t on_msg_received_cb,
		             elapsed_time_cb_t elapsed_time_cb,
					 struct mqtt_client_connect_opts_t* conn_opts,
					 uint32_t timeout_on_connect_response_ms);
enum mqtt_client_err_t MQTTClient_connect(struct mqtt_client_t* mqtt_client);
enum mqtt_client_err_t MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg, uint8_t qos, bool retain);
enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic, uint8_t qos);
void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client);
void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client);
void MQTTClient_loop(struct mqtt_client_t* mqtt_client);


#endif
