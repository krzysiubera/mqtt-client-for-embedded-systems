#ifndef MQTT_RECEIVE_H
#define MQTT_RECEIVE_H

#include <stdint.h>
#include "mqtt_cb_info.h"

uint32_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info);

#endif
