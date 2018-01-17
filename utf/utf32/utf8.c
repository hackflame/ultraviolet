// =============================================================================
// <utf/utf32/utf8.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include "../intro.h"

// -----------------------------------------------------------------------------

{
  const u32* restrict i = in;

#if T(EXPLICIT)
  const u32* restrict e = in + len;
#endif

#if T(SIZE)
  T_size_t sz = 0;
#else
  u8* restrict o = out;
  u8* restrict m = out + size;
  int n;
#endif

#if T(UTF32_TO8_SIMD)
  // Include the SIMD code path
  #include T_UTF32_TO8_SIMD
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uf32 c = *i;

    // One ASCII byte
    if (likely (utf32_chr_is_lead1 (c)))
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
    }

    // Two UTF-8 bytes
    else if (likely (utf32_chr_is_lead2 (c)))
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
    }

    // Three UTF-8 bytes
    else if (likely (utf32_chr_is_lead3 (c)))
    {
#if T(VALID)
      // Check for UTF-16 surrogate character
      if (unlikely (utf16_byte_is_surr (c))) goto invalid;

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
    }

    // Four UTF-8 bytes
#if T(VALID)
    else if (unlikely (utf32_chr_is_lead4 (c)))
#else
    else
#endif
    {
#if T(VALID)
      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
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
      o[0] = utf8_chr_get_lead4 (c);
      o[1] = utf8_chr_get_trail3 (c);
      o[2] = utf8_chr_get_trail2 (c);
      o[3] = utf8_chr_get_trail1 (c);

      o += 4u;
#endif
    }

#if T(VALID)
    // Invalid character
    else
    {
invalid:
      *end = (u32*)i;

  #if T(SIZE)
      *num = sz;
  #else
      *num = (T_size_t)(o - out);
  #endif

      return INT_MIN;
    }
#endif

    i++;

#if T(UTF32_TO8_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

  *end = (u32*)i;

#if T(SIZE)
  *num = sz;
#else
  *num = (T_size_t)(o - out);
#endif

  return 0;

#if !T(SIZE)
need_space:
  *end = (u32*)i;
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
