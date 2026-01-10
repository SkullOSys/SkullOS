#ifndef _STDDEF_H
#define _STDDEF_H

#include <stdint.h>

#define NULL ((void*)0)

typedef uint32_t size_t;
typedef int32_t ptrdiff_t;
typedef struct {
    long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
    long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;

typedef int32_t wchar_t;

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#endif // _STDDEF_H
