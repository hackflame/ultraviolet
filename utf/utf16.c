// =============================================================================
// <utf/utf16.c>
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

size_t utf16_istr_length (const u16* str)
{
  #include "utf16/length.c"
}

// -----------------------------------------------------------------------------
// Character count
// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

int utf16_str_valid (const u16* in, size_t len
, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf16.c"
}

int utf16_str_runes (const u16* in, size_t len
, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf16.c"
}

// -----------------------------------------------------------------------------

int utf16_istr_valid (const u16* in
, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf16.c"
}

int utf16_istr_runes (const u16* in
, u16** end, size_t* num)
{
  #include "utf16/utf16.c"
}

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-16 -> UTF-8
// -----------------------------------------------------------------------------

int utf16_str_to8 (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf8.c"
}

int utf16_str_to8_fast (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf8.c"
}

// -----------------------------------------------------------------------------

int utf16_istr_to8 (const u16* restrict in
, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf8.c"
}

int utf16_istr_to8_fast (const u16* restrict in
, u8* restrict out, size_t size
, u16** end, size_t* num)
{
  #include "utf16/utf8.c"
}

// -----------------------------------------------------------------------------
// UTF-16 -> UTF-32
// -----------------------------------------------------------------------------

int utf16_str_to32 (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf16/utf32.c"
}

int utf16_str_to32_fast (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_EXPLICIT

  #include "utf16/utf32.c"
}

// -----------------------------------------------------------------------------

int utf16_istr_to32 (const u16* restrict in
, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #define T_VALID

  #include "utf16/utf32.c"
}

int utf16_istr_to32_fast (const u16* restrict in
, u32* restrict out, size_t size
, u16** end, size_t* num)
{
  #include "utf16/utf32.c"
}

// -----------------------------------------------------------------------------
// Byte order swapping
// -----------------------------------------------------------------------------

void utf16_str_bswap (u16* str, size_t len)
{
  #define T_EXPLICIT

  #include "utf16/bswap.c"
}

u16* utf16_istr_bswap (u16* str)
{
  #include "utf16/bswap.c"
}
