#include "mqtt_cb_info.h"
#include <string.h>

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb, elapsed_time_cb_t elapsed_time_cb)
{
	memset(&cb_info->connack_resp, 0, sizeof(cb_info->connack_resp));
	cb_info->connack_resp_available = false;
	cb_info->msg_received_cb = msg_received_cb;
	cb_info->last_activity = 0;
	cb_info->elapsed_time_cb = elapsed_time_cb;
}
