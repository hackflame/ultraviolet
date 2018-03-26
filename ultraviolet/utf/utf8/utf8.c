// =============================================================================
// <ultraviolet/utf/utf8/utf8.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u8* i = in;
  size_t pts = 0;

#if T_EXPLICIT
  const u8* e = in + len;
  ptr_diff_t t;
#endif

#if T_UTF8_COUNT_SIMD
  // Include the SIMD code path
  #include "utf8/simd.c"
#endif

#if T_EXPLICIT && !T_VALID && CPU_X86
  // Use the `bsr` instruction to count UTF-8 characters
  while (i != e)
  {
    register u32 c = *i;
    register uint n = bsr32 (/*~c << 24*/bswap32 (c));

    n = utf8_byte_is_lead1 (c) ? 1u : n;

    if (unlikely ((i + n) > e))
    {
      t = e - (i + n);
      goto too_short;
    }

    i += n;
    pts++;

  #if T_UTF8_COUNT_SIMD
    if (likely (i >= r)) goto simd;
  #endif
  }

  goto done;
#else
#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u8f c = *i;

    // Single ASCII byte
    if (likely (utf8_byte_is_lead1 (c)))
    {
#if !T_EXPLICIT
      if (unlikely (c == '\0')) break;
#endif

      i++;
      pts++;
    }

    // Multibyte sequence
    else
    {
      // Two UTF-8 bytes
      if (likely (utf8_byte_is_lead2 (c)))
      {
#if T_EXPLICIT
        // Check if the input ends abruptly
        if (unlikely ((i + 2u) > e))
        {
          t = -1;
          goto too_short;
        }
#endif

        u16f c1 = i[1];
        unused(c1);

#if T_VALID
        // Check if the input byte is a UTF-8 suffix byte
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
#elif !T_EXPLICIT
        // Invalid sequence
        if (unlikely (c1 == '\0')) goto invalid;
#endif

#if T_VALID
        // Compose the UTF-32 codepoint from two UTF-8 bytes
        u16f w = utf8_char_make2 (c, c1);

        // Check for overlong sequence
        if (unlikely (utf32_char_is_lead1 (w))) goto invalid;
#endif

        i += 2u;
      }

      // Three UTF-8 bytes
      else if (likely (utf8_byte_is_lead3 (c)))
      {
#if T_EXPLICIT
        if (unlikely ((i + 3u) > e))
        {
          t = e - (i + 3u);
          goto too_short;
        }
#endif

#if T_EXPLICIT && T_VALID && INT_16BIT
        u16f c1 = i[1];
        u16f c2 = i[2];

        if (unlikely (((c1 | (c2 << 8)) & 0xC0C0u) != 0x8080u)) goto invalid;

        u16f w = utf8_char_make3 (c, c1, c2);
#else
        u16f c1 = i[1];
        unused(c1);

  #if T_VALID
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
  #elif !T_EXPLICIT
        if (unlikely (c1 == '\0')) goto invalid;
  #endif

        u16f c2 = i[2];
        unused(c2);

  #if T_VALID
        if (unlikely (!utf8_byte_is_trail (c2))) goto invalid;

        // Compose the UTF-32 codepoint from three UTF-8 bytes
        u16f w = utf8_char_make3 (c, c1, c2);
  #elif !T_EXPLICIT
        if (unlikely (c2 == '\0')) goto invalid;
  #endif
#endif

#if T_VALID
        // Check for overlong sequence
        if (unlikely (utf32_char_is_lead2 (w))) goto invalid;

        // Check for UTF-16 surrogate character
        if (unlikely (utf16_byte_is_surr (w))) goto invalid;

        // Check for Unicode non-character
        if (unlikely (utf16_char_is_non (w))) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_char_is_rsrv (w))) goto invalid;
#endif

        i += 3u;
      }

      // Four UTF-8 bytes
#if T_VALID
      else if (unlikely (utf8_byte_is_lead4 (c)))
#else
      else
#endif
      {
#if T_EXPLICIT
        if (unlikely ((i + 4u) > e))
        {
          t = e - (i + 4u);
          goto too_short;
        }
#endif

#if T_EXPLICIT && T_VALID && INT_32BIT
        u32f c1 = i[1];
        u32f c2 = i[2];
        u32f c3 = i[3];

        if (unlikely (((c1 | (c2 << 8) | (c3 << 16)) & 0xC0C0C0u) != 0x808080u)) goto invalid;

        u32f w = utf8_char_make4 (c, c1, c2, c3);
#else
        u32f c1 = i[1];
        unused(c1);

  #if T_VALID
        if (unlikely (!utf8_byte_is_trail (c1))) goto invalid;
  #elif !T_EXPLICIT
        if (unlikely (c1 == '\0')) goto invalid;
  #endif

        u32f c2 = i[2];
        unused(c2);

  #if T_VALID
        if (unlikely (!utf8_byte_is_trail (c2))) goto invalid;
  #elif !T_EXPLICIT
        if (unlikely (c2 == '\0')) goto invalid;
  #endif

        u32f c3 = i[3];
        unused(c3);

  #if T_VALID
        if (unlikely (!utf8_byte_is_trail (c3))) goto invalid;

        // Compose the UTF-32 codepoint from four UTF-8 bytes
        u32f w = utf8_char_make4 (c, c1, c2, c3);
  #elif !T_EXPLICIT
        if (unlikely (c3 == '\0')) goto invalid;
  #endif
#endif

#if T_VALID
        // Check if the character exceeds the maximum allowed Unicode codepoint
        if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_char_is_rsrv (w))) goto invalid;
#endif

        i += 4u;
      }

#if T_VALID
      else goto invalid;
#endif

      pts++;
    }

#if T_UTF8_COUNT_SIMD
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }
#endif

#if T_EXPLICIT && !T_VALID && CPU_X86
done:
#endif
  *end = ptr_unconst(i);
  *num = pts;

  return 0;

#if T_VALID || !T_EXPLICIT
invalid:
  *end = ptr_unconst(i);
  *num = pts;

  return INT_MIN;
#endif

#if T_EXPLICIT
too_short:
  *end = ptr_unconst(i);
  *num = pts;

  return (int)t;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
