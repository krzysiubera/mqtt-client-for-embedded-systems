#ifndef MQTT_REQUESTS_QUEUE_H
#define MQTT_REQUESTS_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include "mqtt_config.h"
#include "mqtt_context.h"

enum mqtt_conv_state_t
{
	MQTT_NO_REQUEST = 0,
	MQTT_WAITING_FOR_PUBACK = 1,
	MQTT_WAITING_FOR_SUBACK = 2,
	MQTT_WAITING_FOR_PUBREC = 3,
	MQTT_WAITING_FOR_PUBCOMP = 4,
	MQTT_WAITING_FOR_PUBREL = 5,
	MQTT_WAITING_FOR_UNSUBACK = 6
};

struct mqtt_req_t
{
	uint16_t packet_id;
	union mqtt_context_t context;
	enum mqtt_conv_state_t conv_state;
};

struct mqtt_req_queue_t
{
	struct mqtt_req_t requests[MQTT_REQUESTS_QUEUE_LEN];
	uint8_t num_active_req;
};

void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue);
bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req);
bool mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, enum mqtt_conv_state_t current_conv_state,
		                   uint16_t packet_id, uint8_t* idx_at_found);

#endif
