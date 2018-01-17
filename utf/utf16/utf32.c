// =============================================================================
// <utf/utf16/utf32.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include "../intro.h"

// -----------------------------------------------------------------------------

{
  const u16* restrict i = in;

#if T(EXPLICIT)
  const u16* restrict e = in + len;
#endif

#if T(SIZE)
  T_size_t sz = 0;
#else
  u32* restrict o = out;
  u32* restrict m = out + size;
#endif

#if T(UTF16_TO32_SIMD)
  // Include the SIMD code path
  #include T_UTF16_TO32_SIMD
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uint c = *i;

    // BMP character
    if (likely (!utf16_byte_is_surr (c)))
    {
#if T(VALID)
      // Check for Unicode non-character
      if (unlikely (utf16_chr_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if T(SIZE)
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
need_space:
        *end = (u16*)i;
        *num = (T_size_t)(o - out);

        return 1;
      }

      *o = c;
      o++;
#endif

#if !T(EXPLICIT)
      if (unlikely (c == (uint)'\0')) break;
#endif

      i++;
    }

    // Surrogate pair
#if T(VALID)
    else if (unlikely (utf16_byte_is_surr_high (c)))
#else
    else
#endif
    {
#if T(EXPLICIT)
      // Check if the input ends abruptly
      if (unlikely ((i + 2u) > e))
      {
too_short:
        *end = (u16*)i;

  #if T(SIZE)
        *num = sz;
  #else
        *num = (T_size_t)(o - out);
  #endif

        return -1;
      }
#endif

      // Get the low surrogate character
      uint cs = i[1];

#if T(VALID)
      // Check if it's actually a low surrogate
      if (unlikely (!utf16_byte_is_surr_low (cs))) goto invalid;
#elif !T(EXPLICIT)
      // Invalid sequence
      if (unlikely (cs == (uint)'\0')) goto invalid;
#endif

      // Compose the UTF-32 codepoint from the UTF-16 surrogate pair
      u32 w = utf16_surr_to_chr (c, cs);

#if T(VALID)
      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

#if T(SIZE)
      sz++;
#else
      if (unlikely (o == m)) goto need_space;

      *o = w;
      o++;
#endif

      i += 2u;
    }

#if T(VALID)
    else goto invalid;
#endif

#if T(UTF16_TO32_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

  *end = (u16*)i;

#if T(SIZE)
  *num = sz;
#else
  *num = (T_size_t)(o - out);
#endif

  return 0;

invalid:
  *end = (u16*)i;

#if T(SIZE)
  *num = sz;
#else
  *num = (T_size_t)(o - out);
#endif

  return INT_MIN;
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
#undef T_SIZE

// -----------------------------------------------------------------------------

#include "../outro.h"
