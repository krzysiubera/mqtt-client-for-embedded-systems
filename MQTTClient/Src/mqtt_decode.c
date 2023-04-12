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
	header.remaining_len = get_remaining_len(mqtt_data);
	header.digits_remaining_len = get_digits_remaining_len(header.remaining_len);
	return header;
}

struct mqtt_connack_msg_t decode_connack_msg(uint8_t* mqtt_data, struct mqtt_header_t connack_header)
{
	static struct mqtt_connack_msg_t connack_msg;
	memset(&connack_msg, 0, sizeof(connack_msg));

	if (connack_header.remaining_len != 2)
		return connack_msg;

	connack_msg.conn_rc = mqtt_data[3];
	return connack_msg;
}

struct mqtt_puback_msg_t decode_puback_msg(uint8_t* mqtt_data, struct mqtt_header_t puback_header)
{
	static struct mqtt_puback_msg_t puback_msg;
	memset(&puback_msg, 0, sizeof(puback_msg));

	if (puback_header.remaining_len != 2)
		return puback_msg;

	puback_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return puback_msg;
}

struct mqtt_pubrec_msg_t decode_pubrec_msg(uint8_t* mqtt_data, struct mqtt_header_t pubrec_header)
{
	static struct mqtt_pubrec_msg_t pubrec_msg;
	memset(&pubrec_msg, 0, sizeof(pubrec_msg));

	if (pubrec_header.remaining_len != 2)
		return pubrec_msg;

	pubrec_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return pubrec_msg;
}

struct mqtt_pubcomp_msg_t decode_pubcomp_msg(uint8_t* mqtt_data, struct mqtt_header_t pubcomp_header)
{
	static struct mqtt_pubcomp_msg_t pubcomp_msg;
	memset(&pubcomp_msg, 0, sizeof(pubcomp_msg));

	if (pubcomp_header.remaining_len != 2)
		return pubcomp_msg;

	pubcomp_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
	return pubcomp_msg;
}

struct mqtt_suback_msg_t decode_suback_msg(uint8_t* mqtt_data, struct mqtt_header_t suback_header)
{
	static struct mqtt_suback_msg_t suback_msg;
	memset(&suback_msg, 0, sizeof(suback_msg));

	if (suback_header.remaining_len != 3)
		return suback_msg;

	suback_msg.packet_id = (mqtt_data[2] << 8) | (mqtt_data[3]);
	suback_msg.suback_rc = mqtt_data[4];
	return suback_msg;
}
