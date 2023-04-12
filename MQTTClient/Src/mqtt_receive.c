#include "mqtt_receive.h"
#include "mqtt_decode.h"

uint32_t get_mqtt_packet(uint8_t* mqtt_data, uint16_t tot_len, struct mqtt_cb_info_t* cb_info)
{
	struct mqtt_header_t header = decode_mqtt_header(mqtt_data);

	switch (header.packet_type)
	{
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
	default:
		break;
	}
	return 0;
}
