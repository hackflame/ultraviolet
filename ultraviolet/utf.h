// =============================================================================
//                               Ultraviolet
// -----------------------------------------------------------------------------
// <ultraviolet/utf.h>
//
// Unicode transformation library for UTF-8, UTF-16, and UTF-32 encodings.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#ifndef H_D937C20EDC684AA1802F8E6F29FE54C3
#define H_D937C20EDC684AA1802F8E6F29FE54C3 1

// -----------------------------------------------------------------------------

#define ULTRAVIOLET ((0L * 10000) + (0L * 100) + 1L)

// -----------------------------------------------------------------------------

#include <cwift/string.h>
#include <cwift/unicode/string16.h>
#include <cwift/unicode/string32.h>
#include <cwift/bitops.h>

// =============================================================================
// Constants
// -----------------------------------------------------------------------------
// Maximum multibyte sequence lengths
// -----------------------------------------------------------------------------

#define UTF8_SEQ_LEN_MAX  4u
#define UTF16_SEQ_LEN_MAX 2u

// -----------------------------------------------------------------------------
// http://unicode.org/faq/utf_bom.html
// -----------------------------------------------------------------------------

#define UTF16_HIGH_SURR_BEG 0xD800u
#define UTF16_HIGH_SURR_END 0xDBFFu

#define UTF16_LOW_SURR_BEG 0xDC00u
#define UTF16_LOW_SURR_END 0xDFFFu

#define UTF16_LEAD_OFF (UTF16_HIGH_SURR_BEG - (0x10000u >> 10))
#define UTF16_SURR_OFF (0x10000u - ((u32f)UTF16_HIGH_SURR_BEG << 10) - UTF16_LOW_SURR_BEG)

// =============================================================================
// Macros
// -----------------------------------------------------------------------------
// UTF-8 byte tests
// -----------------------------------------------------------------------------
// Byte kind
// -----------------------------------------------------------------------------

#define utf8_byte_is_lead(c) (char_is_ascii (c) || (((c) & 0xC0u) == 0xC0u))
#define utf8_byte_is_trail(c) (((c) & 0xC0u) == 0x80u)

// -----------------------------------------------------------------------------
// Length of the multibyte sequence from the leading byte
// -----------------------------------------------------------------------------

#define utf8_byte_is_lead1(c) char_is_ascii (c)
#define utf8_byte_is_lead2(c) (((c) & 0xE0u) == 0xC0u)
#define utf8_byte_is_lead3(c) (((c) & 0xF0u) == 0xE0u)
#define utf8_byte_is_lead4(c) (((c) & 0xF8u) == 0xF0u)

// -----------------------------------------------------------------------------
// Bits with actual character data
#define utf8_byte_bits(c) ((c) & 0x3Fu)

// -----------------------------------------------------------------------------
// UTF-8 character composition / decomposition
// -----------------------------------------------------------------------------
// Compose the leading byte
// -----------------------------------------------------------------------------

#define utf8_char_set_lead4(c) ((u32f)((c) & 0x7u) << 18)
#define utf8_char_set_lead3(c) (((c) & 0xFu) << 12)
#define utf8_char_set_lead2(c) (((c) & 0x1Fu) << 6)

// -----------------------------------------------------------------------------
// Compose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_char_set_trail3(c) (utf8_byte_bits (c) << 12)
#define utf8_char_set_trail2(c) (utf8_byte_bits (c) << 6)
#define utf8_char_set_trail1(c) utf8_byte_bits (c)

// -----------------------------------------------------------------------------
// Compose the whole character
// -----------------------------------------------------------------------------

#define utf8_char_make4(c4, c3, c2, c1) (utf8_char_set_lead4 (c4) | utf8_char_set_trail3 (c3) | utf8_char_set_trail2 (c2) | utf8_char_set_trail1 (c1))
#define utf8_char_make3(c3, c2, c1) (utf8_char_set_lead3 (c3) | utf8_char_set_trail2 (c2) | utf8_char_set_trail1 (c1))
#define utf8_char_make2(c2, c1) (utf8_char_set_lead2 (c2) | utf8_char_set_trail1 (c1))

// -----------------------------------------------------------------------------
// Decompose the leading byte
// -----------------------------------------------------------------------------

#if 1
  #define utf8_char_get_lead4(c) (((c) >> 18) | 0xF0u)
  #define utf8_char_get_lead3(c) (((c) >> 12) | 0xE0u)
  #define utf8_char_get_lead2(c) (((c) >> 6) | 0xC0u)
#else
  #define utf8_char_get_lead4(c) ((((c) & 0x1C0000u) >> 18) | 0xF0u)
  #define utf8_char_get_lead3(c) ((((c) & 0xF000u) >> 12) | 0xE0u)
  #define utf8_char_get_lead2(c) ((((c) & 0x7C0u) >> 6) | 0xC0u)
#endif

// -----------------------------------------------------------------------------
// Decompose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_char_get_trail3(c) (utf8_byte_bits ((c) >> 12) | 0x80u)
#define utf8_char_get_trail2(c) (utf8_byte_bits ((c) >> 6) | 0x80u)
#define utf8_char_get_trail1(c) (utf8_byte_bits (c) | 0x80u)

// -----------------------------------------------------------------------------
// UTF-16 byte tests
// -----------------------------------------------------------------------------
// Test if a byte is (any) surrogate byte
// -----------------------------------------------------------------------------

#if 1
  #define utf16_byte_is_surr(c) (((c) & 0xF800u) == 0xD800u)
#else
  #define utf16_byte_is_surr(c) (((c) >= UTF16_HIGH_SURR_BEG) && ((c) <= UTF16_LOW_SURR_END))
#endif

// -----------------------------------------------------------------------------
// Test if a byte is a high surrogate byte
// -----------------------------------------------------------------------------

#if 1
  #define utf16_byte_is_surr_high(c) (((c) & 0xFC00u) == 0xD800u)
#else
  #define utf16_byte_is_surr_high(c) (((c) >= UTF16_HIGH_SURR_BEG) && ((c) <= UTF16_HIGH_SURR_END))
#endif

// -----------------------------------------------------------------------------
// Test if a byte is a low surrogate byte
// -----------------------------------------------------------------------------

#if 1
  #define utf16_byte_is_surr_low(c) (((c) & 0xFC00u) == 0xDC00u)
#else
  #define utf16_byte_is_surr_low(c) (((c) >= UTF16_LOW_SURR_BEG) && ((c) <= UTF16_LOW_SURR_END))
#endif

// -----------------------------------------------------------------------------
// UTF-16 character tests
// -----------------------------------------------------------------------------
// Test if a character is a non-character
#define utf16_char_is_non(c) (((c) - 0xFDD0u) < 32u)

// -----------------------------------------------------------------------------
// Test if a character is a reserved character
#define utf16_char_is_rsrv(c) (((c) & 0xFFFEu) == 0xFFFEu)

// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-16 character decodes to.
// Test for a surrogate UTF-16 byte first.
// -----------------------------------------------------------------------------

#define utf16_char_is_lead1(c) char_is_ascii (c)
#define utf16_char_is_lead2(c) ((c) < 0x800u)
#define utf16_char_is_lead3(c) ((c) >= 0x800u)

// -----------------------------------------------------------------------------
// Construct a UTF-16 surrogate pair from a UTF-32 character
// -----------------------------------------------------------------------------

#define utf16_make_surr_high(c) (((c) >> 10) + UTF16_LEAD_OFF)
#define utf16_make_surr_low(c) (((c) & 0x3FFu) | UTF16_LOW_SURR_BEG)

// -----------------------------------------------------------------------------
// Construct a UTF-32 character from a UTF-16 surrogate pair
#define utf16_surr_to_char(hi, lo) ((((u32f)((hi) & 0x3FFu) << 10) + 0x10000u) | ((lo) & 0x3FFu))

// -----------------------------------------------------------------------------
// UTF-32 character tests
// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-32 character decodes to
// -----------------------------------------------------------------------------

#define utf32_char_is_lead1(c) char_is_ascii (c)
#define utf32_char_is_lead2(c) utf16_char_is_lead2 (c)
#define utf32_char_is_lead3(c) ((c) < 0x10000u)
#define utf32_char_is_lead4(c) ((c) < 0x110000u)

// -----------------------------------------------------------------------------
// Test if a given UTF-32 character decodes into UTF-16 surrogate pair
#define utf32_char_is_surr(c) ((c) >= 0x10000u)

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
    if (utf8_byte_is_lead (*buf)) return ptr_unconst(buf);
    buf++;
  }

  return null;
}

static inline u8* utf8_str_rsync (const u8* buf, const u8* beg)
{
  while (buf >= beg)
  {
    if (utf8_byte_is_lead (*buf)) return ptr_unconst(buf);
    buf--;
  }

  return null;
}

// -----------------------------------------------------------------------------
// Implicit string
// -----------------------------------------------------------------------------

static inline u8* utf8_istr_sync (const u8* str)
{
  while (true)
  {
    if (utf8_byte_is_lead (*str)) return ptr_unconst(str);
    str++;
  }

  assume_unreachable();
}

#define utf8_istr_rsync(str) utf8_str_rsync (str)

// -----------------------------------------------------------------------------
// `ptr` must be synchronized
static inline u8* utf8_char_next (const u8* ptr)
{
  register u8f b = *ptr;

  if (utf8_byte_is_lead1 (b)) return ptr_unconst(ptr + 1u);

#if CPU_X86
  register uint n = bsr32 (~b << 24);

  if (unlikely (n > 4u)) return null;

  return ptr_unconst(ptr + n);
#else
  if (utf8_byte_is_lead2 (b)) return ptr_unconst(ptr + 2u);
  if (utf8_byte_is_lead3 (b)) return ptr_unconst(ptr + 3u);
  if (utf8_byte_is_lead4 (b)) return ptr_unconst(ptr + 4u);

  return null;
#endif
}

// -----------------------------------------------------------------------------

static inline u8* utf8_str_seek (const u8* buf, const u8* end)
{
  const u8* ptr = utf8_char_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return ptr_unconst(ptr);
}

static inline u8* utf8_str_rseek (const u8* buf, const u8* beg)
{
  return utf8_str_rsync (buf - 1u, beg);
}

// -----------------------------------------------------------------------------

static inline u8* utf8_istr_seek (const u8* str)
{
  return utf8_istr_sync (str + 1u);
}

#define utf8_istr_rseek(str) utf8_str_rseek (str)

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

static inline u16* utf16_str_sync (const u16* buf, const u16* end)
{
  if (utf16_byte_is_surr_low (*buf))
  {
    buf++;
    if (unlikely (buf == end)) return null;
  }

  return ptr_unconst(buf);
}

static inline u16* utf16_str_rsync (const u16* buf, const u16* beg)
{
  if (utf16_byte_is_surr_low (*buf))
  {
    if (unlikely (buf == beg)) return null;
    buf--;
  }

  return ptr_unconst(buf);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_istr_sync (const u16* str)
{
  if (utf16_byte_is_surr_low (*str)) return ptr_unconst(str + 1u);
  return ptr_unconst(str);
}

#define utf16_istr_rsync(str) utf16_str_rsync (str)

// -----------------------------------------------------------------------------

static inline u16* utf16_char_next (const u16* ptr)
{
  register u16f b = *ptr;

  if (!utf16_byte_is_surr (b)) return ptr_unconst(ptr + 1u);
  if (utf16_byte_is_surr_high (b)) return ptr_unconst(ptr + 2u);

  return null;
}

// -----------------------------------------------------------------------------

static inline u16* utf16_str_seek (const u16* buf, const u16* end)
{
  const u16* ptr = utf16_char_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return ptr_unconst(ptr);
}

static inline u16* utf16_str_rseek (const u16* buf, const u16* beg)
{
  return utf16_str_rsync (buf - 1u, beg);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_istr_seek (const u16* str)
{
  return utf16_istr_sync (str + 1u);
}

#define utf16_istr_rseek(str) utf16_str_rseek (str)

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

extern int utf8_istr_valid (const u8* restrict str
, u8** restrict end, size_t* restrict num);

extern int utf8_istr_runes (const u8* restrict str
, u8** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

extern int utf16_str_valid (const u16* restrict buf, size_t len
, u16** restrict end, size_t* restrict num);

extern int utf16_str_runes (const u16* restrict buf, size_t len
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

extern int utf16_istr_valid (const u16* restrict str
, u16** restrict end, size_t* restrict num);

extern int utf16_istr_runes (const u16* restrict str
, u16** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

extern int utf32_str_valid (const u32* restrict buf
, size_t len, u32** restrict end);

extern int utf32_istr_valid (const u32* restrict str
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

extern int utf8_istr_to16 (const u8* restrict in
, u16* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_istr_to16_fast (const u8* restrict in
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

extern int utf8_istr_to32 (const u8* restrict in
, u32* restrict out, size_t size
, u8** restrict end, size_t* restrict num);

extern int utf8_istr_to32_fast (const u8* restrict in
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

extern int utf16_istr_to8 (const u16* restrict in
, u8* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_istr_to8_fast (const u16* restrict in
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

extern int utf16_istr_to32 (const u16* restrict in
, u32* restrict out, size_t size
, u16** restrict end, size_t* restrict num);

extern int utf16_istr_to32_fast (const u16* restrict in
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

extern int utf32_istr_to8 (const u32* restrict in
, u8* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_istr_to8_fast (const u32* restrict in
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

extern int utf32_istr_to16 (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

extern int utf32_istr_to16_fast (const u32* restrict in
, u16* restrict out, size_t size
, u32** restrict end, size_t* restrict num);

// -----------------------------------------------------------------------------

#endif
