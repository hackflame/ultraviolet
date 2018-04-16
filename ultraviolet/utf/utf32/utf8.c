// =============================================================================
// <ultraviolet/utf/utf32/utf8.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u32* i = in;

#if T_EXPLICIT
  const u32* e = in + len;
#endif

#if T_SIZE
  size_t sz = 0;
#else
  u8* o = out;
  const u8* m = out + size;
  ptr_diff_t n;
#endif

#if T_UTF32_TO8_SIMD
  // Include the SIMD code path
  #include "utf8/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u32f c = *i;

    // One ASCII byte
    if (likely (utf32_codep_is_lead1 (c)))
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

      *o = (u8)c;
      o++;
#endif

#if !T_EXPLICIT
      if (unlikely (c == '\0')) break;
#endif
    }

    // Two UTF-8 bytes
    else if (likely (utf32_codep_is_lead2 (c)))
    {
#if T_SIZE
      sz += 2u;
#else
      if (unlikely ((o + 2u) > m))
      {
        n = (o + 2u) - m;
        goto need_space;
      }

      // Compose the 2-byte UTF-8 codepoint
      o[0] = (u8)utf8_codep_get_lead2 (c);
      o[1] = (u8)utf8_codep_get_trail1 (c);

      o += 2u;
#endif
    }

    // Three UTF-8 bytes
    else if (likely (utf32_codep_is_lead3 (c)))
    {
#if T_VALID
      // Check for UTF-16 surrogate character
      if (unlikely (utf16_cunit_is_surr (c))) goto invalid;

      // Check for Unicode non-character
      if (unlikely (utf16_codep_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (c))) goto invalid;
#endif

#if T_SIZE
      sz += 3u;
#else
      if (unlikely ((o + 3u) > m))
      {
        n = (o + 3u) - m;
        goto need_space;
      }

      // Compose the 3-byte UTF-8 codepoint
      o[0] = (u8)utf8_codep_get_lead3 (c);
      o[1] = (u8)utf8_codep_get_trail2 (c);
      o[2] = (u8)utf8_codep_get_trail1 (c);

      o += 3u;
#endif
    }

    // Four UTF-8 bytes
#if T_VALID
    else if (unlikely (utf32_codep_is_lead4 (c)))
#else
    else
#endif
    {
#if T_VALID
      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (c))) goto invalid;
#endif

#if T_SIZE
      sz += 4u;
#else
      if (unlikely ((o + 4u) > m))
      {
        n = (o + 4u) - m;
        goto need_space;
      }

      // Compose the 4-byte UTF-8 codepoint
      o[0] = (u8)utf8_codep_get_lead4 (c);
      o[1] = (u8)utf8_codep_get_trail3 (c);
      o[2] = (u8)utf8_codep_get_trail2 (c);
      o[3] = (u8)utf8_codep_get_trail1 (c);

      o += 4u;
#endif
    }

#if T_VALID
    // Invalid character
    else
    {
invalid:
      *end = ptr_unconst(i);

  #if T_SIZE
      *num = sz;
  #else
      *num = (size_t)(o - out);
  #endif

      return INT_MIN;
    }
#endif

    i++;

#if T_UTF32_TO8_SIMD
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
