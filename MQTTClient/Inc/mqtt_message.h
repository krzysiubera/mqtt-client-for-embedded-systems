#ifndef MQTT_MESSAGE_H
#define MQTT_MESSAGE_H

struct mqtt_pub_msg_t
{
	char* topic;
	char* payload;
	uint8_t qos;
	bool retain;
};

struct mqtt_sub_msg_t
{
	char* topic;
	uint8_t qos;
};

#endif
