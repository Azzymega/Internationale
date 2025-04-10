#pragma once
#include <ltbase.h>

VOID RtlCopyMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlMoveMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlSetMemory(VOID *destination, UINTPTR target, UINTPTR length);

void* memset(void* dest, int value, size_t length);
size_t strlen(const char *string);
int memcmp(const void *s1, const void *s2, size_t n);
char *itoa(int value, char *str, int base);
