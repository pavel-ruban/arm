#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str)
{
   uint16_t i = 0;
   while(str[i++] != 0);

   return i;
}

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
