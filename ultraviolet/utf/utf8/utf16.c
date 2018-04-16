// =============================================================================
// <ultraviolet/utf/utf8/utf16.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u8* i = in;

#if T_EXPLICIT
  const u8* e = in + len;
  ptr_diff_t t;
#endif

#if T_SIZE
  size_t sz = 0;
#else
  u16* o = out;
  const u16* m = out + size;
  ptr_diff_t n;
#endif

#if T_UTF8_TO16_SIMD
  // Include the SIMD code path
  #include "utf16/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u8f c = *i;

    // One ASCII byte
    if (likely (utf8_cunit_is_lead1 (c)))
    {
#if T_SIZE
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
        n = 1;
        goto need_space;
      }

      *o = (u16)c;
      o++;
#endif

#if !T_EXPLICIT
      if (unlikely (c == '\0')) break;
#endif

      i++;
    }

    // Two UTF-8 bytes
    else if (likely (utf8_cunit_is_lead2 (c)))
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

#if T_VALID
      // Check if the input byte is a UTF-8 suffix byte
      if (unlikely (!utf8_cunit_is_trail (c1))) goto invalid;
#elif !T_EXPLICIT
      // Invalid sequence
      if (unlikely (c1 == '\0')) goto invalid;
#endif

      // Compose the UTF-16 codepoint from two UTF-8 bytes
      u16f w = utf8_codep_make2 (c, c1);

#if T_VALID
      // Check for overlong sequence
      if (unlikely (utf16_codep_is_lead1 (w))) goto invalid;
#endif

#if T_SIZE
      unused(w);
      sz++;
#else
      if (unlikely (o == m))
      {
        n = 1;
        goto need_space;
      }

      *o = (u16)w;
      o++;
#endif

      i += 2u;
    }

    // Three UTF-8 bytes
    else if (likely (utf8_cunit_is_lead3 (c)))
    {
#if T_EXPLICIT
      if (unlikely ((i + 3u) > e))
      {
        t = e - (i + 3u);
        goto too_short;
      }
#endif

#if T_EXPLICIT && INT_16BIT
      u16f c1 = i[1];
      u16f c2 = i[2];

  #if T_VALID
      if (unlikely (((c1 | (c2 << 8)) & 0xC0C0u) != 0x8080u)) goto invalid;
  #endif

      u16f w = utf8_codep_make3 (c, c1, c2);
#else
      u16f c1 = i[1];

  #if T_VALID
      if (unlikely (!utf8_cunit_is_trail (c1))) goto invalid;
  #elif !T_EXPLICIT
      if (unlikely (c1 == '\0')) goto invalid;
  #endif

      u16f c2 = i[2];

  #if T_VALID
      if (unlikely (!utf8_cunit_is_trail (c2))) goto invalid;
  #elif !T_EXPLICIT
      if (unlikely (c2 == '\0')) goto invalid;
  #endif

      // Compose the UTF-16 codepoint from three UTF-8 bytes
      u16f w = utf8_codep_make3 (c, c1, c2);
#endif

#if T_VALID
      // Check for overlong sequence
      if (unlikely (utf16_codep_is_lead2 (w))) goto invalid;

      // Check for UTF-16 surrogate character
      if (unlikely (utf16_cunit_is_surr (w))) goto invalid;

      // Check for Unicode non-character
      if (unlikely (utf16_codep_is_non (w))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (w))) goto invalid;
#endif

#if T_SIZE
      unused(w);
      sz++;
#else
      if (unlikely (o == m))
      {
        n = 1;
        goto need_space;
      }

      *o = (u16)w;
      o++;
#endif

      i += 3u;
    }

    // Four UTF-8 bytes
#if T_VALID
    else if (unlikely (utf8_cunit_is_lead4 (c)))
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

#if T_EXPLICIT && INT_32BIT
      u32f c1 = i[1];
      u32f c2 = i[2];
      u32f c3 = i[3];

  #if T_VALID
      if (unlikely (((c1 | (c2 << 8) | (c3 << 16)) & 0xC0C0C0u) != 0x808080u)) goto invalid;
  #endif

      u32f w = utf8_codep_make4 (c, c1, c2, c3);
#else
      u32f c1 = i[1];

  #if T_VALID
      if (unlikely (!utf8_cunit_is_trail (c1))) goto invalid;
  #elif !T_EXPLICIT
      if (unlikely (c1 == '\0')) goto invalid;
  #endif

      u32f c2 = i[2];

  #if T_VALID
      if (unlikely (!utf8_cunit_is_trail (c2))) goto invalid;
  #elif !T_EXPLICIT
      if (unlikely (c2 == '\0')) goto invalid;
  #endif

      u32f c3 = i[3];

  #if T_VALID
      if (unlikely (!utf8_cunit_is_trail (c3))) goto invalid;
  #elif !T_EXPLICIT
      if (unlikely (c3 == '\0')) goto invalid;
  #endif

      // Compose the UTF-16 codepoint from four UTF-8 bytes
      u32f w = utf8_codep_make4 (c, c1, c2, c3);
#endif

#if T_VALID
      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (w))) goto invalid;
#endif

#if T_SIZE
      unused(w);
      sz += 2u;
#else
      if (unlikely ((o + 2u) > m))
      {
        n = (o + 2u) - m;
        goto need_space;
      }

      o[0] = (u16)utf16_make_surr_high (w);
      o[1] = (u16)utf16_make_surr_low (w);

      o += 2u;
#endif

      i += 4u;
    }

#if T_VALID
    else goto invalid;
#endif

#if T_UTF8_TO16_SIMD
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

  *end = ptr_unconst(i);

#if T_SIZE
  *num = sz;
#else
  *num = (size_t)(o - out);
#endif

  return 0;

#if T_VALID || !T_EXPLICIT
invalid:
  *end = ptr_unconst(i);

  #if T_SIZE
  *num = sz;
  #else
  *num = (size_t)(o - out);
  #endif

  return INT_MIN;
#endif

#if T_EXPLICIT
too_short:
  *end = ptr_unconst(i);

  #if T_SIZE
  *num = sz;
  #else
  *num = (size_t)(o - out);
  #endif

  return (int)t;
#endif

#if !T_SIZE
need_space:
  *end = ptr_unconst(i);
  *num = (size_t)(o - out);

  return (int)n;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
#undef T_SIZE
