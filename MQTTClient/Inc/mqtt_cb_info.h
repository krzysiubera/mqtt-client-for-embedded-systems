#ifndef MQTT_CB_INFO_H
#define MQTT_CB_INFO_H

#include <stdbool.h>
#include <stdint.h>
#include "mqtt_packets.h"

typedef void (*msg_received_cb_t)(struct mqtt_publish_resp_t* publish_resp);
typedef uint32_t (*elapsed_time_cb_t)();

struct mqtt_cb_info_t
{
	struct mqtt_connack_resp_t connack_resp;
	bool connack_resp_available;
	msg_received_cb_t msg_received_cb;
	uint32_t last_activity;
	elapsed_time_cb_t elapsed_time_cb;
};

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb, elapsed_time_cb_t elapsed_time_cb);

#endif
