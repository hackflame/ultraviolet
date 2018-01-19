// =============================================================================
// <utf/utf32/utf16.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/intro.h>

// -----------------------------------------------------------------------------

{
  const u32* restrict i = in;

#if T(EXPLICIT)
  const u32* restrict e = in + len;
#endif

#if T(SIZE)
  T_size_t sz = 0;
#else
  u16* restrict o = out;
  u16* restrict m = out + size;
  int n;
#endif

#if T(UTF32_TO16_SIMD)
  // Include the SIMD code path
  #include T_UTF32_TO16_SIMD
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uf32 c = *i;

    // Single BMP character
    if (likely (!utf32_chr_is_surr (c)))
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

    // Surrogate pair
#if T(VALID)
    else if (likely (utf32_chr_is_surr (c)))
#else
    else
#endif
    {
#if T(VALID)
      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if T(SIZE)
      sz += 2u;
#else
      if (unlikely ((o + 2u) > m))
      {
        n = (o + 2u) - m;
        goto need_space;
      }

      // Compose the UTF-16 surrogate pair
      o[0] = utf16_make_surr_high (c);
      o[1] = utf16_make_surr_low (c);

      o += 2u;
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

#if T(UTF32_TO16_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

done:
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

#include <quantum/structures/string/outro.h>
