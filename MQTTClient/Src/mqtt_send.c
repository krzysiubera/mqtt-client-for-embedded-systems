#include "mqtt_send.h"
#include "mqtt_client.h"
#include "tcp_connection_raw.h"
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

void send_utf8_encoded_str(struct tcp_pcb* pcb, uint8_t* msg, uint16_t len)
{
	uint8_t str_len_encoded[2];
	str_len_encoded[0] = (len >> 8) & 0xFF;
	str_len_encoded[1] = (len & 0xFF);
	TCPHandler_write(pcb, str_len_encoded, 2);
	TCPHandler_write(pcb, msg, len);
}

void send_u16(struct tcp_pcb* pcb, uint16_t* val)
{
	uint16_t len = *val;
	uint8_t u16_as_bytes[2] = {(len >> 8) & 0xFF, (len & 0xFF)};
	TCPHandler_write(pcb, u16_as_bytes, 2);
}

void send_u8(struct tcp_pcb* pcb, uint8_t* val)
{
	TCPHandler_write(pcb, val, 1);
}

void send_fixed_header(struct tcp_pcb* pcb, uint8_t ctrl_field, uint16_t remaining_len)
{
	uint8_t remaining_len_encoded[4];
	memset(remaining_len_encoded, 0, 4);
	uint8_t size_remaining_len_field = encode_remaining_len(remaining_len_encoded, remaining_len);

	TCPHandler_write(pcb, &ctrl_field, 1);
	TCPHandler_write(pcb, remaining_len_encoded, size_remaining_len_field);
}

void send_buffer(struct tcp_pcb* pcb, uint8_t* buffer, uint16_t len)
{
	TCPHandler_write(pcb, buffer, len);
}
