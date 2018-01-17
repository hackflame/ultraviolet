// =============================================================================
//                               Ultraviolet
// -----------------------------------------------------------------------------
// <utf.h>
//
// Unicode transformation library for UTF-8, UTF-16, and UTF-32 encodings.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#ifndef H_D937C20EDC684AA1802F8E6F29FE54C3
#define H_D937C20EDC684AA1802F8E6F29FE54C3

// -----------------------------------------------------------------------------

#define UTF(feat) (defined(UTF_##feat))

// -----------------------------------------------------------------------------

#define ULTRAVIOLET_VER 0x000001u
#define ULTRAVIOLET_VER_STR "0.0.1"

// -----------------------------------------------------------------------------

#include <quantum/bitops.h>
#include <quantum/character.h>
#include <quantum/unicode.h>

// =============================================================================
// Constants
// -----------------------------------------------------------------------------
// Maximum multibyte sequence lengths
// -----------------------------------------------------------------------------

#define UTF8_SEQ_MAX  4u
#define UTF16_SEQ_MAX 2u

// -----------------------------------------------------------------------------
// http://unicode.org/faq/utf_bom.html
// -----------------------------------------------------------------------------

#define UTF16_HIGH_SURR_BEG 0xD800u
#define UTF16_HIGH_SURR_END 0xDBFFu

#define UTF16_LOW_SURR_BEG 0xDC00u
#define UTF16_LOW_SURR_END 0xDFFFu

#define UTF16_LEAD_OFFSET (UTF16_HIGH_SURR_BEG - (0x10000u >> 10))
#define UTF16_SURR_OFFSET (0x10000u - ((uf32)UTF16_HIGH_SURR_BEG << 10) - UTF16_LOW_SURR_BEG)

// =============================================================================
// Types
// -----------------------------------------------------------------------------
// Psst: just use `u8`, `u16`, and `u32`
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

typedef u8 utf8_char_t;
typedef utf8_char_t utf8_chr_t;

typedef u16 utf16_char_t;
typedef utf16_char_t utf16_chr_t;

typedef u32 utf32_char_t;
typedef utf32_char_t utf32_chr_t;

// -----------------------------------------------------------------------------

typedef struct utf8_str_s
{
  size_t len;
  utf8_chr_t* buf;
} utf8_str_t;

static const utf8_str_t utf8_str_null = (utf8_str_t){0};

#define utf8_str_make(buf, len) (utf8_str_t){(len), (buf)}
#define utf8_str_from_istr(str) utf8_str_make ((str), utf8_istr_length (str))

// -----------------------------------------------------------------------------

typedef struct utf8_str_const_s
{
  size_t len;
  const utf8_chr_t* buf;
} utf8_str_const_t;

static const utf8_str_const_t utf8_str_const_null = (utf8_str_const_t){0};

#define utf8_str_const_make(buf, len) (utf8_str_const_t){(len), (buf)}

#define utf8_str_const_from_istr(str) utf8_str_const_make ((str), utf8_istr_length (str))
#define utf8_str_const_from_cstr(str) utf8_str_const_make ((str), cstrlen (str))

// -----------------------------------------------------------------------------
// These typedefs should not be used.
//
// They are provided only for self-documentation purposes and completeness sake.
// -----------------------------------------------------------------------------

typedef utf8_chr_t* utf8_istr_t;
typedef const utf8_chr_t* utf8_istr_const_t;

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

typedef struct utf16_str_s
{
  size_t len;
  utf16_chr_t* buf;
} utf16_str_t;

static const utf16_str_t utf16_str_null = (utf16_str_t){0};

#define utf16_str_make(buf, len) (utf16_str_t){(len), (buf)}
#define utf16_str_from_istr(str) utf16_str_make ((str), utf16_istr_length (str))

// -----------------------------------------------------------------------------

typedef struct utf16_str_const_s
{
  size_t len;
  const utf16_chr_t* buf;
} utf16_str_const_t;

static const utf16_str_const_t utf16_str_const_null = (utf16_str_const_t){0};

#define utf16_str_const_make(buf, len) (utf16_str_const_t){(len), (buf)}

#define utf16_str_const_from_istr(str) utf16_str_const_make ((str), utf16_istr_length (str))
#define utf16_str_const_from_cstr(str) utf16_str_const_make ((str), cstrlen (str))

// -----------------------------------------------------------------------------

typedef utf16_chr_t* utf16_istr_t;
typedef const utf16_chr_t* utf16_istr_const_t;

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

typedef struct utf32_str_s
{
  size_t len;
  utf32_chr_t* buf;
} utf32_str_t;

static const utf32_str_t utf32_str_null = (utf32_str_t){0};

#define utf32_str_make(buf, len) (utf32_str_t){(len), (buf)}
#define utf32_str_from_istr(str) utf32_str_make ((str), utf32_istr_length (str))

// -----------------------------------------------------------------------------

typedef struct utf32_str_const_s
{
  size_t len;
  const utf32_chr_t* buf;
} utf32_str_const_t;

static const utf32_str_const_t utf32_str_const_null = (utf32_str_const_t){0};

#define utf32_str_const_make(buf, len) (utf32_str_const_t){(len), (buf)}

#define utf32_str_const_from_istr(str) utf32_str_const_make ((str), utf32_istr_length (str))
#define utf32_str_const_from_cstr(str) utf32_str_const_make ((str), cstrlen (str))

// -----------------------------------------------------------------------------

typedef utf32_chr_t* utf32_istr_t;
typedef const utf32_chr_t* utf32_istr_const_t;

// =============================================================================
// Macros
// -----------------------------------------------------------------------------
// UTF-8 byte tests
// -----------------------------------------------------------------------------
// Byte kind
// -----------------------------------------------------------------------------

#define utf8_byte_is_lead(c) (chr_is_ascii (c) || (((c) & 0xC0u) == 0xC0u))
#define utf8_byte_is_trail(c) (((c) & 0xC0u) == 0x80u)

// -----------------------------------------------------------------------------
// Length of the multibyte sequence from the leading byte
// -----------------------------------------------------------------------------

#define utf8_byte_is_lead1(c) chr_is_ascii (c)
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

#define utf8_chr_set_lead4(c) ((uf32)((c) & 0x7u) << 18)
#define utf8_chr_set_lead3(c) (((c) & 0xFu) << 12)
#define utf8_chr_set_lead2(c) (((c) & 0x1Fu) << 6)

// -----------------------------------------------------------------------------
// Compose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_chr_set_trail3(c) (utf8_byte_bits (c) << 12)
#define utf8_chr_set_trail2(c) (utf8_byte_bits (c) << 6)
#define utf8_chr_set_trail1(c) utf8_byte_bits (c)

// -----------------------------------------------------------------------------
// Compose the whole character
// -----------------------------------------------------------------------------

#define utf8_chr_make4(c4, c3, c2, c1) (utf8_chr_set_lead4 (c4) | utf8_chr_set_trail3 (c3) | utf8_chr_set_trail2 (c2) | utf8_chr_set_trail1 (c1))
#define utf8_chr_make3(c3, c2, c1) (utf8_chr_set_lead3 (c3) | utf8_chr_set_trail2 (c2) | utf8_chr_set_trail1 (c1))
#define utf8_chr_make2(c2, c1) (utf8_chr_set_lead2 (c2) | utf8_chr_set_trail1 (c1))

// -----------------------------------------------------------------------------
// Decompose the leading byte
// -----------------------------------------------------------------------------

#if 1
  #define utf8_chr_get_lead4(c) (((c) >> 18) | 0xF0u)
  #define utf8_chr_get_lead3(c) (((c) >> 12) | 0xE0u)
  #define utf8_chr_get_lead2(c) (((c) >> 6) | 0xC0u)
#else
  #define utf8_chr_get_lead4(c) ((((c) & 0x1C0000u) >> 18) | 0xF0u)
  #define utf8_chr_get_lead3(c) ((((c) & 0xF000u) >> 12) | 0xE0u)
  #define utf8_chr_get_lead2(c) ((((c) & 0x7C0u) >> 6) | 0xC0u)
#endif

// -----------------------------------------------------------------------------
// Decompose the trailing bytes
// -----------------------------------------------------------------------------

#define utf8_chr_get_trail3(c) (utf8_byte_bits ((c) >> 12) | 0x80u)
#define utf8_chr_get_trail2(c) (utf8_byte_bits ((c) >> 6) | 0x80u)
#define utf8_chr_get_trail1(c) (utf8_byte_bits (c) | 0x80u)

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
#define utf16_chr_is_non(c) (((c) - 0xFDD0u) < 32u)

// -----------------------------------------------------------------------------
// Test if a character is a reserved character
#define utf16_chr_is_rsrv(c) (((c) & 0xFFFEu) == 0xFFFEu)

// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-16 character decodes to.
// Test for a surrogate UTF-16 byte first.
// -----------------------------------------------------------------------------

#define utf16_chr_is_lead1(c) chr_is_ascii (c)
#define utf16_chr_is_lead2(c) ((c) < 0x800u)
#define utf16_chr_is_lead3(c) ((c) >= 0x800u)

// -----------------------------------------------------------------------------
// Construct a UTF-16 surrogate pair from a UTF-32 character
// -----------------------------------------------------------------------------

#define utf16_make_surr_high(c) (((c) >> 10) + UTF16_LEAD_OFFSET)
#define utf16_make_surr_low(c) (((c) & 0x3FFu) | UTF16_LOW_SURR_BEG)

// -----------------------------------------------------------------------------
// Construct a UTF-32 character from a UTF-16 surrogate pair
#define utf16_surr_to_chr(hi, lo) ((((uf32)((hi) & 0x3FFu) << 10) + 0x10000u) | ((lo) & 0x3FFu))

// -----------------------------------------------------------------------------
// UTF-32 character tests
// -----------------------------------------------------------------------------
// UTF-8 multibyte sequence length that a given UTF-32 character decodes to
// -----------------------------------------------------------------------------

#define utf32_chr_is_lead1(c) chr_is_ascii (c)
#define utf32_chr_is_lead2(c) utf16_chr_is_lead2 (c)
#define utf32_chr_is_lead3(c) ((c) < 0x10000u)
#define utf32_chr_is_lead4(c) ((c) < 0x110000u)

// -----------------------------------------------------------------------------
// Test if a given UTF-32 character decodes into UTF-16 surrogate pair
#define utf32_chr_is_surr(c) ((c) >= 0x10000u)

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
    if (utf8_byte_is_lead (*buf)) return (u8*)buf;
    buf++;
  }

  return null;
}

static inline u8* utf8_str_rsync (const u8* buf, const u8* beg)
{
  while (buf >= beg)
  {
    if (utf8_byte_is_lead (*buf)) return (u8*)buf;
    buf--;
  }

  return null;
}

// -----------------------------------------------------------------------------
// Versions for implicit strings
// -----------------------------------------------------------------------------

static inline u8* utf8_istr_sync (const u8* str)
{
  while (true)
  {
    if (utf8_byte_is_lead (*str)) return (u8*)str;
    str++;
  }

  assume_unreachable();
}

#define utf8_istr_rsync(str) utf8_str_rsync (str)

// -----------------------------------------------------------------------------
// `ptr` must be synchronized
// -----------------------------------------------------------------------------

static inline u8* utf8_chr_next (const u8* ptr)
{
  register uint b = *ptr;

  if (utf8_byte_is_lead1 (b)) return (u8*)(ptr + 1u);

#if CPU(X86)
  register uint n = bsr32 (~b << 24);

  if (unlikely (n > 4u)) return null;

  return (u8*)(ptr + n);
#else
  if (utf8_byte_is_lead2 (b)) return (u8*)(ptr + 2u);
  if (utf8_byte_is_lead3 (b)) return (u8*)(ptr + 3u);
  if (utf8_byte_is_lead4 (b)) return (u8*)(ptr + 4u);

  return null;
#endif
}

// -----------------------------------------------------------------------------

static inline u8* utf8_str_seek (const u8* buf, const u8* end)
{
  const u8* ptr = utf8_chr_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return (u8*)ptr;
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

  return (u16*)buf;
}

static inline u16* utf16_str_rsync (const u16* buf, const u16* beg)
{
  if (utf16_byte_is_surr_low (*buf))
  {
    if (unlikely (buf == beg)) return null;
    buf--;
  }

  return (u16*)buf;
}

// -----------------------------------------------------------------------------

static inline u16* utf16_istr_sync (const u16* str)
{
  if (utf16_byte_is_surr_low (*str)) return (u16*)(str + 1u);
  return (u16*)str;
}

#define utf16_istr_rsync(str) utf16_str_rsync (str)

// -----------------------------------------------------------------------------

static inline u16* utf16_chr_next (const u16* ptr)
{
  register uint b = *ptr;

  if (!utf16_byte_is_surr (b)) return (u16*)(ptr + 1u);
  if (utf16_byte_is_surr_high (b)) return (u16*)(ptr + 2u);

  return null;
}

// -----------------------------------------------------------------------------

static inline u16* utf16_str_seek (const u16* buf, const u16* end)
{
  const u16* ptr = utf16_chr_next (buf);

  if (unlikely (ptr == null)) return null;
  if (unlikely (ptr >= end)) return null;

  return (u16*)ptr;
}

static inline u16* utf16_str_rseek (const u16* buf, const u16* beg)
{
  utf16_str_rsync (buf - 1u, beg);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_istr_seek (const u16* str)
{
  utf16_istr_sync (str + 1u);
}

#define utf16_istr_rseek(str) utf16_str_rseek (str)

// -----------------------------------------------------------------------------
// Number of bytes in an implicit UTF string
// -----------------------------------------------------------------------------

extern size_t utf8_istr_length (const u8* str);
extern size_t utf16_istr_length (const u16* str);
extern size_t utf32_istr_length (const u32* str);

// -----------------------------------------------------------------------------
// Validation and number of characters in a UTF string
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

extern int utf8_str_valid (const u8* buf, size_t len
, u8** end, size_t* num);

extern int utf8_str_runes (const u8* buf, size_t len
, u8** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf8_istr_valid (const u8* str
, u8** end, size_t* num);

extern int utf8_istr_runes (const u8* str
, u8** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

extern int utf16_str_valid (const u16* buf, size_t len
, u16** end, size_t* num);

extern int utf16_str_runes (const u16* buf, size_t len
, u16** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf16_istr_valid (const u16* str
, u16** end, size_t* num);

extern int utf16_istr_runes (const u16* str
, u16** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-32
// -----------------------------------------------------------------------------

extern int utf32_str_valid (const u32* buf
, size_t len, u32** end);

extern int utf32_istr_valid (const u32* str
, u32** end);

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-8 to UTF-16
// -----------------------------------------------------------------------------

extern int utf8_str_to16 (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_str_to16_fast (const u8* restrict in, size_t len
, u16* restrict out, size_t size
, u8** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf8_istr_to16 (const u8* restrict in
, u16* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_istr_to16_fast (const u8* restrict in
, u16* restrict out, size_t size
, u8** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-8 to UTF-32
// -----------------------------------------------------------------------------

extern int utf8_str_to32 (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_str_to32_fast (const u8* restrict in, size_t len
, u32* restrict out, size_t size
, u8** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf8_istr_to32 (const u8* restrict in
, u32* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_istr_to32_fast (const u8* restrict in
, u32* restrict out, size_t size
, u8** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-8
// -----------------------------------------------------------------------------

extern int utf16_str_to8 (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** end, size_t* num);

extern int utf16_str_to8_fast (const u16* restrict in, size_t len
, u8* restrict out, size_t size
, u16** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf16_istr_to8 (const u16* restrict in
, u8* restrict out, size_t size
, u16** end, size_t* num);

extern int utf16_istr_to8_fast (const u16* restrict in
, u8* restrict out, size_t size
, u16** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-32
// -----------------------------------------------------------------------------

extern int utf16_str_to32 (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** end, size_t* num);

extern int utf16_str_to32_fast (const u16* restrict in, size_t len
, u32* restrict out, size_t size
, u16** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf16_istr_to32 (const u16* restrict in
, u32* restrict out, size_t size
, u16** end, size_t* num);

extern int utf16_istr_to32_fast (const u16* restrict in
, u32* restrict out, size_t size
, u16** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-8
// -----------------------------------------------------------------------------

extern int utf32_str_to8 (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** end, size_t* num);

extern int utf32_str_to8_fast (const u32* restrict in, size_t len
, u8* restrict out, size_t size
, u32** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf32_istr_to8 (const u32* restrict in
, u8* restrict out, size_t size
, u32** end, size_t* num);

extern int utf32_istr_to8_fast (const u32* restrict in
, u8* restrict out, size_t size
, u32** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-16
// -----------------------------------------------------------------------------

extern int utf32_str_to16 (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** end, size_t* num);

extern int utf32_str_to16_fast (const u32* restrict in, size_t len
, u16* restrict out, size_t size
, u32** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf32_istr_to16 (const u32* restrict in
, u16* restrict out, size_t size
, u32** end, size_t* num);

extern int utf32_istr_to16_fast (const u32* restrict in
, u16* restrict out, size_t size
, u32** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF byte order swapping
// -----------------------------------------------------------------------------
// UTF-16LE to UTF-16BE and vice versa
// -----------------------------------------------------------------------------

extern void utf16_str_bswap (u16* buf, size_t len);
extern u16* utf16_istr_bswap (u16* str);

// -----------------------------------------------------------------------------
// UTF-32LE to UTF-32BE and vice versa
// -----------------------------------------------------------------------------

extern void utf32_str_bswap (u32* buf, size_t len);
extern u32* utf32_istr_bswap (u32* str);

// -----------------------------------------------------------------------------

#endif
