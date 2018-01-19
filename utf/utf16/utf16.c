// =============================================================================
// <utf/utf16/utf16.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/intro.h>

// -----------------------------------------------------------------------------

{
  const u16* i = in;
  T_size_t pts = 0;

#if T(EXPLICIT)
  const u16* e = in + len;
#endif

#if T(UTF16_COUNT_SIMD)
  // Include the SIMD code path
  #include T_UTF16_COUNT_SIMD
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

      pts++;

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
      if (unlikely ((i + 2u) > e)) goto too_short;
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

      pts++;

      i += 2u;
    }

#if T(VALID)
    else goto invalid;
#endif

#if T(UTF16_COUNT_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

done:
  *end = (u16*)i;
  *num = pts;

  return 0;

invalid:
  *end = (u16*)i;
  *num = pts;

  return INT_MIN;

#if T(EXPLICIT)
too_short:
  *end = (u16*)i;
  *num = pts;

  return -1;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID

// -----------------------------------------------------------------------------

#include <quantum/structures/string/outro.h>
