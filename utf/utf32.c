// =============================================================================
// <utf/utf32.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/build.h>
#include <quantum/core.h>

#include <quantum/bswap.h>
#include <quantum/swar.h>
#include <quantum/simd.h>

#include "templates.h"
#include "simd.h"

#include "../utf.h"

// -----------------------------------------------------------------------------
// Byte length
// -----------------------------------------------------------------------------

size_t utf32_istr_length (const u32* str)
{
  #include "utf32/length.c"
}

// -----------------------------------------------------------------------------
// Character count
// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

int utf32_str_valid (const u32* in, size_t len
, u32** end)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf32.c"
}

int utf32_istr_valid (const u32* in
, u32** end)
{
  #define T_VALID

  #include "utf32/utf32.c"
}

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-32 -> UTF-8
// -----------------------------------------------------------------------------

int utf32_str_to8 (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf8.c"
}

int utf32_str_to8_fast (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf32/utf8.c"
}

// -----------------------------------------------------------------------------

int utf32_istr_to8 (const u32* restrict in
, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_VALID

  #include "utf32/utf8.c"
}

int utf32_istr_to8_fast (const u32* restrict in
, u8* restrict out, size_t size
, u32** end, size_t* num)
{
  #include "utf32/utf8.c"
}

// -----------------------------------------------------------------------------
// UTF-32 -> UTF-16
// -----------------------------------------------------------------------------

int utf32_str_to16 (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf32/utf16.c"
}

int utf32_str_to16_fast (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf32/utf16.c"
}

// -----------------------------------------------------------------------------

int utf32_istr_to16 (const u32* restrict in
, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #define T_VALID

  #include "utf32/utf16.c"
}

int utf32_istr_to16_fast (const u32* restrict in
, u16* restrict out, size_t size
, u32** end, size_t* num)
{
  #include "utf32/utf16.c"
}

// -----------------------------------------------------------------------------
// Byte order swapping
// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

void utf32_str_bswap (u32* str, size_t len)
{
  #define T_EXPLICIT

  #include "utf32/bswap.c"
}

u32* utf32_istr_bswap (u32* str)
{
  #include "utf32/bswap.c"
}
