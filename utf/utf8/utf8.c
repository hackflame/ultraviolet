// =============================================================================
// <utf/utf8/utf8.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/structures/string/intro.h>

// -----------------------------------------------------------------------------

{
  const u8* i = in;
  T_size_t pts = 0;

#if T(EXPLICIT)
  const u8* e = in + len;
  int t;
#endif

#if T(UTF8_COUNT_SIMD)
  // Include the SIMD code path
  #include T_UTF8_COUNT_SIMD
#endif

#if CPU(X86) && T(EXPLICIT) && !T(VALID)
  // It is possible to use the `bsr` instruction
  // to count UTF-8 characters if validation is not requested
  while (i < e)
  {
    register uint c = *i;
    register u32 n = bsr32 (~c << 24);

    i += utf8_byte_is_lead1 (c) ? 1u : n;
    pts++;

  #if T(UTF8_COUNT_SIMD)
    if (likely (i >= r)) goto simd;
  #endif
  }

  goto done;
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uint c = *i;

    // Single ASCII byte
    if (likely (utf8_byte_is_lead1 (c)))
    {
      pts++;

#if !T(EXPLICIT)
      if (unlikely (c == (uint)'\0')) break;
#endif

      i++;
    }

    // Multibyte sequence
    else
    {
      u32 w;

      // Two UTF-8 bytes
      if (likely (utf8_byte_is_lead2 (c)))
      {
#if T(EXPLICIT)
        // Check if the input ends abruptly
        if (unlikely ((i + 2u) > e))
        {
          t = -1;
          goto too_short;
        }
#endif

        uint c1 = i[1];

#if T(VALID)
        // Check if the input byte is a UTF-8 suffix byte
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
#elif !T(EXPLICIT)
        // Invalid sequence
        if (unlikely (c1 == (uint)'\0')) goto invalid;
#endif

        // Compose the UTF-32 codepoint from two UTF-8 bytes
        w = utf8_chr_make2 (c, c1);

#if T(VALID)
        // Check for overlong sequence
        if (unlikely (utf32_chr_is_lead1 (w))) goto invalid;
#endif

        i += 2u;
      }

      // Three UTF-8 bytes
      else if (likely (utf8_byte_is_lead3 (c)))
      {
#if T(EXPLICIT)
        if (unlikely ((i + 3u) > e))
        {
          t = e - (i + 3u);
          goto too_short;
        }
#endif

#if CPU(UNALIGNED_ACCESS) && T(EXPLICIT) && INT(16BIT) && 0
        u16 cv = *(u16*)(i + 1u);

  #if T(VALID)
        if (unlikely ((cv & 0xC0C0u) != 0x8080u)) goto invalid;
  #endif

  #if CPU(LITTLE_ENDIAN)
        w = utf8_chr_set_lead3 (c) | ((cv & 0x3Fu) << 6) | ((cv & 0x3F00u) >> 8);
  #else
        w = utf8_chr_set_lead3 (c) | ((cv & 0x3F00u) >> 2) | (cv & 0x3Fu);
  #endif
#elif T(EXPLICIT) && INT(16BIT) && 1
        u16 c1 = i[1];
        u16 c2 = i[2];

  #if T(VALID)
        if (unlikely (((c1 | (c2 << 8)) & 0xC0C0u) != 0x8080u)) goto invalid;
  #endif

        w = utf8_chr_make3 (c, c1, c2);
#else
        uint c1 = i[1];

  #if T(VALID)
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c1 == (uint)'\0')) goto invalid;
  #endif

        uint c2 = i[2];

  #if T(VALID)
        if (unlikely (!utf8_byte_is_trail (c2))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c2 == (uint)'\0')) goto invalid;
  #endif

        // Compose the UTF-32 codepoint from three UTF-8 bytes
        w = utf8_chr_make3 (c, c1, c2);
#endif

#if T(VALID)
        // Check for overlong sequence
        if (unlikely (utf32_chr_is_lead2 (w))) goto invalid;

        // Check for UTF-16 surrogate character
        if (unlikely (utf16_byte_is_surr (w))) goto invalid;

        // Check for Unicode non-character
        if (unlikely (utf16_chr_is_non (w))) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

        i += 3u;
      }

      // Four UTF-8 bytes
#if T(VALID)
      else if (unlikely (utf8_byte_is_lead4 (c)))
#else
      else
#endif
      {
#if T(EXPLICIT)
        if (unlikely ((i + 4u) > e))
        {
          t = e - (i + 4u);
          goto too_short;
        }
#endif

#if CPU(UNALIGNED_ACCESS) && T(EXPLICIT) && INT(32BIT) && 0
        u32 cv = *(u32*)i;

  #if CPU(LITTLE_ENDIAN)
    #if T(VALID)
        if (unlikely ((cv & 0xC0C0C000u) != 0x80808000u)) goto invalid;
    #endif

        w = utf8_chr_set_lead4 (c) | ((cv & 0x3F00u) << 6) | ((cv & 0x3Fu) << 12) | ((cv & 0x3F0000u) >> 16);
  #else
    #if T(VALID)
        if (unlikely ((cv & 0xC0C0C0u) != 0x808080u)) goto invalid;
    #endif

        w = utf8_chr_set_lead4 (c) | ((cv & 0x3F0000u) >> 4) | ((cv & 0x3F00u) >> 2) | (cv & 0x3Fu);
  #endif
#elif T(EXPLICIT) && INT(32BIT) && 1
        u32 c1 = i[1];
        u32 c2 = i[2];
        u32 c3 = i[3];

  #if T(VALID)
        if (unlikely (((c1 | (c2 << 8) | (c3 << 16)) & 0xC0C0C0u) != 0x808080u)) goto invalid;
  #endif

        w = utf8_chr_make4 (c, c1, c2, c3);
#else
        uint c1 = i[1];

  #if T(VALID)
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c1 == (uint)'\0')) goto invalid;
  #endif

        uint c2 = i[2];

  #if T(VALID)
        if (unlikely (!utf8_byte_is_trail (c2))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c2 == (uint)'\0')) goto invalid;
  #endif

        uint c3 = i[3];

  #if T(VALID)
        if (unlikely (!utf8_byte_is_trail (c3))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c3 == (uint)'\0')) goto invalid;
  #endif

        // Compose the UTF-32 codepoint from four UTF-8 bytes
        w = utf8_chr_make4 (c, c1, c2, c3);
#endif

#if T(VALID)
        // Check if the character exceeds the maximum allowed Unicode codepoint
        if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

        i += 4u;
      }

#if T(VALID)
      else goto invalid;
#endif

      pts++;
    }

#if T(UTF8_COUNT_SIMD)
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

done:
  *end = (u8*)i;
  *num = pts;

  return 0;

invalid:
  *end = (u8*)i;
  *num = pts;

  return INT_MIN;

#if T(EXPLICIT)
too_short:
  *end = (u8*)i;
  *num = pts;

  return t;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID

// -----------------------------------------------------------------------------

#include <quantum/structures/string/outro.h>
