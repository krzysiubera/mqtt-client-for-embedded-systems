#include <mqtt_req_queue.h>
#include <string.h>

void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue)
{
	memset(&req_queue->requests, 0, sizeof(req_queue->requests));
	req_queue->head = 0;
	req_queue->tail = 0;
	req_queue->count = 0;
}

bool mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req)
{
	if (req_queue->count == MQTT_REQUESTS_QUEUE_LEN)
		return false;

	req_queue->requests[req_queue->head] = *req;
	req_queue->head = (req_queue->head + 1) % MQTT_REQUESTS_QUEUE_LEN;
	req_queue->count++;
	return true;
}

bool mqtt_req_queue_remove(struct mqtt_req_queue_t* req_queue)
{
	if (req_queue->count == 0)
		return false;

	req_queue->tail = (req_queue->tail + 1) % MQTT_REQUESTS_QUEUE_LEN;
	req_queue->count--;
	return true;
}

bool mqtt_req_queue_update(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* item_to_update)
{
	if (req_queue->count == 0)
		return false;

	for (uint8_t idx = 0; idx < MQTT_REQUESTS_QUEUE_LEN; ++idx)
	{
		struct mqtt_req_t searched_req = req_queue->requests[idx];
		if (searched_req.packet_id == item_to_update->packet_id)
		{
			req_queue->requests[idx].packet_type = item_to_update->packet_type;
			return true;
		}
	}
	return false;
}
