#include "mqtt_receive.h"
#include "mqtt_decode.h"
#include "mqtt_encode.h"
#include "tcp_connection_raw.h"

uint32_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_client_t* mqtt_client)
{
	struct mqtt_header_t header = decode_mqtt_header(mqtt_data);

	switch (header.packet_type)
	{
	case MQTT_CONNACK_PACKET:
	{
		struct mqtt_connack_resp_t connack_resp;
		enum mqtt_client_err_t rc = decode_connack_resp(mqtt_data, &header, &connack_resp);
		if (rc == MQTT_SUCCESS)
		{
			mqtt_client->connack_resp = connack_resp;
			mqtt_client->connack_resp_available = true;
		}
		return (tot_len - 1 - header.digits_remaining_len) - CONNACK_RESP_LEN;
	}
	case MQTT_PUBACK_PACKET:
	{
		struct mqtt_puback_resp_t puback_resp;
		enum mqtt_client_err_t rc = decode_puback_resp(mqtt_data, &header, &puback_resp);
		if (rc == MQTT_SUCCESS)
		{
			// check if PUBACK req is in queue
		}
		return (tot_len - 1 - header.digits_remaining_len) - PUBACK_RESP_LEN;
	}
	case MQTT_PUBREC_PACKET:
	{
		struct mqtt_pubrec_resp_t pubrec_resp;
		enum mqtt_client_err_t rc = decode_pubrec_resp(mqtt_data, &header, &pubrec_resp);
		if (rc == MQTT_SUCCESS)
		{
			// check if PUBREC req is in queue - if it is, send pubrel with the same packet id
			encode_mqtt_pubrel_msg(mqtt_client->pcb, &pubrec_resp.packet_id);

			TCPHandler_output(mqtt_client->pcb);
			mqtt_client->last_activity = mqtt_client->elapsed_time_cb();

			// put PUBCOMP request to queue
		}
		return (tot_len - 1 - header.digits_remaining_len) - PUBREC_RESP_LEN;
	}
	case MQTT_PUBCOMP_PACKET:
	{
		struct mqtt_pubcomp_resp_t pubcomp_resp;
		enum mqtt_client_err_t rc = decode_pubcomp_resp(mqtt_data, &header, &pubcomp_resp);
		if (rc == MQTT_SUCCESS)
		{
			// check if PUBCOMP request is in queue
		}
		return (tot_len - 1 - header.digits_remaining_len) - PUBCOMP_RESP_LEN;
	}
	case MQTT_SUBACK_PACKET:
	{
		struct mqtt_suback_resp_t suback_resp;
		enum mqtt_client_err_t rc = decode_suback_resp(mqtt_data, &header, &suback_resp);
		if (rc == MQTT_SUCCESS)
		{
			// check if SUBACK request in queue
		}
		return (tot_len - 1 - header.digits_remaining_len) - SUBACK_RESP_LEN;
	}
	case MQTT_PUBLISH_PACKET:
	{
		struct mqtt_publish_resp_t publish_resp;
		enum mqtt_client_err_t rc = decode_publish_resp(mqtt_data, &header, &publish_resp);
		if (rc == MQTT_SUCCESS)
		{
			mqtt_client->msg_received_cb(&publish_resp);
			if (publish_resp.qos == 0)
			{
				// do nothing
			}
			else if (publish_resp.qos == 1)
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

				// put to queue request for PUBREL
			}
		}
		// bytes left in buf
		return (tot_len - 1 - header.digits_remaining_len) - header.remaining_len;
	}
	case MQTT_PUBREL_PACKET:
	{
		struct mqtt_pubrel_resp_t pubrel_resp;
		enum mqtt_client_err_t rc = decode_pubrel_resp(mqtt_data, &header, &pubrel_resp);
		if (rc == MQTT_SUCCESS)
		{
			// check if PUBREL msg in queue - if it is, send PUBCOMP with the same message ID
			encode_mqtt_pubcomp_msg(mqtt_client->pcb, &pubrel_resp.packet_id);

			TCPHandler_output(mqtt_client->pcb);
			mqtt_client->last_activity = mqtt_client->elapsed_time_cb();
		}
		return (tot_len - 1 - header.digits_remaining_len) - PUBREL_RESP_LEN;
	}

	default:
		return 0;
	}
}
