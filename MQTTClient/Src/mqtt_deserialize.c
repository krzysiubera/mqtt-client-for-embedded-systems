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
		struct mqtt_connack_msg_t connack_msg;
		connack_msg.conn_rc = mqtt_data[1 + header.digits_remaining_len + 1];
		cb_info->connack_msg = connack_msg;
		cb_info->connack_msg_available = true;
		break;
	}
	case MQTT_SUBACK_PACKET:
	{
		enum mqtt_suback_rc_t suback_rc = mqtt_data[1 + header.digits_remaining_len + 2];
		if (suback_rc == cb_info->last_qos_subscribed)
		{
			cb_info->suback_received = true;
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
		uint32_t payload_len = (header.remaining_len - 1 - header.digits_remaining_len - topic_len - pkt_id_len);

		cb_info->msg_received_cb(topic, topic_len, payload, payload_len, qos);

		// bytes left in buf
		return (tot_len - 2) - header.remaining_len;

	}
	case MQTT_PUBACK_PACKET:
	{
		if (header.remaining_len == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->puback_received = true;
		}
		break;
	}
	case MQTT_PUBREC_PACKET:
	{
		if (header.remaining_len == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->pubrec_received = true;
		}
		break;
	}
	case MQTT_PUBCOMP_PACKET:
	{
		if (header.remaining_len == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 8) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->pubcomp_received = true;
		}
		break;
	}

	default:
		break;
	}
	return 0;
}
