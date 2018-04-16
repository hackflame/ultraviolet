// =============================================================================
// <ultraviolet/utf/utf16/utf32.c>
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
  u32* o = out;
  const u32* m = out + size;
#endif

#if T_UTF16_TO32_SIMD
  // Include the SIMD code path
  #include "utf32/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u16f c = *i;

    // BMP character
    if (likely (!utf16_cunit_is_surr (c)))
    {
#if T_VALID
      // Check for Unicode non-character
      if (unlikely (utf16_codep_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (c))) goto invalid;
#endif

#if T_SIZE
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m)) goto need_space;

      *o = (u32)c;
      o++;
#endif

#if !T_EXPLICIT
      if (unlikely (c == '\0')) break;
#endif

      i++;
    }

    // Surrogate pair
#if T_VALID
    else if (unlikely (utf16_cunit_is_surr_high (c)))
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
      if (unlikely (!utf16_cunit_is_surr_low (cs))) goto invalid;
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
      if (unlikely (utf16_codep_is_rsrv (w))) goto invalid;
#endif

#if T_SIZE
      unused(w);
      sz++;
#else
      if (unlikely (o == m)) goto need_space;

      *o = (u32)w;
      o++;
#endif

      i += 2u;
    }

#if T_VALID
    else goto invalid;
#endif

#if T_UTF16_TO32_SIMD
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

  return 1;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
#undef T_SIZE
