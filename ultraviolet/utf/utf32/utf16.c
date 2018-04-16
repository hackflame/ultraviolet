// =============================================================================
// <ultraviolet/utf/utf32/utf16.c>
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
  u16* o = out;
  const u16* m = out + size;
  ptr_diff_t n;
#endif

#if T_UTF32_TO16_SIMD
  // Include the SIMD code path
  #include "utf16/simd.c"
#endif

#if T_EXPLICIT
  while (i != e)
#else
  while (true)
#endif
  {
    register u32f c = *i;

    // Single BMP character
    if (likely (!utf32_codep_is_surr (c)))
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
    }

    // Surrogate pair
#if T_VALID
    else if (likely (utf32_codep_is_surr (c)))
#else
    else
#endif
    {
#if T_VALID
      // Check for reserved Unicode character
      if (unlikely (utf16_codep_is_rsrv (c))) goto invalid;
#endif

#if T_SIZE
      sz += 2u;
#else
      if (unlikely ((o + 2u) > m))
      {
        n = (o + 2u) - m;
        goto need_space;
      }

      // Compose the UTF-16 surrogate pair
      o[0] = (u16)utf16_make_surr_high (c);
      o[1] = (u16)utf16_make_surr_low (c);

      o += 2u;
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

#if T_UTF32_TO16_SIMD
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
