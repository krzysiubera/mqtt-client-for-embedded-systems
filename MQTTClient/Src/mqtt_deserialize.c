#include "mqtt_deserialize.h"
#include "mqtt_packets.h"
#include "mqtt_rc.h"

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


uint32_t deserialize_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info)
{
	struct mqtt_header_t header;
	header.packet_type = mqtt_data[0] & 0xF0;
	header.remaining_len = get_remaining_len(mqtt_data);
	header.digits_remaining_len = get_digits_remaining_len(header.remaining_len);

	switch (header.packet_type)
	{
	case MQTT_CONNACK_PACKET:
	{
		if (header.remaining_len == 2)
		{
			struct mqtt_connack_msg_t connack_msg;
			connack_msg.conn_rc = mqtt_data[3];
			cb_info->connack_msg = connack_msg;
			cb_info->connack_msg_available = true;
		}
		break;
	}
	case MQTT_SUBACK_PACKET:
	{
		if (header.remaining_len == 3)
		{
			struct mqtt_suback_msg_t suback_msg;
			suback_msg.packet_id = (mqtt_data[2] << 8) | (mqtt_data[3]);
			suback_msg.suback_rc = mqtt_data[4];
			cb_info->suback_msg = suback_msg;
			cb_info->suback_msg_available = true;
		}
		break;
	}
	case MQTT_PUBLISH_PACKET:
	{
		uint8_t qos = (mqtt_data[0] & 0x06) >> 1;
		uint8_t* topic = mqtt_data + 1 + header.digits_remaining_len + 2;
		uint32_t topic_len = ((mqtt_data[1 + header.digits_remaining_len] << 8)) | mqtt_data[1 + header.digits_remaining_len + 1];
		uint32_t pkt_id_len = (qos != 0) ? 2 : 0;
		uint8_t* payload = mqtt_data + 1 + header.digits_remaining_len + 2 + topic_len + pkt_id_len;
		uint32_t payload_len = (header.remaining_len - 2 - topic_len - pkt_id_len);

		cb_info->msg_received_cb(topic, topic_len, payload, payload_len, qos);

		// bytes left in buf
		return (tot_len - 1 - header.digits_remaining_len) - header.remaining_len;

	}
	case MQTT_PUBACK_PACKET:
	{
		if (header.remaining_len == 2)
		{
			struct mqtt_puback_msg_t puback_msg;
			puback_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			cb_info->puback_msg = puback_msg;
			cb_info->puback_msg_available = true;
		}
		break;
	}
	case MQTT_PUBREC_PACKET:
	{
		if (header.remaining_len == 2)
		{
			struct mqtt_pubrec_msg_t pubrec_msg;
			pubrec_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			cb_info->pubrec_msg = pubrec_msg;
			cb_info->pubrec_msg_available = true;
		}
		break;
	}
	case MQTT_PUBCOMP_PACKET:
	{
		if (header.remaining_len == 2)
		{
			struct mqtt_pubcomp_msg_t pubcomp_msg;
			pubcomp_msg.packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			cb_info->pubcomp_msg = pubcomp_msg;
			cb_info->pubcomp_msg_available = true;
		}
		break;
	}

	default:
		break;
	}
	return 0;
}
