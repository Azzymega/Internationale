#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef _KERNEL_ASSERT
#define _KERNEL_ASSERT
#if __STDC_VERSION__ == 199901L
void HalAssert(const char *file, const char* func, const char *line);
#else
void _Kassert( char const * const );
#endif
#define __symbol2value( x ) #x
#define __symbol2string( x ) __symbol2value( x )
#endif

#undef assert

#ifdef NDEBUG
#define assert( ignore ) ( (void) 0 )
#else
#if __STDC_VERSION__ == 199901L
#define assert( expression ) ( ( expression ) ? (void) 0 \
: HalAssert( "Assertion failed: " #expression \
", function ", __func__, \
", file " __FILE__ \
", line " __symbol2string( __LINE__ ) \
"." ) )
#else
#define assert( expression ) ( ( expression ) ? (void) 0 \
: _Kassert( "Assertion failed: " #expression \
", file " __FILE__ \
", line " __symbol2string( __LINE__ ) \
"." ) )
#endif
#endif

#define INUSTATIC static
#define INUGLOBAL
#define INUCONST const
#define INUNATIVE
#define INUVOLATILE volatile
#define INUEXTENSION
#define IN
#define OUT
#define INOUT
#define INUSTUB
#define INUDEPRECATED
#define INUEXTERN extern
#define INUALIGN(count)  __attribute__ ((aligned (count)))
#define INUPACKED __attribute__((packed))
#define INUNAKED __attribute__((naked))
#define INUCALLEESAVEDREGS __attribute__((no_callee_saved_registers))
#define INUINTERRUPT __attribute__((interrupt))
#define INUFORCEINLINE __attribute__((always_inline))
#define INUINTERRUPTCALL __attribute__((no_caller_saved_registers))
#define INUIMPORT extern
#define INULIKELY(x)       __builtin_expect(!!(x), 1)
#define INUUNLIKELY(x)     __builtin_expect(!!(x), 0)
#define INUINLINE __attribute__((always_inline))
#define INUCPUCONTEXT


#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

typedef void VOID;
typedef uint8_t BOOLEAN;
typedef char CHAR;
typedef wchar_t WCHAR;

typedef uint8_t BYTE;
typedef int8_t SBYTE;

typedef float SINGLE;
typedef double DOUBLE;

typedef intptr_t INTPTR;
typedef int64_t INT64;
typedef int32_t INT32;
typedef int16_t INT16;

typedef uintptr_t UINTPTR;
typedef uint64_t UINT64;
typedef uint32_t UINT32;
typedef uint16_t UINT16;

typedef size_t SIZE;
typedef UINTPTR CONTROL_LEVEL;
typedef intptr_t INUSTATUS;
typedef VOID* REFERENCE;

enum INU_SYSTEM_STATUSES
{
    STATUS_SUCCESS,
    STATUS_FAIL,
    STATUS_UNIMPLEMENTED,
    STATUS_INSUFFICIENT_RESOURCES,
};

typedef void (*TRAP_HANDLER)(VOID *);

#define INU_SUCCESS(status) status == STATUS_SUCCESS