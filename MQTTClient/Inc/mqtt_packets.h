#ifndef MQTT_PACKETS_H
#define MQTT_PACKETS_H

#include <stdint.h>
#include <stdbool.h>
#include "mqtt_rc.h"

enum mqtt_packet_type_t
{
	MQTT_CONNECT_PACKET = (1 << 4),
	MQTT_CONNACK_PACKET = (2 << 4),
	MQTT_PUBLISH_PACKET = (3 << 4),
	MQTT_PUBACK_PACKET = (4 << 4),
	MQTT_PUBREC_PACKET = (5 << 4),
	MQTT_PUBREL_PACKET = (6 << 4),
	MQTT_PUBCOMP_PACKET = (7 << 4),
	MQTT_SUBSCRIBE_PACKET = (8 << 4),
	MQTT_SUBACK_PACKET = (9 << 4),
	MQTT_UNSUBSCRIBE_PACKET = (10 << 4),
	MQTT_UNSUBACK_PACKET = (11 << 4),
	MQTT_PINGREQ_PACKET = (12 << 4),
	MQTT_PINGRESP_PACKET = (13 << 4),
	MQTT_DISCONNECT_PACKET = (14 << 4)
};

struct mqtt_header_t
{
	enum mqtt_packet_type_t packet_type;
	uint32_t remaining_len;
	uint8_t digits_remaining_len;
};

struct mqtt_connack_resp_t
{
	bool session_present;
	enum mqtt_connection_rc_t conn_rc;
};

struct mqtt_puback_resp_t
{
	uint16_t packet_id;
};

struct mqtt_pubrec_resp_t
{
	uint16_t packet_id;
};

struct mqtt_pubcomp_resp_t
{
	uint16_t packet_id;
};

struct mqtt_suback_resp_t
{
	uint16_t packet_id;
	enum mqtt_suback_rc_t suback_rc;
};

struct mqtt_publish_resp_t
{
	uint8_t* topic;
	uint16_t topic_len;
	uint8_t* data;
	uint16_t data_len;
	uint16_t packet_id;
	uint8_t qos;
};

#endif
