#ifndef TCP_CONNECTION_RAW_H
#define TCP_CONNECTION_RAW_H

#include "lwip/tcp.h"
#include "mqtt_client.h"

void TCPHandler_process_lwip_packets();
void TCPHandler_set_ip_address(ip_addr_t* broker_ip_addr);
struct tcp_pcb* TCPHandler_get_pcb();
enum mqtt_client_err_t TCPHandler_connect(struct mqtt_client_t* mqtt_client);
void TCPHandler_write(struct tcp_pcb* pcb, uint8_t* packet, uint16_t len_packet);
void TCPHandler_output(struct tcp_pcb* pcb);
void TCPHandler_close(struct tcp_pcb* pcb);

#endif
