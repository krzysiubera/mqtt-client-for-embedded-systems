#include "mqtt_encode.h"
#include <stdint.h>
#include <string.h>
#include "mqtt_packets.h"
#include "mqtt_send.h"
#include "mqtt_helpers.h"
#include "mqtt_config.h"

#define CLEAN_SESSION 1
static char* protocol_name = "MQTT";
static uint8_t protocol_version = 0x04;

static uint16_t get_connect_packet_len(const struct mqtt_client_connect_opts_t* conn_opts)
{
	uint16_t len = 10 + 2 + strlen(conn_opts->client_id);
	if (conn_opts->will_msg.topic)
		len += (2 + strlen(conn_opts->will_msg.topic));
	if (conn_opts->will_msg.payload)
		len += (2 + strlen(conn_opts->will_msg.payload));
	if (conn_opts->username)
		len += (2 + strlen(conn_opts->username));
	if (conn_opts->password)
		len += (2 + strlen(conn_opts->password));
	return len;
}

static uint8_t get_connect_flags(const struct mqtt_client_connect_opts_t* conn_opts)
{
	return ((conn_opts->username ? 1 : 0) << 7) |
			((conn_opts->password ? 1 : 0) << 6) |
			(conn_opts->will_msg.retain << 5) |
            (conn_opts->will_msg.qos << 4) |
			((conn_opts->will_msg.payload ? 1 : 0) << 2) |
			(CLEAN_SESSION << 1);
}

static uint16_t get_publish_packet_len(char* topic, char* msg, uint8_t qos)
{
	uint16_t remaining_len = 2 + strlen(topic) + strlen(msg);
	if (qos != 0)
		remaining_len += 2;
	return remaining_len;
}

static uint16_t get_subscribe_packet_len(char* topic)
{
	return 2 + 2 + strlen(topic) + 1;
}

static uint16_t get_unsubscribe_packet_len(char* topic)
{
	return 2 + 2 + strlen(topic);
}

static uint16_t get_packet_id(uint16_t* last_packet_id)
{
	if ((*last_packet_id) == 65535)
	{
		(*last_packet_id) = 0;
	}
	(*last_packet_id)++;
	return (*last_packet_id);
}

static uint16_t get_packet_len(uint16_t remaining_len)
{
	return remaining_len + 1 + get_digits_remaining_len(remaining_len);
}

enum mqtt_client_err_t encode_mqtt_connect_msg(struct mqtt_client_t* mqtt_client)
{
	uint16_t remaining_len = get_connect_packet_len(mqtt_client->conn_opts);
	uint16_t packet_len = get_packet_len(remaining_len);
	if (packet_len > TCPHandler_get_space_in_output_buffer(mqtt_client->pcb))
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	uint8_t ctrl_field = (uint8_t) MQTT_CONNECT_PACKET;
	uint8_t connect_flags = get_connect_flags(mqtt_client->conn_opts);
	send_fixed_header(mqtt_client->pcb, ctrl_field, remaining_len);
	send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) protocol_name, strlen(protocol_name));
	send_u8(mqtt_client->pcb, &protocol_version);
	send_u8(mqtt_client->pcb, &connect_flags);

	uint16_t keepalive_seconds = mqtt_client->conn_opts->keepalive_ms / 1000;
	send_u16(mqtt_client->pcb, &keepalive_seconds);

	char* client_id = mqtt_client->conn_opts->client_id;
	send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) client_id, strlen(client_id));

	struct mqtt_pub_msg_t will_msg = mqtt_client->conn_opts->will_msg;
	if (will_msg.topic)
		send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) will_msg.topic, strlen(will_msg.topic));
	if (will_msg.payload)
		send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) will_msg.payload, strlen(will_msg.payload));

	char* username = mqtt_client->conn_opts->username;
	if (username)
		send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) username, strlen(username));

	char* password = mqtt_client->conn_opts->password;
	if (password)
		send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) password, strlen(password));
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_publish_msg(struct mqtt_client_t* mqtt_client, struct mqtt_pub_msg_t* pub_msg)
{
	uint16_t remaining_len = get_publish_packet_len(pub_msg->topic, pub_msg->payload, pub_msg->qos);
	uint16_t packet_len = get_packet_len(remaining_len);
	if (packet_len > TCPHandler_get_space_in_output_buffer(mqtt_client->pcb))
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	if (pub_msg->qos != 0 && mqtt_client->req_queue.num_active_req == MQTT_REQUESTS_QUEUE_LEN)
		return MQTT_REQUESTS_QUEUE_FULL;

	if (strlen(pub_msg->payload) + 1 > MQTT_MAX_PAYLOAD_LEN)
		return MQTT_MESSAGE_TOO_LONG;

	if (strlen(pub_msg->topic) + 1 > MQTT_MAX_TOPIC_LEN)
		return MQTT_TOPIC_TOO_LONG;

	uint8_t ctrl_field = (MQTT_PUBLISH_PACKET | (0 << 3) | (pub_msg->qos << 1) | pub_msg->retain);
	send_fixed_header(mqtt_client->pcb, ctrl_field, remaining_len);

	send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) pub_msg->topic, strlen(pub_msg->topic));
	uint16_t current_packet_id = 0;
	if (pub_msg->qos != 0)
	{
		current_packet_id = get_packet_id(&mqtt_client->last_packet_id);
		send_u16(mqtt_client->pcb, &current_packet_id);
	}
	send_buffer(mqtt_client->pcb, (uint8_t*) pub_msg->payload, strlen(pub_msg->payload));
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_subscribe_msg(struct mqtt_client_t* mqtt_client, struct mqtt_sub_msg_t* sub_msg)
{
	uint16_t remaining_len = get_subscribe_packet_len(sub_msg->topic);
	uint16_t packet_len = get_packet_len(remaining_len);
	if (packet_len > TCPHandler_get_space_in_output_buffer(mqtt_client->pcb))
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	if (mqtt_client->req_queue.num_active_req == MQTT_REQUESTS_QUEUE_LEN)
		return MQTT_REQUESTS_QUEUE_FULL;

	if (strlen(sub_msg->topic) + 1 > MQTT_MAX_TOPIC_LEN)
		return MQTT_TOPIC_TOO_LONG;

	uint8_t ctrl_field = (MQTT_SUBSCRIBE_PACKET | 0x02);
	send_fixed_header(mqtt_client->pcb, ctrl_field, remaining_len);

	uint16_t current_packet_id = get_packet_id(&mqtt_client->last_packet_id);
	send_u16(mqtt_client->pcb, &current_packet_id);
	send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) sub_msg->topic, strlen(sub_msg->topic));
	send_u8(mqtt_client->pcb, &sub_msg->qos);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_pingreq_msg(struct mqtt_client_t* mqtt_client)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, MQTT_PINGREQ_PACKET, 0);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_disconnect_msg(struct mqtt_client_t* mqtt_client)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, MQTT_DISCONNECT_PACKET, 0);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_pubrel_msg(struct mqtt_client_t* mqtt_client, uint16_t* packet_id)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, (MQTT_PUBREL_PACKET | 0x02), 2);
	send_u16(mqtt_client->pcb, packet_id);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_puback_msg(struct mqtt_client_t* mqtt_client, uint16_t* packet_id)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, MQTT_PUBACK_PACKET, 2);
	send_u16(mqtt_client->pcb, packet_id);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_pubrec_msg(struct mqtt_client_t* mqtt_client, uint16_t* packet_id)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, MQTT_PUBREC_PACKET, 2);
	send_u16(mqtt_client->pcb, packet_id);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_pubcomp_msg(struct mqtt_client_t* mqtt_client, uint16_t* packet_id)
{
	if (TCPHandler_get_space_in_output_buffer(mqtt_client->pcb) < 2)
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	send_fixed_header(mqtt_client->pcb, MQTT_PUBCOMP_PACKET, 2);
	send_u16(mqtt_client->pcb, packet_id);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t encode_mqtt_unsubscribe_msg(struct mqtt_client_t* mqtt_client, struct mqtt_unsub_msg_t* unsub_msg)
{
	uint16_t remaining_len = get_unsubscribe_packet_len(unsub_msg->topic);
	uint16_t packet_len = get_packet_len(remaining_len);
	if (packet_len > TCPHandler_get_space_in_output_buffer(mqtt_client->pcb))
		return MQTT_NOT_ENOUGH_SPACE_IN_OUTPUT_BUFFER;

	if (mqtt_client->req_queue.num_active_req == MQTT_REQUESTS_QUEUE_LEN)
		return MQTT_REQUESTS_QUEUE_FULL;

	if (strlen(unsub_msg->topic) + 1 > MQTT_MAX_TOPIC_LEN)
		return MQTT_TOPIC_TOO_LONG;

	uint8_t ctrl_field = (MQTT_UNSUBSCRIBE_PACKET | 0x02);
	send_fixed_header(mqtt_client->pcb, ctrl_field, remaining_len);

	uint16_t current_packet_id = get_packet_id(&mqtt_client->last_packet_id);
	send_u16(mqtt_client->pcb, &current_packet_id);
	send_utf8_encoded_str(mqtt_client->pcb, (uint8_t*) unsub_msg->topic, strlen(unsub_msg->topic));
	return MQTT_SUCCESS;
}
