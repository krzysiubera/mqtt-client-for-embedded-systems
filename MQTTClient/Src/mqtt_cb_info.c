#include "mqtt_cb_info.h"

void MQTTCbInfo_init(struct mqtt_cb_info_t* cb_info, msg_received_cb_t msg_received_cb)
{
	cb_info->mqtt_connected = false;
	cb_info->last_subscribe_success = false;
	cb_info->msg_received_cb = msg_received_cb;
}
