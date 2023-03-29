#include "mqtt_deserialize.h"
#include "mqtt_packets.h"
#include "mqtt_rc.h"

struct remaining_len_info_t
{
	uint32_t num_digits;
	uint32_t remaining_len;
};


static struct remaining_len_info_t get_digits_remaining_length(uint8_t* mqtt_data)
{
	static struct remaining_len_info_t rem_len_info;

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
	pos--;

	rem_len_info.num_digits = pos;
	rem_len_info.remaining_len = remaining_len;

	return rem_len_info;
}


void deserialize_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info)
{
	enum mqtt_packet_type_t pkt_type = mqtt_data[0] & 0xF0;

	struct remaining_len_info_t rem_len_info = get_digits_remaining_length(mqtt_data);
	uint32_t digits_remaining_len = rem_len_info.num_digits;
	uint32_t remaining_len = rem_len_info.remaining_len;

	switch (pkt_type)
	{
	case MQTT_CONNACK_PACKET:
	{
		enum mqtt_connection_rc_t conn_rc = mqtt_data[1 + digits_remaining_len + 1];
		if (conn_rc == MQTT_CONNECTION_ACCEPTED)
		{
			cb_info->mqtt_connected = true;
		}
		break;
	}
	case MQTT_SUBACK_PACKET:
	{
		enum mqtt_suback_rc_t suback_rc = mqtt_data[1 + digits_remaining_len + 2];
		if (suback_rc == cb_info->last_qos_subscribed)
		{
			cb_info->suback_received = true;
		}
		break;
	}
	case MQTT_PUBLISH_PACKET:
	{
		uint8_t qos = (mqtt_data[0] & 0x06) >> 1;
		uint8_t* topic = mqtt_data + 1 + digits_remaining_len + 2;
		uint32_t topic_len = ((mqtt_data[1 + digits_remaining_len] << 8)) | mqtt_data[1 + digits_remaining_len + 1];
		uint32_t pkt_id_len = (qos != 0) ? 2 : 0;
		uint8_t* data = mqtt_data + 1 + digits_remaining_len + 2 + topic_len + pkt_id_len;


		uint32_t data_len_expected = (remaining_len - 1 - digits_remaining_len - topic_len - pkt_id_len);
		uint32_t data_len_in_buffer = tot_len - (1 + digits_remaining_len + 2 + topic_len + pkt_id_len);

		if (data_len_expected == data_len_in_buffer)
			cb_info->msg_received_cb(topic, topic_len, data, data_len_expected, qos);

		break;
	}
	case MQTT_PUBACK_PACKET:
	{
		if (mqtt_data[1] == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 3) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->puback_received = true;
		}
		break;
	}
	case MQTT_PUBREC_PACKET:
	{
		if (mqtt_data[1] == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 3) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->pubrec_received = true;
		}
		break;
	}
	case MQTT_PUBCOMP_PACKET:
	{
		if (mqtt_data[1] == 2)
		{
			uint16_t received_packet_id = (mqtt_data[2] << 3) + mqtt_data[3];
			if (received_packet_id == cb_info->last_packet_id)
				cb_info->pubcomp_received = true;
		}
		break;
	}

	default:
		break;
	}
}
