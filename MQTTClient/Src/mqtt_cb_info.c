#include "mqtt_cb_info.h"
#include <string.h>

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb)
{
	memset(&cb_info->connack_msg, 0, sizeof(cb_info->connack_msg));
	cb_info->connack_msg_available = false;


	cb_info->suback_received = false;
	cb_info->puback_received = false;
	cb_info->pubrec_received = false;
	cb_info->pubcomp_received = false;
	cb_info->last_packet_id = 0;
	cb_info->last_qos_subscribed = 0;
	cb_info->msg_received_cb = msg_received_cb;
}
