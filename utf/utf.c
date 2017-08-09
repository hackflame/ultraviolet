// =============================================================================
// <utf.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/build.h>
#include <quantum/core.h>

#include <quantum/bswap.h>
#include <quantum/swar.h>
#include <quantum/simd.h>

#include "utf.h"

// =============================================================================
// Length
// -----------------------------------------------------------------------------
// Microtemplates
// -----------------------------------------------------------------------------

#define utf_length() do\
{                      \
  while (*s != '\0') s++;\
  return (size_t)(s - str);\
}       \
while (0)

#define utf_align(type, align) do\
{                                \
  const type* p = ptr_align_ceil (align, str);\
                                 \
  while (s != p)                 \
  {                              \
    if (unlikely (*s == '\0')) return (size_t)(s - str);\
    s++;\
  }     \
}       \
while (0)

// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

size_t utf8_stri_length (const u8* str)
{
  const u8* s = str;

#if CPU(SSE2)
  utf_align (u8, 16u);

  while (true)
  {
    xi128 x = _mm_load_si128 ((const xi128*)s);

    u32 m = _mm_movemask_epi8 (_mm_cmpeq_epi8 (x, _mm_setzero_si128()));

    if (unlikely (m != 0)) return (size_t)(s - str) + bsf32 (m);

    s += 16;
  }
#elif CPU(64BIT)
  utf_align (u8, 8u);

  while (true)
  {
    if (unlikely (swar_has_zero8 (*(const u64*)s))) break;
    s += 8;
  }

  utf_length();
#else
  utf_align (u8, 4u);

  while (true)
  {
    if (unlikely (swar_has_zero4 (*(const u32*)s))) break;
    s += 4;
  }

  utf_length();
#endif
}

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

size_t utf16_stri_length (const u16* str)
{
  const u16* s = str;

#if CPU(SSE2)
  utf_align (u16, 16u);

  while (true)
  {
    xi128 x = _mm_load_si128 ((const xi128*)s);

    u32 m = _mm_movemask_epi8 (_mm_cmpeq_epi16 (x, _mm_setzero_si128()));

    if (unlikely (m != 0)) return (size_t)(s - str) + (bsf32 (m) / 2u);

    s += 8;
  }
#elif CPU(64BIT)
  utf_align (u16, 8u);

  while (true)
  {
    u64 v = *(u64*)s;

    if (unlikely (((v - 0x0001000100010001u) & ~v & 0x8000800080008000u) != 0)) break;

    s += 4;
  }

  utf_length();
#else
  utf_align (u16, 4u);

  while (true)
  {
    u32 v = *(u32*)s;

    if (unlikely (((v - 0x00010001u) & ~v & 0x80008000u) != 0)) break;

    s += 2;
  }

  utf_length();
#endif
}

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

size_t utf32_stri_length (const u32* str)
{
  const u32* s = str;

#if CPU(SSE2)
  utf_align (u32, 16u);

  while (true)
  {
    xi128 x = _mm_load_si128 ((const xi128*)s);

    u32 m = _mm_movemask_epi8 (_mm_cmpeq_epi32 (x, _mm_setzero_si128()));

    if (unlikely (m != 0)) return (size_t)(s - str) + (bsf32 (m) / 4u);

    s += 4;
  }
#elif CPU(64BIT)
  utf_align (u32, 8u);

  while (true)
  {
    u64 v = *(u64*)s;

    if (unlikely (((v - 0x0000000100000001u) & ~v & 0x8000000080000000u) != 0)) break;

    s += 2;
  }

  utf_length();
#else
  utf_length();
#endif
}

// -----------------------------------------------------------------------------

#undef utf_length
#undef utf_align

// =============================================================================
// Number of codepoints
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

int utf8_str_valid (const u8* in, size_t len, u8** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf8.c"
}

int utf8_str_runes (const u8* in, size_t len, u8** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf8/utf8.c"
}

// -----------------------------------------------------------------------------

int utf8_stri_valid (const u8* in, u8** end, size_t* num)
{
  #define T_VALID

  #include "utf8/utf8.c"
}

int utf8_stri_runes (const u8* in, u8** end, size_t* num)
{
  #include "utf8/utf8.c"
}

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

int utf16_str_valid (const u16* in, size_t len, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf16.c"
}

int utf16_str_runes (const u16* in, size_t len, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf16.c"
}

// -----------------------------------------------------------------------------

int utf16_stri_valid (const u16* in, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf16.c"
}

int utf16_stri_runes (const u16* in, u16** end, size_t* num)
{
  #include "utf16/utf16.c"
}

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

int utf32_str_valid (const u32* in, size_t len, u32** end)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf32.c"
}

int utf32_stri_valid (const u32* in, u32** end)
{
  #define T_VALID

  #include "utf32/utf32.c"
}

// =============================================================================
// Conversion
// -----------------------------------------------------------------------------
// UTF-8 -> UTF-16
// -----------------------------------------------------------------------------

int utf8_str_to16 (const u8* restrict in, size_t len, u16* restrict out
, size_t size, u8** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf8to16.c"
}

int utf8_str_to16_fast (const u8* restrict in, size_t len, u16* restrict out
, size_t size, u8** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf8/utf8to16.c"
}

// -----------------------------------------------------------------------------

int utf8_stri_to16 (const u8* restrict in, u16* restrict out, size_t size
, u8** end, size_t* num)
{
  #define T_VALID

  #include "utf8/utf8to16.c"
}

int utf8_stri_to16_fast (const u8* restrict in, u16* restrict out, size_t size
, u8** end, size_t* num)
{
  #include "utf8/utf8to16.c"
}

// -----------------------------------------------------------------------------
// UTF-8 -> UTF-32
// -----------------------------------------------------------------------------

int utf8_str_to32 (const u8* restrict in, size_t len, u32* restrict out
, size_t size, u8** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf8to32.c"
}

int utf8_str_to32_fast (const u8* restrict in, size_t len, u32* restrict out
, size_t size, u8** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf8/utf8to32.c"
}

// -----------------------------------------------------------------------------

int utf8_stri_to32 (const u8* restrict in, u32* restrict out, size_t size
, u8** end, size_t* num)
{
  #define T_VALID

  #include "utf8/utf8to32.c"
}

int utf8_stri_to32_fast (const u8* restrict in, u32* restrict out, size_t size
, u8** end, size_t* num)
{
  #include "utf8/utf8to32.c"
}

// -----------------------------------------------------------------------------
// UTF-16 -> UTF-8
// -----------------------------------------------------------------------------

int utf16_str_to8 (const u16* restrict in, size_t len, u8* restrict out
, size_t size, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf16to8.c"
}

int utf16_str_to8_fast (const u16* restrict in, size_t len, u8* restrict out
, size_t size, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf16to8.c"
}

// -----------------------------------------------------------------------------

int utf16_stri_to8 (const u16* restrict in, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf16to8.c"
}

int utf16_stri_to8_fast (const u16* restrict in, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #include "utf16/utf16to8.c"
}

// -----------------------------------------------------------------------------
// UTF-16 -> UTF-32
// -----------------------------------------------------------------------------

int utf16_str_to32 (const u16* restrict in, size_t len, u32* restrict out
, size_t size, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf16to32.c"
}

int utf16_str_to32_fast (const u16* restrict in, size_t len, u32* restrict out
, size_t size, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf16to32.c"
}

// -----------------------------------------------------------------------------

int utf16_stri_to32 (const u16* restrict in, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf16to32.c"
}

int utf16_stri_to32_fast (const u16* restrict in, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #include "utf16/utf16to32.c"
}

// -----------------------------------------------------------------------------
// UTF-32 -> UTF-8
// -----------------------------------------------------------------------------

int utf32_str_to8 (const u32* restrict in, size_t len, u8* restrict out
, size_t size, u32** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf32to8.c"
}

int utf32_str_to8_fast (const u32* restrict in, size_t len, u8* restrict out
, size_t size, u32** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf32/utf32to8.c"
}

// -----------------------------------------------------------------------------

int utf32_stri_to8 (const u32* restrict in, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_VALID

  #include "utf32/utf32to8.c"
}

int utf32_stri_to8_fast (const u32* restrict in, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #include "utf32/utf32to8.c"
}

// -----------------------------------------------------------------------------
// UTF-32 -> UTF-16
// -----------------------------------------------------------------------------

int utf32_str_to16 (const u32* restrict in, size_t len, u16* restrict out
, size_t size, u32** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf32to16.c"
}

int utf32_str_to16_fast (const u32* restrict in, size_t len, u16* restrict out
, size_t size, u32** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf32/utf32to16.c"
}

// -----------------------------------------------------------------------------

int utf32_stri_to16 (const u32* restrict in, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_VALID

  #include "utf32/utf32to16.c"
}

int utf32_stri_to16_fast (const u32* restrict in, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #include "utf32/utf32to16.c"
}

// =============================================================================
// Byte order swapping
// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

void utf16_str_bswap (u16* str, size_t len)
{
  #define T_EXPLICIT

  #include "utf16/utf16bswap.c"
}

u16* utf16_stri_bswap (u16* str)
{
  #include "utf16/utf16bswap.c"
}

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

void utf32_str_bswap (u32* str, size_t len)
{
  #define T_EXPLICIT

  #include "utf32/utf32bswap.c"
}

u32* utf32_stri_bswap (u32* str)
{
  #include "utf32/utf32bswap.c"
}
