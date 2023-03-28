#ifndef MQTT_DESERIALIZE_H
#define MQTT_DESERIALIZE_H

#include <stdint.h>
#include "mqtt_cb_info.h"

void deserialize_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info_ptr);

#endif
