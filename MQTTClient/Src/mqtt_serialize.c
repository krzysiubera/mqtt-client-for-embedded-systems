#include "mqtt_serialize.h"
#include <string.h>

static uint8_t encode_remaining_len(uint8_t* buf, uint32_t remaining_len)
{
	uint8_t num_encoded_chars = 0;
	do
	{
		uint8_t d = remaining_len % 128;
		remaining_len /= 128;
		if (remaining_len > 0)
			d |= 0x80;
		buf[num_encoded_chars++] = d;
	} while (remaining_len > 0);
	return num_encoded_chars;
}

void serialize_utf8_encoded_str(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* msg, uint16_t len)
{
	uint8_t str_len_encoded[2];
	str_len_encoded[0] = (len >> 8) & 0xFF;
	str_len_encoded[1] = (len & 0xFF);
	TCPConnectionRaw_write(tcp_connection_raw, str_len_encoded, 2);
	TCPConnectionRaw_write(tcp_connection_raw, msg, len);
}

void serialize_u16(struct tcp_connection_raw_t* tcp_connection_raw, uint16_t* val)
{
	uint16_t len = *val;
	uint8_t u16_as_bytes[2] = {(len >> 8) & 0xFF, (len & 0xFF)};
	TCPConnectionRaw_write(tcp_connection_raw, u16_as_bytes, 2);
}

void serialize_u8(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* val)
{
	TCPConnectionRaw_write(tcp_connection_raw, val, 1);
}

void serialize_fixed_header(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t ctrl_field, uint32_t remaining_len)
{
	uint8_t remaining_len_encoded[4];
	memset(remaining_len_encoded, 0, 4);
	uint8_t size_remaining_len_field = encode_remaining_len(remaining_len_encoded, remaining_len);

	TCPConnectionRaw_write(tcp_connection_raw, &ctrl_field, 1);
	TCPConnectionRaw_write(tcp_connection_raw, remaining_len_encoded, size_remaining_len_field);
}

void serialize_buffer(struct tcp_connection_raw_t* tcp_connection_raw, uint8_t* buffer, uint16_t len)
{
	TCPConnectionRaw_write(tcp_connection_raw, buffer, len);
}
