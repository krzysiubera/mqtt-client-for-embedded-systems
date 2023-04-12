#include "mqtt_cb_info.h"
#include <string.h>

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb)
{
	cb_info->mqtt_payload = NULL;
	cb_info->mqtt_msg_len = 0;
	cb_info->msg_received_cb = msg_received_cb;
}
