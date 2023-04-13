#ifndef MQTT_DECODE_H
#define MQTT_DECODE_H

#include <stdint.h>
#include "mqtt_packets.h"
#include "mqtt_client.h"

#define CONNACK_RESP_LEN 2
#define PUBACK_RESP_LEN 2
#define PUBREC_RESP_LEN 2
#define PUBCOMP_RESP_LEN 2
#define PUBREL_RESP_LEN 2
#define SUBACK_RESP_LEN 3

struct mqtt_header_t decode_mqtt_header(uint8_t* mqtt_data);
enum mqtt_client_err_t decode_connack_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_connack_resp_t* connack_resp);
enum mqtt_client_err_t decode_puback_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_puback_resp_t* puback_resp);
enum mqtt_client_err_t decode_pubrec_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubrec_resp_t* pubrec_resp);
enum mqtt_client_err_t decode_pubcomp_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubcomp_resp_t* pubcomp_resp);
enum mqtt_client_err_t decode_suback_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_suback_resp_t* suback_resp);
enum mqtt_client_err_t decode_publish_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_publish_resp_t* publish_resp);
enum mqtt_client_err_t decode_pubrel_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubrel_resp_t* pubrel_resp);



#endif
