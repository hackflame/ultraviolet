// =============================================================================
// <ultraviolet/utf/utf32.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/platform.h>
#include <quantum/core.h>

#include <quantum/bswap.h>
#include <quantum/swar.h>
#include <quantum/simd.h>

#include "simd.h"

#include "../utf.h"

// -----------------------------------------------------------------------------
// Character count
// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

int utf32_str_valid (const u32* restrict in, size_t len
, u32** restrict end)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf32/utf32.c"
}

int utf32_istr_valid (const u32* restrict in
, u32** restrict end)
{
  #define T_VALID 1

  #include "utf32/utf32.c"
}

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-32 -> UTF-8
// -----------------------------------------------------------------------------

int utf32_str_to8 (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf32/utf8.c"
}

int utf32_str_to8_fast (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1

  #include "utf32/utf8.c"
}

// -----------------------------------------------------------------------------

int utf32_istr_to8 (const u32* restrict in
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_VALID 1

  #include "utf32/utf8.c"
}

int utf32_istr_to8_fast (const u32* restrict in
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #include "utf32/utf8.c"
}

// -----------------------------------------------------------------------------
// UTF-32 -> UTF-16
// -----------------------------------------------------------------------------

int utf32_str_to16 (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf32/utf16.c"
}

int utf32_str_to16_fast (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1

  #include "utf32/utf16.c"
}

// -----------------------------------------------------------------------------

int utf32_istr_to16 (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #define T_VALID 1

  #include "utf32/utf16.c"
}

int utf32_istr_to16_fast (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num)
{
  #include "utf32/utf16.c"
}
