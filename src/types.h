#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

static constexpr i8  i8_m = INT8_MAX;
static constexpr i16 i16_m = INT16_MAX;
static constexpr i32 i32_m = INT32_MAX;
static constexpr i64 i64_m = INT64_MAX;
static constexpr u8  u8_m = UINT8_MAX;
static constexpr u16 u16_m = UINT16_MAX;
static constexpr u32 u32_m = UINT32_MAX;
static constexpr u64 u64_m = UINT64_MAX;

typedef float f32;
typedef double f64;

typedef signed char schar;
typedef unsigned char uchar;

typedef bool b8;

typedef int_least8_t    li8;
typedef int_least16_t  li16;
typedef int_least32_t  li32;
typedef int_least64_t  li64;
typedef uint_least8_t   lu8;
typedef uint_least16_t lu16;
typedef uint_least32_t lu32;
typedef uint_least64_t lu64;

typedef int_fast8_t    fi8;
typedef int_fast16_t  fi16;
typedef int_fast32_t  fi32;
typedef int_fast64_t  fi64;
typedef uint_fast8_t   fu8;
typedef uint_fast16_t fu16;
typedef uint_fast32_t fu32;
typedef uint_fast64_t fu64;

#define typeof(type) std::remove_pointer_t<decltype(type)>

#endif