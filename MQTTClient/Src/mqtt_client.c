#include <string.h>
#include "mqtt_client.h"
#include "mqtt_packets.h"
#include "mqtt_decode.h"
#include "mqtt_encode.h"
#include "tcp_connection_raw.h"

static const uint32_t timeout_on_connect_response_ms = 5000;

void wait_for_connack(struct mqtt_client_t* mqtt_client)
{
	bool expired = false;
	uint32_t entry_time_ms = mqtt_client->elapsed_time_cb();
	do
	{
		TCPHandler_process_lwip_packets();
		expired = (mqtt_client->elapsed_time_cb() - entry_time_ms > timeout_on_connect_response_ms);
	} while ((!mqtt_client->connack_resp_available) && (!expired));
}

void MQTTClient_init(struct mqtt_client_t* mqtt_client, elapsed_time_cb_t elapsed_time_cb,
					 const struct mqtt_client_connect_opts_t* conn_opts)
{
	mqtt_client->conn_opts = conn_opts;
	mqtt_client->last_packet_id = 0;
	mqtt_client->mqtt_connected = false;
	mqtt_client->pcb = NULL;
	memset(&mqtt_client->connack_resp, 0, sizeof(mqtt_client->connack_resp));
	mqtt_client->connack_resp_available = false;
	mqtt_client->last_activity = 0;
	mqtt_client->elapsed_time_cb = elapsed_time_cb;
	mqtt_req_queue_init(&mqtt_client->req_queue);
	mqtt_client->on_msg_received_cb = NULL;
	mqtt_client->on_pub_completed_cb = NULL;
	mqtt_client->on_sub_completed_cb = NULL;
}

void MQTTClient_set_cb_on_msg_received(struct mqtt_client_t* mqtt_client, on_msg_received_cb_t on_msg_received_cb)
{
	mqtt_client->on_msg_received_cb = on_msg_received_cb;
}

void MQTTClient_set_cb_on_sub_completed(struct mqtt_client_t* mqtt_client, on_sub_completed_cb_t on_sub_completed_cb)
{
	mqtt_client->on_sub_completed_cb = on_sub_completed_cb;
}

void MQTTClient_set_cb_on_pub_completed(struct mqtt_client_t* mqtt_client, on_pub_completed_cb_t on_pub_completed_cb)
{
	mqtt_client->on_pub_completed_cb = on_pub_completed_cb;
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
		return MQTT_TCP_CONNECT_FAILURE;

	rc = encode_mqtt_connect_msg(mqtt_client->pcb, mqtt_client->conn_opts);
	if (rc != MQTT_SUCCESS)
		return rc;

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	wait_for_connack(mqtt_client);
	if (!mqtt_client->connack_resp_available)
		return MQTT_TIMEOUT_ON_CONNECT;

	mqtt_client->mqtt_connected = (*(&mqtt_client->connack_resp.conn_rc) == MQTT_CONNECTION_ACCEPTED);
	return (mqtt_client->mqtt_connected) ? MQTT_SUCCESS : MQTT_CONNECTION_REFUSED_BY_BROKER;
}

enum mqtt_client_err_t MQTTClient_publish(struct mqtt_client_t* mqtt_client, struct mqtt_pub_msg_t* pub_msg)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	enum mqtt_client_err_t rc = encode_mqtt_publish_msg(mqtt_client->pcb, pub_msg, &mqtt_client->last_packet_id,
			                                            mqtt_client->req_queue.num_active_req);
	if (rc != MQTT_SUCCESS)
		return rc;

	uint16_t current_packet_id = mqtt_client->last_packet_id;

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	if (pub_msg->qos == 1)
	{
		union mqtt_context_t pub_context;
		save_mqtt_pub_context(&pub_context, pub_msg);
		struct mqtt_req_t puback_req = { .packet_id=current_packet_id, .context=pub_context, .conv_state=MQTT_WAITING_FOR_PUBACK };
		mqtt_req_queue_add(&mqtt_client->req_queue, &puback_req);
	}
	else if (pub_msg->qos == 2)
	{
		union mqtt_context_t pub_context;
		save_mqtt_pub_context(&pub_context, pub_msg);
		struct mqtt_req_t pubrec_req = { .packet_id=current_packet_id, .context=pub_context, .conv_state=MQTT_WAITING_FOR_PUBREC };
		mqtt_req_queue_add(&mqtt_client->req_queue, &pubrec_req);
	}
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t MQTTClient_subscribe(struct mqtt_client_t* mqtt_client, struct mqtt_sub_msg_t* sub_msg)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	enum mqtt_client_err_t rc = encode_mqtt_subscribe_msg(mqtt_client->pcb, sub_msg, &mqtt_client->last_packet_id,
			                                              mqtt_client->req_queue.num_active_req);
	if (rc != MQTT_SUCCESS)
		return rc;

	uint16_t current_packet_id = mqtt_client->last_packet_id;

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	union mqtt_context_t sub_context;
	save_mqtt_sub_context(&sub_context, sub_msg);
	struct mqtt_req_t suback_req = { .packet_id=current_packet_id, .context=sub_context, .conv_state=MQTT_WAITING_FOR_SUBACK };
	mqtt_req_queue_add(&mqtt_client->req_queue, &suback_req);

	return MQTT_SUCCESS;
}

void MQTTClient_keepalive(struct mqtt_client_t* mqtt_client)
{
	uint32_t current_time = mqtt_client->elapsed_time_cb();
	if ((current_time - mqtt_client->last_activity >= mqtt_client->conn_opts->keepalive_ms) && (mqtt_client->mqtt_connected))
	{
		enum mqtt_client_err_t rc = encode_mqtt_pingreq_msg(mqtt_client->pcb);
		if (rc != MQTT_SUCCESS)
		{
			TCPHandler_close(mqtt_client->pcb);
			mqtt_client->mqtt_connected = false;
			return;
		}

		TCPHandler_output(mqtt_client->pcb);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
	}
}

enum mqtt_client_err_t MQTTClient_disconnect(struct mqtt_client_t* mqtt_client)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	enum mqtt_client_err_t rc = encode_mqtt_disconnect_msg(mqtt_client->pcb);
	if (rc == MQTT_SUCCESS)
		TCPHandler_output(mqtt_client->pcb);

	/* we have to close TCP connection anyways - either with sending MQTT disconnect packet, or not */
	TCPHandler_close(mqtt_client->pcb);
	mqtt_client->mqtt_connected = false;
	return MQTT_SUCCESS;
}

void MQTTClient_loop(struct mqtt_client_t* mqtt_client)
{
	TCPHandler_process_lwip_packets();
	if (mqtt_client->mqtt_connected)
	{
		MQTTClient_keepalive(mqtt_client);
	}
}

enum mqtt_client_err_t MQTTClient_unsubscribe(struct mqtt_client_t* mqtt_client, struct mqtt_unsub_msg_t* unsub_msg)
{
	if (!mqtt_client->mqtt_connected)
		return MQTT_NOT_CONNECTED;

	enum mqtt_client_err_t rc = encode_mqtt_unsubscribe_msg(mqtt_client->pcb, unsub_msg, &mqtt_client->last_packet_id,
			                                                mqtt_client->req_queue.num_active_req);
	if (rc != MQTT_SUCCESS)
		return rc;

	uint16_t current_packet_id = mqtt_client->last_packet_id;

	TCPHandler_output(mqtt_client->pcb);
	mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

	union mqtt_context_t unsub_context;
	save_mqtt_unsub_context(&unsub_context, unsub_msg);
	struct mqtt_req_t unsub_req = { .packet_id=current_packet_id, .context=unsub_context, .conv_state=MQTT_WAITING_FOR_UNSUBACK };
	mqtt_req_queue_add(&mqtt_client->req_queue, &unsub_req);

	return MQTT_SUCCESS;
}
