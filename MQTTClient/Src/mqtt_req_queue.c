#include <mqtt_req_queue.h>
#include <string.h>

/*
 * subscribing to topic - MQTT_WAITING_FOR_SUBACK -> MQTT_NO_REQUEST
 * unsubscribing from topic - MQTT_WAITING_FOR_UNSUBACK -> MQTT_NO_REQUEST
 * sending a message with QOS 1 - MQTT_WAITING_FOR_PUBACK -> MQTT_NO_REQUEST
 * sending a message with QOS 2 - MQTT_WAITING_FOR_PUBREC -> MQTT_SENDING_PUBREL -> MQTT_WAITING_FOR_PUBCOMP -> MQTT_NO_REQUEST
 * receiving a message with QOS 1 - MQTT_SENDING_PUBACK -> MQTT_NO_REQUEST
 * receiving a message with QOS 2 - MQTT_SENDING_PUBREC -> MQTT_WAITING_FOR_PUBREL -> MQTT_SENDING_PUBCOMP -> MQTT_NO_REQUEST
 */

static enum mqtt_conv_state_t get_next_conv_state(enum mqtt_conv_state_t current_conv_state)
{
	switch (current_conv_state)
	{
	case MQTT_WAITING_FOR_SUBACK:
		return MQTT_NO_REQUEST;
	case MQTT_WAITING_FOR_UNSUBACK:
		return MQTT_NO_REQUEST;
	case MQTT_WAITING_FOR_PUBACK:
		return MQTT_NO_REQUEST;
	case MQTT_WAITING_FOR_PUBREC:
		return MQTT_SENDING_PUBREL;
	case MQTT_SENDING_PUBREL:
		return MQTT_WAITING_FOR_PUBCOMP;
	case MQTT_WAITING_FOR_PUBCOMP:
		return MQTT_NO_REQUEST;
	case MQTT_SENDING_PUBACK:
		return MQTT_NO_REQUEST;
	case MQTT_SENDING_PUBREC:
		return MQTT_WAITING_FOR_PUBREL;
	case MQTT_WAITING_FOR_PUBREL:
		return MQTT_SENDING_PUBCOMP;
	case MQTT_SENDING_PUBCOMP:
		return MQTT_NO_REQUEST;
	default:
		return MQTT_NO_REQUEST;
	}
}


void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue)
{
	memset(&req_queue->requests, 0, sizeof(req_queue->requests));
	req_queue->num_active_req = 0;
}

bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req, uint8_t* idx_at_added)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (req_queue->requests[idx].conv_state == MQTT_NO_REQUEST)
		{
			req_queue->requests[idx] = *req;
			*idx_at_added = idx;
			req_queue->num_active_req++;
			return true;
		}
	}
	return false;
}

bool mqtt_req_queue_find(struct mqtt_req_queue_t* req_queue, enum mqtt_conv_state_t current_conv_state,
		                 uint16_t packet_id, uint8_t* idx_at_found)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (req_queue->requests[idx].conv_state == current_conv_state && req_queue->requests[idx].packet_id == packet_id)
		{
			enum mqtt_conv_state_t next_state = get_next_conv_state(current_conv_state);
			req_queue->requests[idx].conv_state = next_state;
			if (next_state == MQTT_NO_REQUEST)
				req_queue->num_active_req--;
			*idx_at_found = idx;
			return true;
		}
	}
	return false;
}
void mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, enum mqtt_conv_state_t current_conv_state, uint8_t idx_to_update)
{
	enum mqtt_conv_state_t next_state = get_next_conv_state(current_conv_state);
	req_queue->requests[idx_to_update].conv_state = next_state;
	if (next_state == MQTT_NO_REQUEST)
		req_queue->num_active_req--;
}
