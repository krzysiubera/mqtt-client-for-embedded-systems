#include <mqtt_req_queue.h>
#include <string.h>

static enum mqtt_conv_state_t get_next_conv_state(enum mqtt_conv_state_t current_conv_state)
{
	if (current_conv_state == MQTT_WAITING_FOR_PUBACK)
		return MQTT_NO_REQUEST;
	else if (current_conv_state == MQTT_WAITING_FOR_SUBACK)
		return MQTT_NO_REQUEST;
	else if (current_conv_state == MQTT_WAITING_FOR_PUBREC)
		return MQTT_WAITING_FOR_PUBCOMP;
	else if (current_conv_state == MQTT_WAITING_FOR_PUBCOMP)
		return MQTT_NO_REQUEST;
	else if (current_conv_state == MQTT_WAITING_FOR_PUBREL)
		return MQTT_NO_REQUEST;
	else if (current_conv_state == MQTT_WAITING_FOR_UNSUBACK)
		return MQTT_NO_REQUEST;
	else
		return MQTT_NO_REQUEST;
}


void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue)
{
	memset(&req_queue->requests, 0, sizeof(req_queue->requests));
	req_queue->num_active_req = 0;
}

bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (req_queue->requests[idx].conv_state == MQTT_NO_REQUEST)
		{
			req_queue->requests[idx] = *req;
			req_queue->num_active_req++;
			return true;
		}
	}
	return false;
}

bool mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, enum mqtt_conv_state_t current_conv_state,
		                   uint16_t packet_id, uint8_t* idx_at_found)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (req_queue->requests[idx].conv_state == current_conv_state && req_queue->requests[idx].packet_id == packet_id)
		{
			enum mqtt_conv_state_t next_state = get_next_conv_state(current_conv_state);
			if (next_state == MQTT_NO_REQUEST)
				req_queue->num_active_req--;
			req_queue->requests[idx].conv_state = next_state;
			*idx_at_found = idx;
			return true;
		}
	}
	return false;
}
