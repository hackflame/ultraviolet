// =============================================================================
// <utf/utf32/bswap.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/intro.h>

// -----------------------------------------------------------------------------

{
  u32* s = str;

#if T(EXPLICIT)
  const u32* e = str + len;
#endif

#if T(UTF32_BSWAP_SIMD)
  #include T_UTF32_BSWAP_SIMD
#endif

#if T(EXPLICIT)
  while (s != e)
#else
  while (true)
#endif
  {
    register u32 c = *s;

#if !T(EXPLICIT)
    if (unlikely (c == (uint)'\0')) break;
#endif

    *s = bswap32 (c);
    s++;
  }

done:;
#if !T(EXPLICIT)
  return s;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT

// -----------------------------------------------------------------------------

#include <quantum/structures/string/outro.h>
