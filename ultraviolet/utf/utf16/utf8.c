// =============================================================================
// <ultraviolet/utf/utf16/utf8.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u16* i = in;

#if T_EXPLICIT
  const u16* e = in + len;
#endif

#if T_SIZE
  size_t sz = 0;
#else
  u8* o = out;
  const u8* m = out + size;
  ptr_diff_t n;
#endif

#if T_UTF16_TO8_SIMD
  // Include the SIMD code path
  #include "utf8/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u16f c = *i;

    // One ASCII byte
    if (likely (utf16_char_is_lead1 (c)))
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

      i++;
    }

    // Two UTF-8 bytes
    else if (likely (utf16_char_is_lead2 (c)))
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
      o[0] = (u8)utf8_char_get_lead2 (c);
      o[1] = (u8)utf8_char_get_trail1 (c);

      o += 2u;
#endif

      i++;
    }

    // Three UTF-8 bytes
    else if (likely (!utf16_byte_is_surr (c)))
    {
#if T_VALID
      // Check for Unicode non-character
      if (unlikely (utf16_char_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_char_is_rsrv (c))) goto invalid;
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
      o[0] = (u8)utf8_char_get_lead3 (c);
      o[1] = (u8)utf8_char_get_trail2 (c);
      o[2] = (u8)utf8_char_get_trail1 (c);

      o += 3u;
#endif

      i++;
    }

    // Four UTF-8 bytes
#if T_VALID
    else if (unlikely (utf16_byte_is_surr_high (c)))
#else
    else
#endif
    {
#if T_EXPLICIT
      // Check if the input ends abruptly
      if (unlikely ((i + 2u) > e)) goto too_short;
#endif

      // Get the low surrogate character
      u32f cs = i[1];

#if T_VALID
      // Check if it's actually a low surrogate
      if (unlikely (!utf16_byte_is_surr_low (cs))) goto invalid;
#elif !T_EXPLICIT
      // Invalid sequence
      if (unlikely (cs == '\0')) goto invalid;
#endif

      // Compose the UTF-32 codepoint from the UTF-16 surrogate pair
      u32f w = utf16_surr_to_char (c, cs);

#if T_VALID
      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_char_is_rsrv (w))) goto invalid;
#endif

#if T_SIZE
      unused(w);
      sz += 4u;
#else
      if (unlikely ((o + 4u) > m))
      {
        n = (o + 4u) - m;
        goto need_space;
      }

      // Compose the 4-byte UTF-8 codepoint
      o[0] = (u8)utf8_char_get_lead4 (w);
      o[1] = (u8)utf8_char_get_trail3 (w);
      o[2] = (u8)utf8_char_get_trail2 (w);
      o[3] = (u8)utf8_char_get_trail1 (w);

      o += 4u;
#endif

      i += 2u;
    }

#if T_VALID
    else goto invalid;
#endif

#if T_UTF16_TO8_SIMD
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

  return -1;
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
