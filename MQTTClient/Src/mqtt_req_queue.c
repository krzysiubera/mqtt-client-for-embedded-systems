#include <mqtt_req_queue.h>
#include <string.h>

void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue)
{
	memset(&req_queue->requests, 0, sizeof(req_queue->requests));
	req_queue->num_active_req = 0;
}

bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (!req_queue->requests[idx].active)
		{
			req_queue->requests[idx] = *req;
			req_queue->num_active_req++;
			return true;
		}
	}
	return false;
}

bool mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, enum mqtt_packet_type_t packet_type, uint16_t packet_id, uint8_t* idx_at_found)
{
	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		if (req_queue->requests[idx].active && req_queue->requests[idx].packet_id == packet_id)
		{
			req_queue->requests[idx].packet_type = packet_type;
			*idx_at_found = idx;
			return true;
		}
	}
	return false;
}

bool mqtt_req_queue_remove(struct mqtt_req_queue_t* req_queue, uint8_t idx_to_remove)
{
	if (!req_queue->requests[idx_to_remove].active)
		return false;

	req_queue->requests[idx_to_remove].active = false;
	req_queue->num_active_req--;
	return true;
}
