#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_decode.h"
#include "mqtt_encode.h"
#include "tcp_connection_raw.h"

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

	encode_mqtt_connect_msg(mqtt_client->pcb, mqtt_client->conn_opts);

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

	uint16_t current_packet_id = encode_mqtt_publish_msg(mqtt_client->pcb, topic, msg, qos, retain, &mqtt_client->last_packet_id);

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	if (qos == 0)
	{
		// do nothing
	}
	else if (qos == 1)
	{
		// put to queue PUBACK req
		return MQTT_SUCCESS;
	}
	else
	{
		// put to queue PUBREC req
	}
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, char* topic, uint8_t qos)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	uint16_t current_packet_id = encode_mqtt_subscribe_msg(mqtt_client->pcb, topic, qos, &mqtt_client->last_packet_id);

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
		encode_mqtt_pingreq_msg(mqtt_client->pcb);
		TCPHandler_output(mqtt_client->pcb);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	}
}

void MQTTClient_disconnect(struct mqtt_client_t* mqtt_client)
{
	if (!mqtt_client->mqtt_connected)
		return;

	encode_mqtt_disconnect_msg(mqtt_client->pcb);
	TCPHandler_output(mqtt_client->pcb);
	TCPHandler_close(mqtt_client->pcb);
	mqtt_client->mqtt_connected = false;
}

void MQTTClient_loop(struct mqtt_client_t* mqtt_client)
{
	MQTTClient_keepalive(mqtt_client);
	TCPHandler_process_lwip_packets();
}
