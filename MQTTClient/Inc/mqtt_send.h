#ifndef MQTT_SEND_H
#define MQTT_SEND_H

#include <stdint.h>
#include "mqtt_client.h"

void send_utf8_encoded_str(struct mqtt_client_t* mqtt_client, uint8_t* msg, uint16_t len);
void send_u16(struct mqtt_client_t* mqtt_client, uint16_t* val);
void send_u8(struct mqtt_client_t* mqtt_client, uint8_t* val);
void send_fixed_header(struct mqtt_client_t* mqtt_client, uint8_t ctrl_field, uint32_t remaining_len);
void send_buffer(struct mqtt_client_t* mqtt_client, uint8_t* buffer, uint16_t len);

#endif
