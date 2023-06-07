#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include "mqtt_packets.h"
#include "mqtt_req_queue.h"
#include "mqtt_message.h"
#include "mqtt_context.h"

typedef void (*on_msg_received_cb_t)(union mqtt_context_t* context);
typedef uint32_t (*elapsed_time_cb_t)();
typedef void (*on_sub_completed_cb_t)(struct mqtt_suback_resp_t* suback_resp, union mqtt_context_t* context);
typedef void (*on_pub_completed_cb_t)(union mqtt_context_t* context);

enum mqtt_client_err_t
{
	MQTT_SUCCESS = 0,
	MQTT_NOT_CONNECTED = 1,
	MQTT_ALREADY_CONNECTED = 2,
	MQTT_TCP_CONNECT_FAILURE = 3,
	MQTT_INVALID_MSG = 4,
	MQTT_MEMORY_ERR = 5,
	MQTT_TIMEOUT_ON_CONNECT = 6,
	MQTT_CONNECTION_REFUSED_BY_BROKER = 7,
};

struct mqtt_client_connect_opts_t
{
	char* client_id;			// required - must not be null
	char* username;				// can be null
	char* password;				// can be null, if username is null, then password must be null as well

	/* topic can be null,
	 * if topic is not null, then payload must not be null,
	 * qos must be 0 if will_topic is null,
	 * retain must be false if will_topic is null */
	struct mqtt_pub_msg_t will_msg;
	uint32_t keepalive_ms;		// required - max allowed value - 65 535 000
};


struct mqtt_client_t
{
	const struct mqtt_client_connect_opts_t* conn_opts;
	uint16_t last_packet_id;
	bool mqtt_connected;
	struct tcp_pcb* pcb;
	struct mqtt_connack_resp_t connack_resp;
	bool connack_resp_available;
	on_msg_received_cb_t on_msg_received_cb;
	on_sub_completed_cb_t on_sub_completed_cb;
	on_pub_completed_cb_t on_pub_completed_cb;
	uint32_t last_activity;
	elapsed_time_cb_t elapsed_time_cb;
	uint32_t timeout_on_connect_response_ms;
	struct mqtt_req_queue_t req_queue;
};

void MQTTClient_init(struct mqtt_client_t* mqtt_client, elapsed_time_cb_t elapsed_time_cb,
					 const struct mqtt_client_connect_opts_t* conn_opts, uint32_t timeout_on_connect_response_ms);
void MQTTClient_set_cb_on_msg_received(struct mqtt_client_t* mqtt_client, on_msg_received_cb_t on_msg_received_cb);
void MQTTClient_set_cb_on_sub_completed(struct mqtt_client_t* mqtt_client, on_sub_completed_cb_t on_sub_completed_cb);
void MQTTClient_set_cb_on_pub_completed(struct mqtt_client_t* mqtt_client, on_pub_completed_cb_t on_pub_completed_cb);
enum mqtt_client_err_t MQTTClient_connect(struct mqtt_client_t* mqtt_client);
enum mqtt_client_err_t MQTTClient_publish(struct mqtt_client_t* mqtt_client, struct mqtt_pub_msg_t* pub_msg);
enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, struct mqtt_sub_msg_t* sub_msg);
void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client);
enum mqtt_client_err_t MQTTClient_disconnect(struct mqtt_client_t* mqtt_client);
void MQTTClient_loop(struct mqtt_client_t* mqtt_client);
enum mqtt_client_err_t MQTTClient_unsubscribe(struct mqtt_client_t* mqtt_client, struct mqtt_unsub_msg_t* unsub_msg);


#endif
