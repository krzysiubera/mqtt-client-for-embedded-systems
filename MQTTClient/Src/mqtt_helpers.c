#include "mqtt_helpers.h"

uint8_t get_digits_remaining_len(uint32_t remaining_len)
{
	if (remaining_len < 128)
		return 1;
	else if (remaining_len < 16384)
		return 2;
	else if (remaining_len < 2097151)
		return 3;
	else if (remaining_len < 268435455)
		return 4;
	else
		return 0;
}
