#include "mqtt_validate.h"
#include "tcp_connection_raw.h"

bool is_output_buffer_full(struct mqtt_client_t* mqtt_client, uint16_t packet_len)
{
	return packet_len > TCPHandler_get_space_in_output_buffer(mqtt_client->pcb);
}

bool is_request_queue_full(struct mqtt_client_t* mqtt_client)
{
	return mqtt_client->req_queue.num_active_req == MQTT_REQUESTS_QUEUE_LEN;
}

bool is_message_too_long(uint16_t message_len)
{
	return message_len > MQTT_MAX_PAYLOAD_LEN;
}

bool is_topic_too_long(uint16_t topic_len)
{
	return topic_len > MQTT_MAX_TOPIC_LEN;
}
