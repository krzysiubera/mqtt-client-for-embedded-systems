#ifndef MQTT_ENCODE_H
#define MQTT_ENCODE_H

#include <stdbool.h>
#include "mqtt_client.h"
#include "tcp_connection_raw.h"

void encode_mqtt_connect_msg(struct tcp_pcb* pcb, struct mqtt_client_connect_opts_t* conn_opts);
uint16_t encode_mqtt_publish_msg(struct tcp_pcb* pcb, char* topic, char* msg, uint8_t qos, bool retain, uint16_t* last_packet_id);
uint16_t encode_mqtt_subscribe_msg(struct tcp_pcb* pcb, char* topic, uint8_t qos, uint16_t* last_packet_id);
void encode_mqtt_pingreq_msg(struct tcp_pcb* pcb);
void encode_mqtt_disconnect_msg(struct tcp_pcb* pcb);
void encode_mqtt_pubrel_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
void encode_mqtt_puback_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
void encode_mqtt_pubrec_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
void encode_mqtt_pubcomp_msg(struct tcp_pcb* pcb, uint16_t* packet_id);

#endif
