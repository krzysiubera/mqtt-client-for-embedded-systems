#ifndef MQTT_CB_INFO_H
#define MQTT_CB_INFO_H

#include <stdbool.h>

typedef void (*msg_received_cb_t)(uint8_t* topic, uint16_t topic_len, uint8_t* data, uint32_t data_len);

struct mqtt_cb_info_t
{
	bool mqtt_connected;
	bool last_subscribe_success;
	msg_received_cb_t msg_received_cb;
};

#endif
