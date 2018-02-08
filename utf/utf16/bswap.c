// =============================================================================
// <utf/utf16/bswap.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/define.h>

// -----------------------------------------------------------------------------

{
  u16* s = str;

#if T(EXPLICIT)
  const u16* e = str + len;
#endif

#if T(UTF16_BSWAP_SIMD)
  #include T_UTF16_BSWAP_SIMD
#endif

#if T(EXPLICIT)
  while (s != e)
#else
  while (true)
#endif
  {
    register u16 c = *s;

#if !T(EXPLICIT)
    if (unlikely (c == (uint)'\0')) break;
#endif

    *s = bswap16 (c);
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

#include <quantum/structures/string/undef.h>
