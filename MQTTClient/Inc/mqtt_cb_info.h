#ifndef MQTT_CB_INFO_H
#define MQTT_CB_INFO_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*msg_received_cb_t)(uint8_t* topic, uint16_t topic_len, uint8_t* data, uint32_t data_len, uint8_t qos);

struct mqtt_cb_info_t
{
	uint8_t* mqtt_payload;
	uint16_t mqtt_msg_len;
	msg_received_cb_t msg_received_cb;
};

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb);

#endif
