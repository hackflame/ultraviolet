// =============================================================================
// <ultraviolet/utf/utf16.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <cwift/platform.h>
#include <cwift/core.h>

#include <cwift/bswap.h>
#include <cwift/swar.h>
#include <cwift/simd.h>

#include "simd.h"

#include "../utf.h"

// -----------------------------------------------------------------------------
// Character count
// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

int utf16_str_valid (const u16* restrict in, size_t len
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf16/utf16.c"
}

int utf16_str_runes (const u16* restrict in, size_t len
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1

  #include "utf16/utf16.c"
}

// -----------------------------------------------------------------------------

int utf16_zstr_valid (const u16* restrict in
, u16** restrict end, size_t* restrict num)
{
  #define T_VALID 1

  #include "utf16/utf16.c"
}

int utf16_zstr_runes (const u16* restrict in
, u16** restrict end, size_t* restrict num)
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
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf16/utf8.c"
}

int utf16_str_to8_fast (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1

  #include "utf16/utf8.c"
}

// -----------------------------------------------------------------------------

int utf16_zstr_to8 (const u16* restrict in
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #define T_VALID 1

  #include "utf16/utf8.c"
}

int utf16_zstr_to8_fast (const u16* restrict in
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #include "utf16/utf8.c"
}

// -----------------------------------------------------------------------------
// UTF-16 -> UTF-32
// -----------------------------------------------------------------------------

int utf16_str_to32 (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1
  #define T_VALID 1

  #include "utf16/utf32.c"
}

int utf16_str_to32_fast (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #define T_EXPLICIT 1

  #include "utf16/utf32.c"
}

// -----------------------------------------------------------------------------

int utf16_zstr_to32 (const u16* restrict in
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #define T_VALID 1

  #include "utf16/utf32.c"
}

int utf16_zstr_to32_fast (const u16* restrict in
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num)
{
  #include "utf16/utf32.c"
}
