#pragma once
#include <ltbase.h>

VOID RtlCopyMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlMoveMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlSetMemory(VOID *destination, UINTPTR target, UINTPTR length);
VOID RtlZeroMemory(VOID *destination, UINTPTR legnth);
BOOLEAN RtlHasFlag(INTPTR enumValue, UINTPTR flag);

VOID RtlSetBit(UINTPTR* pointer, UINTPTR n);
BOOLEAN RtlCheckBit(UINTPTR pointer, UINTPTR n);
VOID RtlClearBit(UINTPTR* pointer, UINTPTR n);

void* memset(void* dest, int value, size_t length);
size_t strlen(const char *string);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, void *src, size_t n);
char *itoa(int value, char *str, int base);
