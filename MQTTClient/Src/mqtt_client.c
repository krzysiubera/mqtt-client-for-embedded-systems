#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_send.h"
#include "mqtt_decode.h"
#include "tcp_connection_raw.h"

#define CLEAN_SESSION 1

static char* protocol_name = "MQTT";
static uint8_t protocol_version = 0x04;

static uint16_t get_packet_id(uint16_t* last_packet_id)
{
	if ((*last_packet_id) == 65535)
	{
		(*last_packet_id) = 0;
	}
	(*last_packet_id)++;
	return (*last_packet_id);
}

void wait_for_connack(struct mqtt_client_t* mqtt_client)
{
	while (!mqtt_client->connack_resp_available)
		TCPHandler_process_lwip_packets();
}

void MQTTClient_init(struct mqtt_client_t* mqtt_client,
					 msg_received_cb_t msg_received_cb,
		             elapsed_time_cb_t elapsed_time_cb,
					 struct mqtt_client_connect_opts_t* conn_opts)
{
	mqtt_client->conn_opts = conn_opts;
	mqtt_client->last_packet_id = 0;
	mqtt_client->mqtt_connected = false;
	mqtt_client->pcb = NULL;
	memset(&mqtt_client->connack_resp, 0, sizeof(mqtt_client->connack_resp));
	mqtt_client->connack_resp_available = false;
	mqtt_client->msg_received_cb = msg_received_cb;
	mqtt_client->last_activity = 0;
	mqtt_client->elapsed_time_cb = elapsed_time_cb;

	TCPHandler_set_ip_address(&mqtt_client->broker_ip_addr);
}

enum mqtt_client_err_t MQTTClient_connect(struct mqtt_client_t* mqtt_client)
{
	if (mqtt_client->mqtt_connected)
		return MQTT_ALREADY_CONNECTED;

	mqtt_client->pcb = TCPHandler_get_pcb();
	if (mqtt_client->pcb == NULL)
		return MQTT_MEMORY_ERR;

	enum mqtt_client_err_t rc = TCPHandler_connect(mqtt_client);
	if (rc != MQTT_SUCCESS)
		return MQTT_CONNECT_FAILURE;

	uint16_t client_id_len = strlen(mqtt_client->conn_opts->client_id);
	uint16_t will_topic_len = (mqtt_client->conn_opts->will_topic != NULL) ? strlen(mqtt_client->conn_opts->will_topic) : 0;
	uint16_t will_msg_len = (mqtt_client->conn_opts->will_msg != NULL) ? strlen(mqtt_client->conn_opts->will_msg) : 0;
	uint16_t username_len = (mqtt_client->conn_opts->username != NULL) ? strlen(mqtt_client->conn_opts->username) : 0;
	uint16_t password_len = (mqtt_client->conn_opts->password != NULL) ? strlen(mqtt_client->conn_opts->password) : 0;

	uint32_t remaining_len = 10 + 2 + client_id_len;
	remaining_len += ((will_topic_len > 0) ? (2 + will_topic_len) : 0);
	remaining_len += ((will_msg_len > 0) ? (2 + will_msg_len) : 0);
	remaining_len += ((username_len > 0) ? (2 + username_len) : 0);
	remaining_len += ((password_len > 0) ? (2 + password_len) : 0);

	uint8_t connect_flags = ((username_len > 0) << 7) |((password_len > 0) << 6) | (mqtt_client->conn_opts->will_retain << 5) |
			                (mqtt_client->conn_opts->will_qos << 4) | ((will_msg_len > 0) << 2) | (CLEAN_SESSION << 1);

	uint8_t ctrl_field = (uint8_t) MQTT_CONNECT_PACKET;
	send_fixed_header(mqtt_client, ctrl_field, remaining_len);
	send_utf8_encoded_str(mqtt_client, (uint8_t*) protocol_name, strlen(protocol_name));
	send_u8(mqtt_client, &protocol_version);
	send_u8(mqtt_client, &connect_flags);

	uint16_t keepalive_seconds = mqtt_client->conn_opts->keepalive_ms / 1000;
	send_u16(mqtt_client, &keepalive_seconds);

	// write payload
	send_utf8_encoded_str(mqtt_client, (uint8_t*) mqtt_client->conn_opts->client_id, client_id_len);
	if (will_topic_len > 0)
		send_utf8_encoded_str(mqtt_client, (uint8_t*) mqtt_client->conn_opts->will_topic, will_topic_len);
	if (will_msg_len > 0)
		send_utf8_encoded_str(mqtt_client, (uint8_t*) mqtt_client->conn_opts->will_msg, will_msg_len);
	if (username_len > 0)
		send_utf8_encoded_str(mqtt_client, (uint8_t*) mqtt_client->conn_opts->username, username_len);
	if (password_len > 0)
		send_utf8_encoded_str(mqtt_client, (uint8_t*) mqtt_client->conn_opts->password, password_len);

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	wait_for_connack(mqtt_client);
	mqtt_client->mqtt_connected = (*(&mqtt_client->connack_resp.conn_rc) == MQTT_CONNECTION_ACCEPTED);
	return (mqtt_client->mqtt_connected) ? MQTT_SUCCESS : MQTT_CONNECT_FAILURE;
}

enum mqtt_client_err_t MQTTClient_publish(struct mqtt_client_t* mqtt_client, char* topic, char* msg, uint8_t qos, bool retain)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	uint32_t remaining_len = 2 + strlen(topic) + strlen(msg);
	if (qos != 0)
		remaining_len += 2;

	uint8_t ctrl_field = (MQTT_PUBLISH_PACKET | (0 << 3) | (qos << 1) | retain);
	send_fixed_header(mqtt_client, ctrl_field, remaining_len);

	send_utf8_encoded_str(mqtt_client, (uint8_t*) topic, strlen(topic));
	uint16_t current_packet_id = 0;
	if (qos != 0)
	{
		current_packet_id = get_packet_id(&mqtt_client->last_packet_id);
		send_u16(mqtt_client, &current_packet_id);
	}

	send_buffer(mqtt_client, (uint8_t*) msg, strlen(msg));

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	if (qos == 0)
	{
		return MQTT_SUCCESS;
	}
	else if (qos == 1)
	{
		// put to queue PUBACK req
		return MQTT_SUCCESS;
	}
	else
	{
		// put to queue PUBREC req

		return MQTT_SUCCESS;
	}
}

enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic, uint8_t qos)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	uint32_t remaining_len = 2 + 2 + strlen(topic) + 1;
	uint8_t ctrl_field = (MQTT_SUBSCRIBE_PACKET | 0x02);
	send_fixed_header(mqtt_client, ctrl_field, remaining_len);

	uint16_t current_packet_id = get_packet_id(&mqtt_client->last_packet_id);
	send_u16(mqtt_client, &current_packet_id);
	send_utf8_encoded_str(mqtt_client, (uint8_t*) topic, strlen(topic));
	send_u8(mqtt_client, &qos);

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	// put SUBACK request to queue
	return MQTT_SUCCESS;
}

void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client)
{
	uint32_t current_time = mqtt_client->elapsed_time_cb();
	if ((current_time - mqtt_client->last_activity >= mqtt_client->conn_opts->keepalive_ms) && (mqtt_client->mqtt_connected))
	{
		send_fixed_header(mqtt_client, MQTT_PINGREQ_PACKET, 0);
		TCPHandler_output(mqtt_client->pcb);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	}
}

void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client)
{
	if (!mqtt_client->mqtt_connected)
		return;

	send_fixed_header(mqtt_client, MQTT_DISCONNECT_PACKET, 0);
	TCPHandler_output(mqtt_client->pcb);
	TCPHandler_close(mqtt_client->pcb);
	mqtt_client->mqtt_connected = false;
}

void MQTTClient_loop(struct mqtt_client_t* mqtt_client)
{
	MQTTClient_keepalive(mqtt_client);
	TCPHandler_process_lwip_packets();
}
