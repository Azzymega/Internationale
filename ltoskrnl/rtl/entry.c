#include <ltbase.h>
#include <hal/hal.h>
#include <rtl/rtl.h>

VOID RtlCopyMemory(VOID *destination, VOID *source, UINTPTR length)
{
    HalCopyMemory(destination, source, length);
}

VOID RtlMoveMemory(VOID *destination, VOID *source, UINTPTR length)
{
    HalMoveMemory(destination,source,length);
}

VOID RtlSetMemory(VOID *destination, UINTPTR target, UINTPTR length)
{
    HalSetMemory(destination, target, length);
}

VOID RtlZeroMemory(VOID* destination, UINTPTR legnth)
{
    RtlSetMemory(destination,0,legnth);
}

BOOLEAN RtlHasFlag(INTPTR enumValue, UINTPTR flag)
{
    if ((enumValue & flag) == flag)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID RtlSetBit(UINTPTR* pointer, UINTPTR n)
{
    *pointer |= (1 << n);
}

BOOLEAN RtlCheckBit(UINTPTR pointer, UINTPTR n)
{
    return (pointer >> n) & 1;
}

VOID RtlClearBit(UINTPTR* pointer, UINTPTR n)
{
    *pointer &= ~(1 << n);
}

void * memset(void *dest, int value, size_t length)
{
    RtlSetMemory(dest,value,length);
    return dest;
}

size_t strlen(const char *string)
{
    size_t Length = 0;
    while (string[Length])
    {
        Length++;
    }
    return Length;
}

int memcmp(const void *aptr, const void *bptr, const size_t size)
{
    const UINT32 aligned = size / 4;
    const UINT32 out = size % 4;

    for (size_t i = 0; i < aligned; ++i)
    {
        const UINT32 *FirstValue = aptr;
        const UINT32 *SecondValue = bptr;

        if (FirstValue[i] < SecondValue[i])
        {
            return -1;
        }
        else if (FirstValue[i] > SecondValue[i])
        {
            return 1;
        }
    }

    const unsigned char *a = (unsigned char *) aptr + (aligned * 4);
    const unsigned char *b = (unsigned char *) bptr + (aligned * 4);

    for (size_t i = 0; i < out; i++)
    {
        if (a[i] < b[i])
        {
            return -1;
        }
        else if (b[i] < a[i])
        {
            return 1;
        }
        else
        {
            continue;
        }
    }

    return 0;
}

void* memcpy(void* dest, void* src, size_t n)
{
    HalCopyMemory(dest,src,n);
    return dest;
}

char * itoa(int value, char *str, int base)
{
    char *rc;
    char *ptr;
    char *low;
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
    }
    low = ptr;
    do
    {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrst"
                "uvwxyz"[35 + value % base];
        value /= base;
    }
    while (value);
    *ptr-- = '\0';
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}
