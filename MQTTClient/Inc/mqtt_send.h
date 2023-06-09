#ifndef MQTT_SEND_H
#define MQTT_SEND_H

#include <stdint.h>
#include "tcp_connection_raw.h"

void send_utf8_encoded_str(struct tcp_pcb* pcb, uint8_t* msg, uint16_t len);
void send_u16(struct tcp_pcb* pcb, uint16_t* val);
void send_u8(struct tcp_pcb* pcb, uint8_t* val);
void send_fixed_header(struct tcp_pcb* pcb, uint8_t ctrl_field, uint16_t remaining_len);
void send_buffer(struct tcp_pcb* pcb, uint8_t* buffer, uint16_t len);

#endif
