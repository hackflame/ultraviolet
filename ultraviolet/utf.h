// =============================================================================
// <ultraviolet/utf.h>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------------

#include <cwift/string.h>
#include <cwift/unicode/string16.h>
#include <cwift/unicode/string32.h>
#include <cwift/bitops.h>

// -----------------------------------------------------------------------------
// UTF-8 code unit tests
// -----------------------------------------------------------------------------
// Code unit kind
// -----------------------------------------------------------------------------

#define utf8_cunit_is_lead(c) (char_is_ascii (c) || (((c) & 0xC0u) == 0xC0u))
#define utf8_cunit_is_trail(c) (((c) & 0xC0u) == 0x80u)

// -----------------------------------------------------------------------------
// Length of the multibyte sequence from the leading code unit
// -----------------------------------------------------------------------------

#define utf8_cunit_is_lead1(c) char_is_ascii (c)
#define utf8_cunit_is_lead2(c) (((c) & 0xE0u) == 0xC0u)
#define utf8_cunit_is_lead3(c) (((c) & 0xF0u) == 0xE0u)
#define utf8_cunit_is_lead4(c) (((c) & 0xF8u) == 0xF0u)

// -----------------------------------------------------------------------------
// Bits with actual code point data
#define utf8_cunit_bits(c) ((c) & 0x3Fu)

// -----------------------------------------------------------------------------
// UTF-8 code point composition / decomposition
// -----------------------------------------------------------------------------
// Compose the leading code unit
// -----------------------------------------------------------------------------

#define utf8_codep_set_lead4(c) ((u32f)((c) & 0x7u) << 18)
#define utf8_codep_set_lead3(c) (((c) & 0xFu) << 12)
#define utf8_codep_set_lead2(c) (((c) & 0x1Fu) << 6)

// -----------------------------------------------------------------------------
// Compose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_codep_set_trail3(c) (utf8_cunit_bits (c) << 12)
#define utf8_codep_set_trail2(c) (utf8_cunit_bits (c) << 6)
#define utf8_codep_set_trail1(c) utf8_cunit_bits (c)

// -----------------------------------------------------------------------------
// Compose the whole code point
// -----------------------------------------------------------------------------

#define utf8_codep_make4(c4, c3, c2, c1) (utf8_codep_set_lead4 (c4) | utf8_codep_set_trail3 (c3) | utf8_codep_set_trail2 (c2) | utf8_codep_set_trail1 (c1))
#define utf8_codep_make3(c3, c2, c1) (utf8_codep_set_lead3 (c3) | utf8_codep_set_trail2 (c2) | utf8_codep_set_trail1 (c1))
#define utf8_codep_make2(c2, c1) (utf8_codep_set_lead2 (c2) | utf8_codep_set_trail1 (c1))

// -----------------------------------------------------------------------------
// Decompose the leading code unit
// -----------------------------------------------------------------------------

#if 1
  #define utf8_codep_get_lead4(c) (((c) >> 18) | 0xF0u)
  #define utf8_codep_get_lead3(c) (((c) >> 12) | 0xE0u)
  #define utf8_codep_get_lead2(c) (((c) >> 6) | 0xC0u)
#else
  #define utf8_codep_get_lead4(c) ((((c) & 0x1C0000u) >> 18) | 0xF0u)
  #define utf8_codep_get_lead3(c) ((((c) & 0xF000u) >> 12) | 0xE0u)
  #define utf8_codep_get_lead2(c) ((((c) & 0x7C0u) >> 6) | 0xC0u)
#endif

// -----------------------------------------------------------------------------
// Decompose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_codep_get_trail3(c) (utf8_cunit_bits ((c) >> 12) | 0x80u)
#define utf8_codep_get_trail2(c) (utf8_cunit_bits ((c) >> 6) | 0x80u)
#define utf8_codep_get_trail1(c) (utf8_cunit_bits (c) | 0x80u)

// -----------------------------------------------------------------------------
// UTF-16 code unit tests
// -----------------------------------------------------------------------------

#define UTF16_HIGH_SURR_START 0xD800u
#define UTF16_HIGH_SURR_END   0xDBFFu

#define UTF16_LOW_SURR_START 0xDC00u
#define UTF16_LOW_SURR_END   0xDFFFu

#define UTF16_LEAD_OFF (UTF16_HIGH_SURR_START - (0x10000u >> 10))
#define UTF16_SURR_OFF (0x10000u - ((u32f)UTF16_HIGH_SURR_START << 10) - UTF16_LOW_SURR_START)

// -----------------------------------------------------------------------------
// Test if a code unit is (any) surrogate code unit
// -----------------------------------------------------------------------------

#if 1
  #define utf16_cunit_is_surr(c) (((c) & 0xF800u) == UTF16_HIGH_SURR_START)
#else
  #define utf16_cunit_is_surr(c) (((c) >= UTF16_HIGH_SURR_START) && ((c) <= UTF16_LOW_SURR_END))
#endif

// -----------------------------------------------------------------------------
// Test if a code unit is a high surrogate code unit
// -----------------------------------------------------------------------------

#if 1
  #define utf16_cunit_is_surr_high(c) (((c) & 0xFC00u) == UTF16_HIGH_SURR_START)
#else
  #define utf16_cunit_is_surr_high(c) (((c) >= UTF16_HIGH_SURR_START) && ((c) <= UTF16_HIGH_SURR_END))
#endif

// -----------------------------------------------------------------------------
// Test if a code unit is a low surrogate code unit
// -----------------------------------------------------------------------------

#if 1
  #define utf16_cunit_is_surr_low(c) (((c) & 0xFC00u) == UTF16_LOW_SURR_START)
#else
  #define utf16_cunit_is_surr_low(c) (((c) >= UTF16_LOW_SURR_START) && ((c) <= UTF16_LOW_SURR_END))
#endif

// -----------------------------------------------------------------------------
// UTF-16 code point tests
// -----------------------------------------------------------------------------
// Test if a code point is a non-character
#define utf16_codep_is_non(c) (((c) - 0xFDD0u) < 32u)

// -----------------------------------------------------------------------------
// Test if a code point is a reserved character
#define utf16_codep_is_rsrv(c) (((c) & 0xFFFEu) == 0xFFFEu)

// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-16 code point decodes to.
// Test for a surrogate UTF-16 code unit first.
// -----------------------------------------------------------------------------

#define utf16_codep_is_lead1(c) char_is_ascii (c)
#define utf16_codep_is_lead2(c) ((c) < 0x800u)
#define utf16_codep_is_lead3(c) ((c) >= 0x800u)

// -----------------------------------------------------------------------------
// Construct a UTF-16 surrogate pair from a UTF-32 code point
// -----------------------------------------------------------------------------

#define utf16_make_surr_high(c) (((c) >> 10) + UTF16_LEAD_OFF)
#define utf16_make_surr_low(c) (((c) & 0x3FFu) | UTF16_LOW_SURR_START)

// -----------------------------------------------------------------------------
// Construct a UTF-32 code point from a UTF-16 surrogate pair
#define utf16_surr_to_char(hi, lo) ((((u32f)((hi) & 0x3FFu) << 10) + 0x10000u) | ((lo) & 0x3FFu))

// -----------------------------------------------------------------------------
// UTF-32 code point tests
// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-32 code point decodes to
// -----------------------------------------------------------------------------

#define utf32_codep_is_lead1(c) char_is_ascii (c)
#define utf32_codep_is_lead2(c) utf16_codep_is_lead2 (c)
#define utf32_codep_is_lead3(c) ((c) < 0x10000u)
#define utf32_codep_is_lead4(c) ((c) < 0x110000u)

// -----------------------------------------------------------------------------
// Test if a given UTF-32 code point decodes into UTF-16 surrogate pair
#define utf32_codep_is_surr(c) ((c) >= 0x10000u)

// =============================================================================
// Functions
// -----------------------------------------------------------------------------
// Navigation
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

static inline u8* utf8_str_sync (const u8* buf, const u8* end)
{
  while (buf != end)
  {
    if (utf8_cunit_is_lead (*buf)) return ptr_unconst(buf);
    buf++;
  }

  return null;
}

static inline u8* utf8_str_rsync (const u8* buf, const u8* start)
{
  while (buf >= start)
  {
    if (utf8_cunit_is_lead (*buf)) return ptr_unconst(buf);
    buf--;
  }

  return null;
}

// -----------------------------------------------------------------------------
// Implicit string
// -----------------------------------------------------------------------------

static inline u8* utf8_zstr_sync (const u8* str)
{
  while (true)
  {
    if (utf8_cunit_is_lead (*str)) return ptr_unconst(str);
    str++;
  }

  assume_unreachable();
}

#define utf8_zstr_rsync utf8_str_rsync

// -----------------------------------------------------------------------------
// `ptr` must be synchronized
// -----------------------------------------------------------------------------

static inline size_t utf8_codep_size (const u8* ptr)
{
  register u8f u = ptr[0];

  if (utf8_cunit_is_lead1 (u)) return 1;

#if CPU_X86
  register size_t n = (size_t)bsr32 (~u << 24);

  if (unlikely (n > 4)) return 0;

  return n;
#else
  if (utf8_cunit_is_lead2 (u)) return 2;
  if (utf8_cunit_is_lead3 (u)) return 3;
  if (utf8_cunit_is_lead4 (u)) return 4;

  return 0;
#endif
}

// -----------------------------------------------------------------------------

static inline u8* utf8_codep_next (const u8* ptr)
{
  size_t size = utf8_codep_size (ptr);

  if (unlikely (size == 0)) return null;

  return ptr_unconst(ptr + size);
}

// -----------------------------------------------------------------------------

static inline u8* utf8_str_seek (const u8* buf, const u8* end)
{
  const u8* ptr = utf8_codep_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return ptr_unconst(ptr);
}

static inline u8* utf8_str_rseek (const u8* buf, const u8* start)
{
  return utf8_str_rsync (buf - 1u, start);
}

// -----------------------------------------------------------------------------

static inline u8* utf8_zstr_seek (const u8* str)
{
  return utf8_zstr_sync (str + 1u);
}

#define utf8_zstr_rseek(str) utf8_str_rseek (str)

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

static inline u16* utf16_str_sync (const u16* buf, const u16* end)
{
  if (utf16_cunit_is_surr_low (*buf))
  {
    buf++;
    if (unlikely (buf == end)) return null;
  }

  return ptr_unconst(buf);
}

static inline u16* utf16_str_rsync (const u16* buf, const u16* start)
{
  if (utf16_cunit_is_surr_low (*buf))
  {
    if (unlikely (buf == start)) return null;
    buf--;
  }

  return ptr_unconst(buf);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_zstr_sync (const u16* str)
{
  if (utf16_cunit_is_surr_low (*str)) return ptr_unconst(str + 1u);
  return ptr_unconst(str);
}

#define utf16_zstr_rsync(str) utf16_str_rsync (str)

// -----------------------------------------------------------------------------

static inline size_t utf16_codep_size (const u16* ptr)
{
  register u16f u = ptr[0];

  if (!utf16_cunit_is_surr (u)) return 1;
  if (utf16_cunit_is_surr_high (u)) return 2;

  return 0;
}

// -----------------------------------------------------------------------------

static inline u16* utf16_codep_next (const u16* ptr)
{
  size_t size = utf16_codep_size (ptr);

  if (unlikely (size == 0)) return null;

  return ptr_unconst(ptr + size);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_str_seek (const u16* buf, const u16* end)
{
  const u16* ptr = utf16_codep_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return ptr_unconst(ptr);
}

static inline u16* utf16_str_rseek (const u16* buf, const u16* start)
{
  return utf16_str_rsync (buf - 1u, start);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_zstr_seek (const u16* str)
{
  return utf16_zstr_sync (str + 1u);
}

#define utf16_zstr_rseek utf16_str_rseek

// -----------------------------------------------------------------------------
// Validation and number of characters in a UTF string
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

extern int utf8_str_valid (const u8* restrict buf, size_t len
, u8** restrict end, size_t* restrict num);

extern int utf8_str_runes (const u8* restrict buf, size_t len
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf8_zstr_valid (const u8* restrict str
, u8** restrict end, size_t* restrict num);

extern int utf8_zstr_runes (const u8* restrict str
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

extern int utf16_str_valid (const u16* restrict buf, size_t len
, u16** restrict end, size_t* restrict num);

extern int utf16_str_runes (const u16* restrict buf, size_t len
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf16_zstr_valid (const u16* restrict str
, u16** restrict end, size_t* restrict num);

extern int utf16_zstr_runes (const u16* restrict str
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

extern int utf32_str_valid (const u32* restrict buf
, size_t len, u32** restrict end);

extern int utf32_zstr_valid (const u32* restrict str
, u32** restrict end);

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-8 to UTF-16
// -----------------------------------------------------------------------------

extern int utf8_str_to16 (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_str_to16_fast (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf8_zstr_to16 (const u8* restrict in
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_zstr_to16_fast (const u8* restrict in
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-8 to UTF-32
// -----------------------------------------------------------------------------

extern int utf8_str_to32 (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_str_to32_fast (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf8_zstr_to32 (const u8* restrict in
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_zstr_to32_fast (const u8* restrict in
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-8
// -----------------------------------------------------------------------------

extern int utf16_str_to8 (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_str_to8_fast (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf16_zstr_to8 (const u16* restrict in
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_zstr_to8_fast (const u16* restrict in
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-32
// -----------------------------------------------------------------------------

extern int utf16_str_to32 (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_str_to32_fast (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf16_zstr_to32 (const u16* restrict in
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_zstr_to32_fast (const u16* restrict in
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-8
// -----------------------------------------------------------------------------

extern int utf32_str_to8 (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_str_to8_fast (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf32_zstr_to8 (const u32* restrict in
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_zstr_to8_fast (const u32* restrict in
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-16
// -----------------------------------------------------------------------------

extern int utf32_str_to16 (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_str_to16_fast (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf32_zstr_to16 (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_zstr_to16_fast (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);
