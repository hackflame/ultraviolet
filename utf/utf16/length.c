// =============================================================================
// <utf/utf16/length.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/intro.h>

// -----------------------------------------------------------------------------

{
  const u16* s = str;

#if T(UTF16_LENGTH_SIMD)
  #include T_UTF16_LENGTH_SIMD
#elif CPU(64BIT)
  utf_align (u16, T_size_t, s, str, 8u);

  while (true)
  {
    u64 v = *(u64*)s;

    if (unlikely (((v - 0x0001000100010001u) & ~v & 0x8000800080008000u) != 0)) break;

    s += 4u;
  }

  utf_length (T_size_t, s, str);
#else
  utf_align (u16, T_size_t, s, str, 4u);

  while (true)
  {
    u32 v = *(u32*)s;

    if (unlikely (((v - 0x00010001u) & ~v & 0x80008000u) != 0)) break;

    s += 2u;
  }

  utf_length (T_size_t, s, str);
#endif
}

// -----------------------------------------------------------------------------

#include <quantum/structures/string/outro.h>
