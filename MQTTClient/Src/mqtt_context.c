#include "mqtt_context.h"
#include <string.h>


void save_mqtt_pub_context(union mqtt_context_t* context, struct mqtt_pub_msg_t* pub_msg)
{
	context->pub.topic_len = strlen(pub_msg->topic);
	memcpy(context->pub.topic, pub_msg->topic, context->pub.topic_len);
	context->pub.topic[context->pub.topic_len] = '\0';

	context->pub.payload_len = strlen(pub_msg->payload);
	memcpy(context->pub.payload, pub_msg->payload, context->pub.payload_len);
	context->pub.payload[context->pub.payload_len] = '\0';

	context->pub.qos = pub_msg->qos;
	context->pub.retain = pub_msg->retain;
}

void save_mqtt_sub_context(union mqtt_context_t* context, struct mqtt_sub_msg_t* sub_msg)
{
	context->sub.topic_len = strlen(sub_msg->topic);
	memcpy(context->sub.topic, sub_msg->topic, context->sub.topic_len);
	context->sub.topic[context->sub.topic_len] = '\0';

	context->sub.qos = sub_msg->qos;
}

void save_mqtt_unsub_context(union mqtt_context_t* context, struct mqtt_unsub_msg_t* unsub_msg)
{
	context->unsub.topic_len = strlen(unsub_msg->topic);
	memcpy(context->unsub.topic, unsub_msg->topic, context->unsub.topic_len);
	context->unsub.topic[context->unsub.topic_len] = '\0';
}

void create_mqtt_context_from_pub_response(union mqtt_context_t* context, struct mqtt_publish_resp_t* publish_resp)
{
	context->pub.topic_len = publish_resp->topic_len;
	memcpy(context->pub.topic, publish_resp->topic, context->pub.topic_len);
	context->pub.topic[context->pub.topic_len] = '\0';

	context->pub.payload_len = publish_resp->data_len;
	memcpy(context->pub.payload, publish_resp->data, context->pub.payload_len);
	context->pub.payload[context->pub.payload_len] = '\0';

	context->pub.qos = publish_resp->qos;
}
