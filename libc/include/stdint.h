#ifndef _STDINT_H
#define _STDINT_H

// Signed integers
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

// Unsigned integers
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// Pointers and size types
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef int32_t intmax_t;
typedef uint32_t uintmax_t;

// size_t and ssize_t are defined in stddef.h

// Limits of exact-width integer types
#define INT8_MIN   (-128)
#define INT16_MIN  (-32768)
#define INT32_MIN  (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX   127
#define INT16_MAX  32767
#define INT32_MAX  2147483647
#define INT64_MAX  9223372036854775807LL

#define UINT8_MAX  255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615ULL

// Limits of pointer-holding signed integer types
#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX

// Limits of greatest-width integer types
#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

// Limits of other integer types
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

#define SIZE_MAX    UINT32_MAX

// Integer constant macros
#define INT8_C(x)  ((int8_t)(x))
#define INT16_C(x) (x)
#define INT32_C(x) (x)
#define INT64_C(x) (x ## LL)

#define UINT8_C(x)  ((uint8_t)(x))
#define UINT16_C(x) (x)
#define UINT32_C(x) (x ## U)
#define UINT64_C(x) (x ## ULL)

#define INTMAX_C(x)  INT64_C(x)
#define UINTMAX_C(x) UINT64_C(x)

#endif // _STDINT_H
