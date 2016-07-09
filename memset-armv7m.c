#include <stdint.h>
#include <stddef.h>

void * memset(void *dst, int val, size_t count)
{
	uint8_t i = 0;
	while (i < count)
	{
		*((uint8_t *) dst++) = val;
		++i;
	}

	return dst;
}
