// =============================================================================
// <ultraviolet/utf/utf32/utf32.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  const u32* i = in;

#if T_EXPLICIT
  const u32* e = in + len;
#endif

#if T_UTF32_COUNT_SIMD
  // Include the SIMD code path
  #include "utf32/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u32f c = *i;
    unused(c);

#if T_VALID
    // Check for UTF-16 surrogate character
    if (unlikely (utf16_cunit_is_surr (c))) goto invalid;

    // Check for Unicode non-character
    if (unlikely (utf16_codep_is_non (c))) goto invalid;

    // Check for reserved Unicode character
    if (unlikely (utf16_codep_is_rsrv (c))) goto invalid;
#endif

#if !T_EXPLICIT
    if (unlikely (c == '\0')) break;
#endif

    i++;

#if T_UTF32_COUNT_SIMD
    // Attempt to parse with SIMD again
    if (likely (i >= r)) goto simd;
#endif
  }

  *end = ptr_unconst(i);
  return 0;

invalid:
  *end = ptr_unconst(i);
  return INT_MIN;
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
#undef T_VALID
