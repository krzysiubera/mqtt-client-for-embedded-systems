#ifndef MQTT_RECEIVE_H
#define MQTT_RECEIVE_H

#include <stdint.h>
#include "mqtt_client.h"

uint32_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_client_t* mqtt_client);

#endif
