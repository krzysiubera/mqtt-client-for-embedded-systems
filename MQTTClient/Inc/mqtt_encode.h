#ifndef MQTT_ENCODE_H
#define MQTT_ENCODE_H

#include <stdbool.h>
#include "mqtt_client.h"
#include "tcp_connection_raw.h"

enum mqtt_client_err_t encode_mqtt_connect_msg(struct tcp_pcb* pcb, const struct mqtt_client_connect_opts_t* conn_opts);
enum mqtt_client_err_t encode_mqtt_publish_msg(struct tcp_pcb* pcb, struct mqtt_pub_msg_t* pub_msg, uint16_t* last_packet_id);
enum mqtt_client_err_t encode_mqtt_subscribe_msg(struct tcp_pcb* pcb, struct mqtt_sub_msg_t* sub_msg, uint16_t* last_packet_id);
enum mqtt_client_err_t encode_mqtt_pingreq_msg(struct tcp_pcb* pcb);
enum mqtt_client_err_t encode_mqtt_disconnect_msg(struct tcp_pcb* pcb);
enum mqtt_client_err_t encode_mqtt_pubrel_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
enum mqtt_client_err_t encode_mqtt_puback_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
enum mqtt_client_err_t encode_mqtt_pubrec_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
enum mqtt_client_err_t encode_mqtt_pubcomp_msg(struct tcp_pcb* pcb, uint16_t* packet_id);
enum mqtt_client_err_t encode_mqtt_unsubscribe_msg(struct tcp_pcb* pcb, struct mqtt_unsub_msg_t* unsub_msg, uint16_t* last_packet_id);

#endif
