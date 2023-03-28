#ifndef MQTT_SERIALIZE_H
#define MQTT_SERIALIZE_H

#include <stdint.h>
#include "tcp_connection_raw.h"

void send_utf8_encoded_str(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* msg, uint16_t len);
void send_u16(struct tcp_connection_raw_t* tcp_connection_raw, uint16_t* val);
void send_u8(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* val);
void send_fixed_header(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t ctrl_field, uint32_t remaining_len);

#endif
