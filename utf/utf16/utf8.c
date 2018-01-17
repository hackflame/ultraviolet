// =============================================================================
// <utf/utf16/utf8.c>
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
  u8* restrict o = out;
  u8* restrict m = out + size;
  int n;
#endif

#if T(UTF16_TO8_SIMD)
  // Include the SIMD code path
  #include T_UTF16_TO8_SIMD
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uint c = *i;

    // One ASCII byte
    if (likely (utf16_chr_is_lead1 (c)))
    {
#if T(SIZE)
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
        n = 1;
        goto need_space;
      }

      *o = c;
      o++;
#endif

#if !T(EXPLICIT)
      if (unlikely (c == (uint)'\0')) break;
#endif

      i++;
    }

    // Two UTF-8 bytes
    else if (likely (utf16_chr_is_lead2 (c)))
    {
#if T(SIZE)
      sz += 2u;
#else
      if (unlikely ((o + 2u) > m))
      {
        n = (o + 2u) - m;
        goto need_space;
      }

      // Compose the 2-byte UTF-8 codepoint
      o[0] = utf8_chr_get_lead2 (c);
      o[1] = utf8_chr_get_trail1 (c);

      o += 2u;
#endif

      i++;
    }

    // Three UTF-8 bytes
    else if (likely (!utf16_byte_is_surr (c)))
    {
#if T(VALID)
      // Check for Unicode non-character
      if (unlikely (utf16_chr_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if T(SIZE)
      sz += 3u;
#else
      if (unlikely ((o + 3u) > m))
      {
        n = (o + 3u) - m;
        goto need_space;
      }

      // Compose the 3-byte UTF-8 codepoint
      o[0] = utf8_chr_get_lead3 (c);
      o[1] = utf8_chr_get_trail2 (c);
      o[2] = utf8_chr_get_trail1 (c);

      o += 3u;
#endif

      i++;
    }

    // Four UTF-8 bytes
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

#if T(SIZE)
      sz += 4u;
#else
      if (unlikely ((o + 4u) > m))
      {
        n = (o + 4u) - m;
        goto need_space;
      }

      // Compose the 4-byte UTF-8 codepoint
      o[0] = utf8_chr_get_lead4 (w);
      o[1] = utf8_chr_get_trail3 (w);
      o[2] = utf8_chr_get_trail2 (w);
      o[3] = utf8_chr_get_trail1 (w);

      o += 4u;
#endif

      i += 2u;
    }

#if T(VALID)
    else goto invalid;
#endif

#if T(UTF16_TO8_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

done:
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

too_short:
  *end = (u16*)i;

#if T(SIZE)
  *num = sz;
#else
  *num = (T_size_t)(o - out);
#endif

  return -1;

#if !T(SIZE)
need_space:
  *end = (u16*)i;
  *num = (T_size_t)(o - out);

  return n;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
#undef T_SIZE

// -----------------------------------------------------------------------------

#include "../outro.h"
