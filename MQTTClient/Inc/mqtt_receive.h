#ifndef MQTT_RECEIVE_H
#define MQTT_RECEIVE_H

#include <stdint.h>
#include "mqtt_client.h"

enum mqtt_client_err_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_client_t* mqtt_client,
		                               uint32_t* bytes_left);

#endif
