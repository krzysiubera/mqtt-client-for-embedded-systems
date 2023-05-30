#include "mqtt_decode.h"
#include <string.h>

static uint32_t get_remaining_len(uint8_t* mqtt_data)
{
	uint32_t multiplier = 1;
	uint32_t remaining_len = 0;
	uint8_t encoded_byte;
	uint8_t pos = 1;
	do
	{
		encoded_byte = mqtt_data[pos];
		remaining_len += (encoded_byte & 127) * multiplier;
		multiplier *= 128;
		pos++;
	} while ((encoded_byte & 128) != 0);
	return remaining_len;
}

static uint8_t get_digits_remaining_len(uint32_t remaining_len)
{
	if (remaining_len < 128)
		return 1;
	else if (remaining_len < 16384)
		return 2;
	else if (remaining_len < 2097151)
		return 3;
	else if (remaining_len < 268435455)
		return 4;
	else
		return 0;
}

struct mqtt_header_t decode_mqtt_header(uint8_t* mqtt_data)
{
	static struct mqtt_header_t header;
	header.packet_type = mqtt_data[0] & 0xF0;
	header.flags = mqtt_data[0] & 0x0F;
	header.remaining_len = get_remaining_len(mqtt_data);
	header.digits_remaining_len = get_digits_remaining_len(header.remaining_len);
	return header;
}
enum mqtt_client_err_t decode_connack_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_connack_resp_t* connack_resp)
{
	if ((header->remaining_len != CONNACK_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	connack_resp->session_present = mqtt_data[2] & 0x01;
	connack_resp->conn_rc = mqtt_data[3];
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_puback_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_puback_resp_t* puback_resp)
{
	if ((header->remaining_len != PUBACK_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	puback_resp->packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_pubrec_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubrec_resp_t* pubrec_resp)
{
	if ((header->remaining_len != PUBREC_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	pubrec_resp->packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_pubcomp_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubcomp_resp_t* pubcomp_resp)
{
	if ((header->remaining_len != PUBCOMP_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	pubcomp_resp->packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_suback_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_suback_resp_t* suback_resp)
{
	if ((header->remaining_len != SUBACK_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	suback_resp->packet_id = (mqtt_data[2] << 8) | (mqtt_data[3]);
	suback_resp->suback_rc = mqtt_data[4];
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_publish_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_publish_resp_t* publish_resp)
{
	publish_resp->qos = (header->flags & 0x06) >> 1;

	publish_resp->topic = mqtt_data + 1 + header->digits_remaining_len + 2;
	publish_resp->topic_len = ((mqtt_data[1 + header->digits_remaining_len] << 8)) | mqtt_data[1 + header->digits_remaining_len + 1];

	uint32_t pkt_id_len = (publish_resp->qos != 0) ? 2 : 0;
	publish_resp->packet_id = 0;
	if (pkt_id_len != 0)
		publish_resp->packet_id = (mqtt_data[1 + header->digits_remaining_len + 2 + publish_resp->topic_len] << 8) | (mqtt_data[1 + header->digits_remaining_len + 2 + publish_resp->topic_len + 1]);

	publish_resp->data = mqtt_data + 1 + header->digits_remaining_len + 2 + publish_resp->topic_len + pkt_id_len;
	publish_resp->data_len = (header->remaining_len - 2 - publish_resp->topic_len - pkt_id_len);

	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_pubrel_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_pubrel_resp_t* pubrel_resp)
{
	if ((header->remaining_len != PUBREL_RESP_LEN) || (header->flags != 2))
		return MQTT_INVALID_MSG;

	pubrel_resp->packet_id = (mqtt_data[2] << 8) | (mqtt_data[3]);
	return MQTT_SUCCESS;
}

enum mqtt_client_err_t decode_unsuback_resp(uint8_t* mqtt_data, struct mqtt_header_t* header, struct mqtt_unsuback_resp_t* unsuback_resp)
{
	if ((header->remaining_len != UNSUBACK_RESP_LEN) || (header->flags != 0))
		return MQTT_INVALID_MSG;

	unsuback_resp->packet_id = (mqtt_data[2] << 8) | (mqtt_data[3]);
	return MQTT_SUCCESS;
}
