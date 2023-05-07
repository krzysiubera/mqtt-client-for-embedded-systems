#include <mqtt_req_queue.h>
#include <string.h>

#include <stdio.h>

void mqtt_req_queue_init(struct mqtt_req_queue_t* req_queue)
{
	memset(&req_queue->requests, 0, sizeof(req_queue->requests));
	req_queue->head = 0;
	req_queue->tail = 0;
	req_queue->count = 0;
}

void mqtt_req_queue_add(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req)
{
	if (req_queue->count && (req_queue->tail % MQTT_REQUESTS_QUEUE_LEN) == req_queue->head)
	{
		req_queue->head = (req_queue->head + 1) % MQTT_REQUESTS_QUEUE_LEN;
		req_queue->count--;
	}

	req_queue->requests[req_queue->tail] = *req;
	req_queue->tail = (req_queue->tail + 1) % MQTT_REQUESTS_QUEUE_LEN;
	req_queue->count++;
}

bool mqtt_req_queue_get(struct mqtt_req_queue_t* req_queue, struct mqtt_req_t* req)
{
	if (req_queue->count == 0)
		return false;

	uint8_t start = req_queue->head;
	uint8_t end = req_queue->tail;

	uint8_t count = 0;

	uint8_t found_at_iter = 0;
	for (uint8_t idx = start; count < req_queue->count; idx = (idx + 1) % MQTT_REQUESTS_QUEUE_LEN)
	{
		struct mqtt_req_t req_in_buff = req_queue->requests[idx];

		if (req_in_buff.packet_type == req->packet_type && req_in_buff.packet_id == req->packet_id)
		{
			printf("Found at iter no: %d\n", count);
			return true;
		}
		found_at_iter++;

		count++;
		if (idx == (end - 1))
			break;
	}
	return false;
}
