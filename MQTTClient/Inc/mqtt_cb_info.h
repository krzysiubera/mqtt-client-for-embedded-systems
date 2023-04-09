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

	bool puback_received;
	bool pubrec_received;
	bool pubcomp_received;
	uint16_t last_packet_id;
	uint8_t last_qos_subscribed;
	msg_received_cb_t msg_received_cb;
};

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb);

#endif
