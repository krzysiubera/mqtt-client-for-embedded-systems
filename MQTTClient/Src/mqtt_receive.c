#include "mqtt_req_queue.h"
#include "mqtt_receive.h"
#include "mqtt_decode.h"
#include "mqtt_encode.h"
#include "tcp_connection_raw.h"

enum mqtt_client_err_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_client_t* mqtt_client,
		                               uint32_t* bytes_left)
{
	struct mqtt_header_t header = decode_mqtt_header(mqtt_data);

	switch (header.packet_type)
	{
	case MQTT_CONNACK_PACKET:
	{
		struct mqtt_connack_resp_t connack_resp;
		enum mqtt_client_err_t rc = decode_connack_resp(mqtt_data, &header, &connack_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		mqtt_client->connack_resp = connack_resp;
		mqtt_client->connack_resp_available = true;

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - CONNACK_RESP_LEN;
		return MQTT_SUCCESS;
	}
	case MQTT_PUBACK_PACKET:
	{
		struct mqtt_puback_resp_t puback_resp;
		enum mqtt_client_err_t rc = decode_puback_resp(mqtt_data, &header, &puback_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		struct mqtt_req_t puback_req = { .packet_type=MQTT_PUBACK_PACKET, .packet_id=puback_resp.packet_id};
		bool found = mqtt_req_queue_update(&mqtt_client->req_queue, &puback_req);
		if (!found)
			return MQTT_ERROR_PARSING_MSG;

		mqtt_req_queue_remove(&mqtt_client->req_queue);

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - PUBACK_RESP_LEN;
		return MQTT_SUCCESS;
	}
	case MQTT_PUBREC_PACKET:
	{
		struct mqtt_pubrec_resp_t pubrec_resp;
		enum mqtt_client_err_t rc = decode_pubrec_resp(mqtt_data, &header, &pubrec_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		struct mqtt_req_t pubrec_req = { .packet_type=MQTT_PUBREC_PACKET, .packet_id=pubrec_resp.packet_id };
		bool found = mqtt_req_queue_update(&mqtt_client->req_queue, &pubrec_req);
		if (!found)
			return MQTT_ERROR_PARSING_MSG;

		encode_mqtt_pubrel_msg(mqtt_client->pcb, &pubrec_resp.packet_id);

		TCPHandler_output(mqtt_client->pcb);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

		struct mqtt_req_t pubcomp_req = { .packet_type=MQTT_PUBCOMP_PACKET, .packet_id=pubrec_resp.packet_id };
		mqtt_req_queue_update(&mqtt_client->req_queue, &pubcomp_req);

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - PUBREC_RESP_LEN;
		return MQTT_SUCCESS;
	}
	case MQTT_PUBCOMP_PACKET:
	{
		struct mqtt_pubcomp_resp_t pubcomp_resp;
		enum mqtt_client_err_t rc = decode_pubcomp_resp(mqtt_data, &header, &pubcomp_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		struct mqtt_req_t pubcomp_req = { .packet_type=MQTT_PUBCOMP_PACKET, .packet_id=pubcomp_resp.packet_id };
		bool found = mqtt_req_queue_update(&mqtt_client->req_queue, &pubcomp_req);
		if (!found)
			return MQTT_ERROR_PARSING_MSG;

		mqtt_req_queue_remove(&mqtt_client->req_queue);

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - PUBCOMP_RESP_LEN;
		return MQTT_SUCCESS;
	}
	case MQTT_SUBACK_PACKET:
	{
		struct mqtt_suback_resp_t suback_resp;
		enum mqtt_client_err_t rc = decode_suback_resp(mqtt_data, &header, &suback_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		struct mqtt_req_t suback_req = { .packet_type=MQTT_SUBACK_PACKET, .packet_id=suback_resp.packet_id };
		bool found = mqtt_req_queue_update(&mqtt_client->req_queue, &suback_req);
		if (!found)
			return MQTT_ERROR_PARSING_MSG;

		mqtt_req_queue_remove(&mqtt_client->req_queue);

		mqtt_client->on_sub_completed_cb(&suback_resp);

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - SUBACK_RESP_LEN;
		return MQTT_SUCCESS;
	}
	case MQTT_PUBLISH_PACKET:
	{
		struct mqtt_publish_resp_t publish_resp;
		enum mqtt_client_err_t rc = decode_publish_resp(mqtt_data, &header, &publish_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		mqtt_client->on_msg_received_cb(&publish_resp);

		if (publish_resp.qos == 1)
		{
			encode_mqtt_puback_msg(mqtt_client->pcb, &publish_resp.packet_id);

			TCPHandler_output(mqtt_client->pcb);
			mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
		}
		else if (publish_resp.qos == 2)
		{
			encode_mqtt_pubrec_msg(mqtt_client->pcb, &publish_resp.packet_id);

			TCPHandler_output(mqtt_client->pcb);
			mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

			struct mqtt_req_t pubrel_req = { .packet_type=MQTT_PUBREL_PACKET, .packet_id=publish_resp.packet_id };
			mqtt_req_queue_add(&mqtt_client->req_queue, &pubrel_req);
		}

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - header.remaining_len;
		return MQTT_SUCCESS;
	}
	case MQTT_PUBREL_PACKET:
	{
		struct mqtt_pubrel_resp_t pubrel_resp;
		enum mqtt_client_err_t rc = decode_pubrel_resp(mqtt_data, &header, &pubrel_resp);
		if (rc != MQTT_SUCCESS)
			return MQTT_ERROR_PARSING_MSG;

		struct mqtt_req_t pubrel_req = { .packet_type=MQTT_PUBREL_PACKET, .packet_id=pubrel_resp.packet_id };
		bool found = mqtt_req_queue_update(&mqtt_client->req_queue, &pubrel_req);
		if (!found)
			return MQTT_ERROR_PARSING_MSG;

		encode_mqtt_pubcomp_msg(mqtt_client->pcb, &pubrel_resp.packet_id);

		TCPHandler_output(mqtt_client->pcb);
		mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

		mqtt_req_queue_remove(&mqtt_client->req_queue);

		*bytes_left = (tot_len - 1 - header.digits_remaining_len) - PUBREL_RESP_LEN;
		return MQTT_SUCCESS;
	}
	default:
	{
		*bytes_left = 0;
		return MQTT_SUCCESS;
	}
	}
}
