// =============================================================================
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

#include <quantum/bitops.h>
#include <quantum/unicode.h>

// =============================================================================
// Constants
// -----------------------------------------------------------------------------
// Maximum sequence lengths
// -----------------------------------------------------------------------------

#define UTF8_SEQ_LEN_MAX  4u
#define UTF16_SEQ_LEN_MAX 2u

// -----------------------------------------------------------------------------
// http://unicode.org/faq/utf_bom.html
// -----------------------------------------------------------------------------

#define UTF16_HI_SURROGATE_START 0xD800u
#define UTF16_HI_SURROGATE_END   0xDBFFu

#define UTF16_LO_SURROGATE_START 0xDC00u
#define UTF16_LO_SURROGATE_END   0xDFFFu

#define UTF16_LEAD_OFFSET (UTF16_HI_SURROGATE_START - (0x10000u >> 10))
#define UTF16_SURROGATE_OFFSET (0x10000u - ((uf32)UTF16_HI_SURROGATE_START << 10) - UTF16_LO_SURROGATE_START)

// =============================================================================
// Macros
// -----------------------------------------------------------------------------
// UTF-8 character tests
// -----------------------------------------------------------------------------
// Leading byte and length of the UTF-8 multibyte sequence that it represents
#define utf8_chr_is_head1(c) ((c) < 0x80u)
#define utf8_chr_is_head2(c) (((c) & 0xE0u) == 0xC0u)
#define utf8_chr_is_head3(c) (((c) & 0xF0u) == 0xE0u)
#define utf8_chr_is_head4(c) (((c) & 0xF8u) == 0xF0u)

// -----------------------------------------------------------------------------
// UTF-8 byte kind
#define utf8_chr_is_ascii utf8_chr_is_head1
#define utf8_chr_is_head(c) (utf8_chr_is_ascii (c) || (((c) & 0xC0u) == 0xC0u))
#define utf8_chr_is_tail(c) (((c) & 0xC0u) == 0x80u)

// -----------------------------------------------------------------------------
// UTF-8 codepoint composition / decomposition
// -----------------------------------------------------------------------------

#define utf8_chr_bits(c) ((c) & 0x3Fu)

// -----------------------------------------------------------------------------
// Compose the leading (or head) byte
#define utf8_codep_comp_head4(c) ((uf32)((c) & 0x7u) << 18)
#define utf8_codep_comp_head3(c) (((c) & 0xFu) << 12)
#define utf8_codep_comp_head2(c) (((c) & 0x1Fu) << 6)

// -----------------------------------------------------------------------------
// Compose the continuation (or tail, suffix, or trailing) bytes
#define utf8_codep_comp_tail3(c) (utf8_chr_bits (c) << 12)
#define utf8_codep_comp_tail2(c) (utf8_chr_bits (c) << 6)
#define utf8_codep_comp_tail1(c) utf8_chr_bits (c)

// -----------------------------------------------------------------------------
// Compose the whole UTF-32 or UTF-16 codepoint
#define utf8_codep_comp4(c4, c3, c2, c1) (utf8_codep_comp_head4 (c4) | utf8_codep_comp_tail3 (c3) | utf8_codep_comp_tail2 (c2) | utf8_codep_comp_tail1 (c1))
#define utf8_codep_comp3(c3, c2, c1) (utf8_codep_comp_head3 (c3) | utf8_codep_comp_tail2 (c2) | utf8_codep_comp_tail1 (c1))
#define utf8_codep_comp2(c2, c1) (utf8_codep_comp_head2 (c2) | utf8_codep_comp_tail1 (c1))

// -----------------------------------------------------------------------------
// Decompose the leading byte
#if 1
  #define utf8_codep_decomp_head4(c) (((c) >> 18) | 0xF0u)
  #define utf8_codep_decomp_head3(c) (((c) >> 12) | 0xE0u)
  #define utf8_codep_decomp_head2(c) (((c) >> 6) | 0xC0u)
#else
  #define utf8_codep_decomp_head4(c) ((((c) & 0x1C0000u) >> 18) | 0xF0u)
  #define utf8_codep_decomp_head3(c) ((((c) & 0xF000u) >> 12) | 0xE0u)
  #define utf8_codep_decomp_head2(c) ((((c) & 0x7C0u) >> 6) | 0xC0u)
#endif

// -----------------------------------------------------------------------------
// Decompose the continuation bytes
#define utf8_codep_decomp_tail3(c) (utf8_chr_bits ((c) >> 12) | 0x80u)
#define utf8_codep_decomp_tail2(c) (utf8_chr_bits ((c) >> 6) | 0x80u)
#define utf8_codep_decomp_tail1(c) (utf8_chr_bits (c) | 0x80u)

// -----------------------------------------------------------------------------
// UTF-16 character tests
// -----------------------------------------------------------------------------
// Test if a character is a surrogate character
#if 1
  #define utf16_chr_is_surr(c) (((c) & 0xF800u) == 0xD800u)
#else
  #define utf16_chr_is_surr(c) (((c) >= UTF16_HI_SURROGATE_START) && ((c) <= UTF16_LO_SURROGATE_END))
#endif

// -----------------------------------------------------------------------------
// Test if a character is a high surrogate character
#if 1
  #define utf16_chr_is_surr_high(c) (((c) & 0xFC00u) == 0xD800u)
#else
  #define utf16_chr_is_surr_high(c) (((c) >= UTF16_HI_SURROGATE_START) && ((c) <= UTF16_HI_SURROGATE_END))
#endif

// -----------------------------------------------------------------------------
// Test if a character is a low surrogate character
#if 1
  #define utf16_chr_is_surr_low(c) (((c) & 0xFC00u) == 0xDC00u)
#else
  #define utf16_chr_is_surr_low(c) (((c) >= UTF16_LO_SURROGATE_START) && ((c) <= UTF16_LO_SURROGATE_END))
#endif

// -----------------------------------------------------------------------------
// Test if a character is a non-character
#define utf16_chr_is_non(c) (((c) - 0xFDD0u) < 32u)

// -----------------------------------------------------------------------------
// Test if a character is a reserved character
#define utf16_chr_is_rsrv(c) (((c) & 0xFFFEu) == 0xFFFEu)

// -----------------------------------------------------------------------------
// Number of UTF-8 sequence characters that a given UTF-8 character decodes to
#define utf16_chr_is8_lead1(c) utf8_chr_is_ascii (c)
#define utf16_chr_is8_lead2(c) ((c) < 0x800u)
// Test for a surrogate UTF-16 character first
#define utf16_chr_is8_lead3(c) ((c) >= 0x800u)

// -----------------------------------------------------------------------------
// Construct UTF-16 surrogate pair from a UTF-32 character
#define utf16_surr_make_high(c) (((c) >> 10) + UTF16_LEAD_OFFSET)
#define utf16_surr_make_low(c) (((c) & 0x3FFu) | UTF16_LO_SURROGATE_START)

// -----------------------------------------------------------------------------
// Construct a UTF-32 character from a UTF-16 surrogate pair
#define utf16_surr_to32(hi, lo) ((((uf32)((hi) & 0x3FFu) << 10) + 0x10000u) | ((lo) & 0x3FFu))

// -----------------------------------------------------------------------------
// UTF-32 character tests
// -----------------------------------------------------------------------------
// Number of UTF-8 sequence characters that a given UTF-32 character decodes to
#define utf32_chr_is8_lead1(c) utf8_chr_is_ascii (c)
#define utf32_chr_is8_lead2(c) utf16_chr_is8_lead2 (c)
#define utf32_chr_is8_lead3(c) ((c) < 0x10000u)
#define utf32_chr_is8_lead4(c) ((c) < 0x110000u)

// -----------------------------------------------------------------------------
// Number of UTF-16 characters that a given UTF-32 character decodes to
#define utf32_chr_is16_chr(c) utf32_chr_is8_lead3 (c)
#define utf32_chr_is16_surr(c) ((c) >= 0x10000u)

// =============================================================================
// Functions
// -----------------------------------------------------------------------------
// Navigation
// -----------------------------------------------------------------------------
// UTF-8
// -----------------------------------------------------------------------------

static inline u8* utf8_ptr_sync (const u8* ptr, const u8* end)
{
  while (ptr != end)
  {
    if (utf8_chr_is_head (*ptr)) return (u8*)ptr;
    ptr++;
  }

  return null;
}

static inline u8* utf8_ptr_rsync (const u8* ptr, const u8* buf)
{
  while (ptr >= buf)
  {
    if (utf8_chr_is_head (*ptr)) return (u8*)ptr;
    ptr--;
  }

  return null;
}

// -----------------------------------------------------------------------------

static inline u8* utf8_ptri_sync (const u8* ptr)
{
  while (true)
  {
    if (utf8_chr_is_head (*ptr)) return (u8*)ptr;
    ptr++;
  }

  assume_unreachable();
}

#define utf8_ptri_rsync utf8_ptr_rsync

// -----------------------------------------------------------------------------

static inline u8* utf8_ptr_seek (const u8* ptr, const u8* end)
{
  register uint c = *ptr;

  if (utf8_chr_is_head1 (c)) return (u8*)(ptr + 1);

#if CPU(X86)
  register uint n = bsr32 (~c << 24);

  if (unlikely (n > 4u)) return null;

  ptr += n;
#else
  if (utf8_chr_is_head2 (c)) ptr += 2;
  else if (utf8_chr_is_head3 (c)) ptr += 3;
  else if (utf8_chr_is_head4 (c)) ptr += 4;
  else return null;
#endif

  if (unlikely (ptr > end)) return null;

  if (likely (ptr != end)) return utf8_chr_is_tail (*ptr) ? null : (u8*)ptr;

  return (u8*)ptr;
}

static inline u8* utf8_ptr_rseek (const u8* ptr, const u8* buf)
{
  return utf8_ptr_rsync (ptr - 1, buf);
}

// -----------------------------------------------------------------------------

static inline u8* utf8_ptri_seek (const u8* ptr)
{
  return utf8_ptri_sync (ptr + 1);
}

#define utf8_ptri_rseek utf8_ptr_rseek

// -----------------------------------------------------------------------------
// UTF-16
// -----------------------------------------------------------------------------

static inline u16* utf16_ptr_sync (const u16* ptr, const u16* end)
{
  if (utf16_chr_is_surr_low (*ptr))
  {
    ptr++;

    if (unlikely (ptr == end)) return null;

    return utf16_chr_is_surr_low (*ptr) ? null : (u16*)ptr;
  }

  return (u16*)ptr;
}

static inline u16* utf16_ptr_rsync (const u16* ptr, const u16* buf)
{
  if (utf16_chr_is_surr_low (*ptr))
  {
    if (unlikely (ptr == buf)) return null;

    ptr--;

    return utf16_chr_is_surr_low (*ptr) ? null : (u16*)ptr;
  }

  return (u16*)ptr;
}

// -----------------------------------------------------------------------------

static inline u16* utf16_ptri_sync (const u16* ptr)
{
  if (utf16_chr_is_surr_low (*ptr))
  {
    ptr++;

    if (unlikely (*ptr == '\0')) return null;

    return utf16_chr_is_surr_low (*ptr) ? null : (u16*)ptr;
  }

  return (u16*)ptr;
}

#define utf16_ptri_rsync utf16_ptr_rsync

// -----------------------------------------------------------------------------

static inline u16* utf16_ptr_seek (const u16* ptr, const u16* end)
{
  if (utf16_chr_is_surr_high (*ptr))
  {
    ptr += 2;

    if (unlikely (ptr > end)) return null;
  }
  else ptr++;

  if (unlikely (ptr != end))
  {
    return utf16_chr_is_surr_low (*ptr) ? null : (u16*)ptr;
  }

  return (u16*)ptr;
}

static inline u16* utf16_ptr_rseek (const u16* ptr, const u16* buf)
{
  utf16_ptr_rsync (ptr - 1, buf);
}

// -----------------------------------------------------------------------------

static inline u16* utf16_ptri_seek (const u16* ptr)
{
  utf16_ptri_sync (ptr + 1);
}

#define utf16_ptri_rseek utf16_ptr_rseek

// -----------------------------------------------------------------------------
// Number of characters in an implicit UTF string
// -----------------------------------------------------------------------------

extern size_t utf8_stri_length (const u8* str);
extern size_t utf16_stri_length (const u16* str);
extern size_t utf32_stri_length (const u32* str);

// -----------------------------------------------------------------------------
// Validation and number of codepoints in a UTF string
// -----------------------------------------------------------------------------

extern int utf8_str_valid (const u8* in, size_t len, u8** end, size_t* num);
extern int utf8_str_runes (const u8* in, size_t len, u8** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf8_stri_valid (const u8* in, u8** end, size_t* num);
extern int utf8_stri_runes (const u8* in, u8** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf16_str_valid (const u16* in, size_t len, u16** end, size_t* num);
extern int utf16_str_runes (const u16* in, size_t len, u16** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf16_stri_valid (const u16* in, u16** end, size_t* num);
extern int utf16_stri_runes (const u16* in, u16** end, size_t* num);

// -----------------------------------------------------------------------------

extern int utf32_str_valid (const u32* in, size_t len, u32** end);
extern int utf32_stri_valid (const u32* in, u32** end);

// -----------------------------------------------------------------------------
// Conversion
// -----------------------------------------------------------------------------
// UTF-8 to UTF-16 conversion
// -----------------------------------------------------------------------------

extern int utf8_str_to16 (const u8* restrict in, size_t len, u16* restrict out
, size_t size, u8** end, size_t* num);

extern int utf8_str_to16_fast (const u8* restrict in, size_t len
, u16* restrict out, size_t size, u8** end, size_t* num);

extern int utf8_stri_to16 (const u8* restrict in, u16* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_stri_to16_fast (const u8* restrict in, u16* restrict out
, size_t size, u8** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-8 to UTF-32 conversion
// -----------------------------------------------------------------------------

extern int utf8_str_to32 (const u8* restrict in, size_t len, u32* restrict out
, size_t size, u8** end, size_t* num);

extern int utf8_str_to32_fast (const u8* restrict in, size_t len
, u32* restrict out, size_t size, u8** end, size_t* num);

extern int utf8_stri_to32 (const u8* restrict in, u32* restrict out, size_t size
, u8** end, size_t* num);

extern int utf8_stri_to32_fast (const u8* restrict in, u32* restrict out
, size_t size, u8** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-8 conversion
// -----------------------------------------------------------------------------

extern int utf16_str_to8 (const u16* restrict in, size_t len, u8* restrict out
, size_t size, u16** end, size_t* num);

extern int utf16_str_to8_fast (const u16* restrict in, size_t len
, u8* restrict out, size_t size, u16** end, size_t* num);

extern int utf16_stri_to8 (const u16* restrict in, u8* restrict out, size_t size
, u16** end, size_t* num);

extern int utf16_stri_to8_fast (const u16* restrict in, u8* restrict out
, size_t size, u16** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-16 to UTF-32 conversion
// -----------------------------------------------------------------------------

extern int utf16_str_to32 (const u16* restrict in, size_t len, u32* restrict out
, size_t size, u16** end, size_t* num);

extern int utf16_str_to32_fast (const u16* restrict in, size_t len
, u32* restrict out, size_t size, u16** end, size_t* num);

extern int utf16_stri_to32 (const u16* restrict in, u32* restrict out
, size_t size, u16** end, size_t* num);

extern int utf16_stri_to32_fast (const u16* restrict in, u32* restrict out
, size_t size, u16** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-8 conversion
// -----------------------------------------------------------------------------

extern int utf32_str_to8 (const u32* restrict in, size_t len, u8* restrict out
, size_t size, u32** end, size_t* num);

extern int utf32_str_to8_fast (const u32* restrict in, size_t len
, u8* restrict out, size_t size, u32** end, size_t* num);

extern int utf32_stri_to8 (const u32* restrict in, u8* restrict out, size_t size
, u32** end, size_t* num);

extern int utf32_stri_to8_fast (const u32* restrict in, u8* restrict out
, size_t size, u32** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF-32 to UTF-16 conversion
// -----------------------------------------------------------------------------

extern int utf32_str_to16 (const u32* restrict in, size_t len, u16* restrict out
, size_t size, u32** end, size_t* num);

extern int utf32_str_to16_fast (const u32* restrict in, size_t len
, u16* restrict out, size_t size, u32** end, size_t* num);

extern int utf32_stri_to16 (const u32* restrict in, u16* restrict out
, size_t size, u32** end, size_t* num);

extern int utf32_stri_to16_fast (const u32* restrict in, u16* restrict out
, size_t size, u32** end, size_t* num);

// -----------------------------------------------------------------------------
// UTF byte order swapping
// -----------------------------------------------------------------------------
// UTF-16LE to UTF-16BE and vice versa
// -----------------------------------------------------------------------------

extern void utf16_str_bswap (u16* str, size_t len);
extern u16* utf16_stri_bswap (u16* str);

// -----------------------------------------------------------------------------
// UTF-32LE to UTF-32BE and vice versa
// -----------------------------------------------------------------------------

extern void utf32_str_bswap (u32* str, size_t len);
extern u32* utf32_stri_bswap (u32* str);

// -----------------------------------------------------------------------------

#endif
