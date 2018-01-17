// =============================================================================
// <utf/utf32/length.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include "../intro.h"

// -----------------------------------------------------------------------------

{
  const u32* s = str;

#if T(UTF32_LENGTH_SIMD)
  #include T_UTF32_LENGTH_SIMD
#elif CPU(64BIT)
  utf_align (u32, T_size_t, s, str, 8u);

  while (true)
  {
    u64 v = *(u64*)s;

    if (unlikely (((v - 0x0000000100000001u) & ~v & 0x8000000080000000u) != 0)) break;

    s += 2u;
  }

  utf_length (T_size_t, s, str);
#else
  utf_length (T_size_t, s, str);
#endif
}

// -----------------------------------------------------------------------------

#include "../outro.h"
