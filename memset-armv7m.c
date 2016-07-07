#include <stdint.h>
#include <stddef.h>

void memset(uint8_t *dst, int val, size_t count)
{
	uint8_t i = 0;
	while (i < count)
	{
		*dst++ = val;
		++i;
	}
}
