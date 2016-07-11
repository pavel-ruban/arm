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

int strncmp(const char *str1, const char *str2, size_t len)
{
    int s1;
    int s2;
    size_t i = 0;

    do {
        s1 = *str1++;
        s2 = *str2++;
	++i;
        if (s1 == 0)
            break;
    } while (s1 == s2 && i < len);
    return (s1 < s2) ? -1 : (s1 > s2);
}
