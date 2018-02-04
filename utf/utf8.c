// =============================================================================
// <utf/utf8.c>
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

size_t utf8_istr_length (const u8* str)
{
  #include "utf8/length.c"
}

// -----------------------------------------------------------------------------
// Character count
// -----------------------------------------------------------------------------

int utf8_str_valid (const u8* restrict in, size_t len
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf8.c"
}

int utf8_str_runes (const u8* restrict in, size_t len
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT

  #include "utf8/utf8.c"
}

// -----------------------------------------------------------------------------

int utf8_istr_valid (const u8* restrict in
, u8** restrict end, size_t* restrict num)
{
  #define T_VALID

  #include "utf8/utf8.c"
}

int utf8_istr_runes (const u8* restrict in
, u8** restrict end, size_t* restrict num)
{
  #include "utf8/utf8.c"
}

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-8 -> UTF-16
// -----------------------------------------------------------------------------

int utf8_str_to16 (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf16.c"
}

int utf8_str_to16_fast (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT

  #include "utf8/utf16.c"
}

// -----------------------------------------------------------------------------

int utf8_istr_to16 (const u8* restrict in
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_VALID

  #include "utf8/utf16.c"
}

int utf8_istr_to16_fast (const u8* restrict in
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #include "utf8/utf16.c"
}

// -----------------------------------------------------------------------------
// UTF-8 -> UTF-32
// -----------------------------------------------------------------------------

int utf8_str_to32 (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT
  #define T_VALID

  #include "utf8/utf32.c"
}

int utf8_str_to32_fast (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT

  #include "utf8/utf32.c"
}

// -----------------------------------------------------------------------------

int utf8_istr_to32 (const u8* restrict in
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #define T_VALID

  #include "utf8/utf32.c"
}

int utf8_istr_to32_fast (const u8* restrict in
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num)
{
  #include "utf8/utf32.c"
}
