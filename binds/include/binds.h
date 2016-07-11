#pragma once
#pragma pack(1)

#include <stdint.h>

#include "spi_binds.h"
#include "rc522_binds.h"
#include "ethernet_binds.h"

extern volatile uint32_t ticks;

void *memcpy(void *s1, const void *s2, size_t n);
void *memset(void *dst, int val, size_t count);
size_t strlen(const char* str);
int strncmp(const char *str1, const char *str2, size_t len);

void __enable_enc28j60_irq();
void __disable_enc28j60_irq();

void Delay(vu32 nCount);
