// =============================================================================
// <ultraviolet/utf/utf16/utf16.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u16* i = in;
  size_t pts = 0;

#if T_EXPLICIT
  const u16* e = in + len;
#endif

#if T_UTF16_COUNT_SIMD
  // Include the SIMD code path
  #include "utf16/simd.c"
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

#if !T_EXPLICIT
      if (unlikely (c == '\0')) break;
#endif

      i++;
      pts++;
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
      unused(cs);

#if T_VALID
      // Check if it's actually a low surrogate
      if (unlikely (!utf16_cunit_is_surr_low (cs))) goto invalid;
#elif !T_EXPLICIT
      // Invalid sequence
      if (unlikely (cs == '\0')) goto invalid;
#endif

#if T_VALID
      // Compose the UTF-32 codepoint from the UTF-16 surrogate pair
      u32f w = utf16_surr_to_char (c, cs);

      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (w))) goto invalid;
#endif

      i += 2u;
      pts++;
    }

#if T_VALID
    else goto invalid;
#endif

#if T_UTF16_COUNT_SIMD
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

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

  return -1;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
