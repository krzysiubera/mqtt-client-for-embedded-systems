#ifndef MQTT_REQUESTS_QUEUE_H
#define MQTT_REQUESTS_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include "mqtt_packets.h"
#include "mqtt_config.h"
#include "mqtt_context.h"

struct mqtt_req_t
{
	enum mqtt_packet_type_t packet_type;
	uint16_t packet_id;
	union mqtt_context_t context;
	bool active;
};

struct mqtt_req_queue_t
{
	struct mqtt_req_t requests[MQTT_REQUESTS_QUEUE_LEN];
	uint8_t num_active_req;
};

void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue);
bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req);
bool mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, enum mqtt_packet_type_t packet_type, uint16_t packet_id, uint8_t* idx_at_found);
bool mqtt_req_queue_remove(struct mqtt_req_queue_t* req_queue, uint8_t idx_to_remove);

#endif
