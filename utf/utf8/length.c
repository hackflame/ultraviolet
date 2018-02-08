// =============================================================================
// <utf/utf8/length.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/define.h>

// -----------------------------------------------------------------------------

{
  const u8* s = str;

#if T(UTF8_LENGTH_SIMD)
  #include T_UTF8_LENGTH_SIMD
#elif CPU(64BIT)
  utf_align (u8, T_size_t, s, str, 8u);

  while (true)
  {
    if (unlikely (swar_has_zero8 (*(const u64*)s))) break;
    s += 8u;
  }

  utf_length (T_size_t, s, str);
#else
  utf_align (u8, T_size_t, s, str, 4u);

  while (true)
  {
    if (unlikely (swar_has_zero4 (*(const u32*)s))) break;
    s += 4u;
  }

  utf_length (T_size_t, s, str);
#endif
}

// -----------------------------------------------------------------------------

#include <quantum/structures/string/undef.h>
