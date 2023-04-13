#include "mqtt_receive.h"
#include "mqtt_decode.h"
#include "mqtt_send.h"
#include "mqtt_client.h"

uint32_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info, struct tcp_pcb* active_pcb)
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
			cb_info->connack_resp = connack_resp;
			cb_info->connack_resp_available = true;
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
			uint8_t header[2] = {(MQTT_PUBREL_PACKET | 0x02), 2};
			uint8_t pkt_id_encoded[2] = {(pubrec_resp.packet_id >> 8) & 0xFF, (pubrec_resp.packet_id & 0xFF)};
			tcp_write(active_pcb, (void*) header, 2, 1);
			tcp_write(active_pcb, (void*) pkt_id_encoded, 2, 1);
			tcp_output(active_pcb);

			cb_info->last_activity = cb_info->elapsed_time_cb();

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
		uint8_t qos = (mqtt_data[0] & 0x06) >> 1;
		uint8_t* topic = mqtt_data + 1 + header.digits_remaining_len + 2;
		uint32_t topic_len = ((mqtt_data[1 + header.digits_remaining_len] << 8)) | mqtt_data[1 + header.digits_remaining_len + 1];
		uint32_t pkt_id_len = (qos != 0) ? 2 : 0;

		uint16_t pkt_id = 0;
		if (pkt_id_len != 0)
			pkt_id = (mqtt_data[1 + header.digits_remaining_len + 2 + topic_len] << 8) | (mqtt_data[1 + header.digits_remaining_len + 2 + topic_len + 1]);

		uint8_t* payload = mqtt_data + 1 + header.digits_remaining_len + 2 + topic_len + pkt_id_len;
		uint32_t payload_len = (header.remaining_len - 2 - topic_len - pkt_id_len);

		cb_info->msg_received_cb(topic, topic_len, payload, payload_len, qos);

		// bytes left in buf
		return (tot_len - 1 - header.digits_remaining_len) - header.remaining_len;

	}

	default:
		return 0;
	}
}
