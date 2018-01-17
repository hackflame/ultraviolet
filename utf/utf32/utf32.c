// =============================================================================
// <utf/utf32/utf32.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include "../intro.h"

// -----------------------------------------------------------------------------

{
  const u32* i = in;

#if T(EXPLICIT)
  const u32* e = in + len;
#endif

#if T(UTF32_COUNT_SIMD)
  // Include the SIMD code path
  #include T_UTF32_COUNT_SIMD
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uf32 c = *i;

#if T(VALID)
    // Check for UTF-16 surrogate character
    if (unlikely (utf16_byte_is_surr (c)))
    {
invalid:
      *end = (u32*)i;
      return INT_MIN;
    }

    // Check for Unicode non-character
    if (unlikely (utf16_chr_is_non (c))) goto invalid;

    // Check for reserved Unicode character
    if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if !T(EXPLICIT)
    if (unlikely (c == (uint)'\0')) break;
#endif

    i++;

#if T(UTF32_COUNT_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

  *end = (u32*)i;

  return 0;
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID

// -----------------------------------------------------------------------------

#include "../outro.h"
