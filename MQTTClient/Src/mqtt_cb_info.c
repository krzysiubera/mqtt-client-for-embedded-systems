#include "mqtt_cb_info.h"
#include <string.h>

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb)
{
	memset(&cb_info->connack_msg, 0, sizeof(cb_info->connack_msg));
	cb_info->connack_msg_available = false;

	memset(&cb_info->suback_msg, 0, sizeof(cb_info->suback_msg));
	cb_info->suback_msg_available = false;

	memset(&cb_info->puback_msg, 0, sizeof(cb_info->puback_msg));
	cb_info->puback_msg_available = false;

	memset(&cb_info->pubrec_msg, 0, sizeof(cb_info->pubrec_msg));
	cb_info->pubrec_msg_available = false;

	memset(&cb_info->pubcomp_msg, 0, sizeof(cb_info->pubcomp_msg));
	cb_info->pubcomp_msg_available = false;

	cb_info->msg_received_cb = msg_received_cb;
}
