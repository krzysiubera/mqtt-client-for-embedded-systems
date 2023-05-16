#ifndef MQTT_CONTEXT_H
#define MQTT_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>
#include "mqtt_config.h"
#include "mqtt_message.h"
#include "mqtt_packets.h"

struct mqtt_pub_context_t
{
	uint8_t topic[MQTT_MAX_TOPIC_LEN];
	uint16_t topic_len;
	uint8_t payload[MQTT_MAX_PAYLOAD_LEN];
	uint16_t payload_len;
	uint8_t qos;
	bool retain;
};

struct mqtt_sub_context_t
{
	uint8_t topic[MQTT_MAX_TOPIC_LEN];
	uint16_t topic_len;
	uint8_t qos;
};

union mqtt_context_t
{
	struct mqtt_pub_context_t pub;
	struct mqtt_sub_context_t sub;
};

void save_mqtt_pub_context(union mqtt_context_t* context, struct mqtt_pub_msg_t* pub_msg);
void save_mqtt_sub_context(union mqtt_context_t* context, struct mqtt_sub_msg_t* sub_msg);
void create_mqtt_context_from_pub_response(union mqtt_context_t* context, struct mqtt_publish_resp_t* publish_resp);

#endif
