#ifndef MQTT_VALIDATE_H
#define MQTT_VALIDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "mqtt_client.h"

bool is_output_buffer_full(struct mqtt_client_t* mqtt_client, uint16_t packet_len);
bool is_request_queue_full(struct mqtt_client_t* mqtt_client);
bool is_message_too_long(uint16_t message_len);
bool is_topic_too_long(uint16_t topic_len);

#endif
