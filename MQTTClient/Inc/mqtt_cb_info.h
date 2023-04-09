#ifndef MQTT_CB_INFO_H
#define MQTT_CB_INFO_H

#include <stdbool.h>
#include <stdint.h>
#include "mqtt_packets.h"

typedef void (*msg_received_cb_t)(uint8_t* topic, uint16_t topic_len, uint8_t* data, uint32_t data_len, uint8_t qos);

struct mqtt_cb_info_t
{

	struct mqtt_connack_msg_t connack_msg;
	bool connack_msg_available;

	struct mqtt_suback_msg_t suback_msg;
	bool suback_msg_available;

	struct mqtt_puback_msg_t puback_msg;
	bool puback_msg_available;

	struct mqtt_pubrec_msg_t pubrec_msg;
	bool pubrec_msg_available;

	struct mqtt_pubcomp_msg_t pubcomp_msg;
	bool pubcomp_msg_available;

	msg_received_cb_t msg_received_cb;
};

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb);

#endif
