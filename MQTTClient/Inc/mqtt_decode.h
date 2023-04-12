#ifndef MQTT_DECODE_H
#define MQTT_DECODE_H

#include <stdint.h>
#include "mqtt_packets.h"

struct mqtt_header_t decode_mqtt_header(uint8_t* mqtt_data);
struct mqtt_connack_msg_t decode_connack_msg(uint8_t* mqtt_data, struct mqtt_header_t connack_header);
struct mqtt_puback_msg_t decode_puback_msg(uint8_t* mqtt_data, struct mqtt_header_t puback_header);
struct mqtt_pubrec_msg_t decode_pubrec_msg(uint8_t* mqtt_data, struct mqtt_header_t pubrec_header);
struct mqtt_pubcomp_msg_t decode_pubcomp_msg(uint8_t* mqtt_data, struct mqtt_header_t pubcomp_header);
struct mqtt_suback_msg_t decode_suback_msg(uint8_t* mqtt_data, struct mqtt_header_t suback_header);

#endif
