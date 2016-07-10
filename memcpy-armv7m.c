#include <stdint.h>
#include <stddef.h>

void *memcpy(void *s1, const void *s2, size_t n)
{
	uint32_t i = 0;

	while (i < n)
	{
		*((uint8_t *) s1++) = *((uint8_t *) s2 + i++);
	}

	return s1;
}
