// =============================================================================
// <utf8/utf8.c>
//
// UTF-8 template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  const u8* restrict i = in;

#if T(EXPLICIT)
  const u8* restrict e = in + len;
  int t;
#endif

  size_t pts = 0;

#if CPU(SSE2)
  const u8* restrict p;
  const u8* restrict r;
  uint a = 0;

  #if T(EXPLICIT)
    p = e - 16;
  #else
again:
    p = ptr_align_ceil (CPU_PAGE_SIZE, i);
    p -= 16;
  #endif

  // Process 16 bytes at a time using SSE
recover:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

  #if !T(EXPLICIT)
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi8 (xi, _mm_setzero_si128())) != 0)) goto scalar;
  #endif

    // Check if the input vector consists only of ASCII characters
    uint ao = _mm_movemask_epi8 (xi);

    if (likely (ao == 0))
    {
      a = 0;

      pts += 16u;
      i += 16;

      continue;
    }

    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 4096) : (i + 16);

    goto fallback;
  }

  #if !T(EXPLICIT)
    if (likely (p[16] != '\0')) goto again;
  #endif

scalar:
  // Prevents return to SSE code when there are
  // less than 16 characters left in the input
  r = i + 16;

fallback:
#endif

#if CPU(X86) && T(EXPLICIT) && !T(VALID) && 1
  // It is possible to use the `bsr` instruction
  // to count UTF-8 codeponts if validation is not required
  while (i < e)
  {
    register uint c = *i;
    register u32 n = bsr32 (~c << 24);

    pts++;
    i += utf8_chr_is_head1 (c) ? 1 : n;

  #if CPU(SSE2)
    if (likely (i >= r)) goto recover;
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
    if (likely (utf8_chr_is_head1 (c)))
    {
      pts++;

#if !T(EXPLICIT)
      if (unlikely (c == '\0')) break;
#endif

      i++;
    }

    // Multibyte sequence
    else
    {
      u32 w;

      // Two UTF-8 bytes
      if (likely (utf8_chr_is_head2 (c)))
      {
#if T(EXPLICIT)
        // Check if the input ends abruptly
        if (unlikely ((i + 2) > e))
        {
          t = -1;

tooshort:
          *end = (u8*)i;
          *num = pts;

          return t;
        }
#endif

        uint c1 = i[1];

#if T(VALID)
        // Check if the input byte is a UTF-8 suffix byte
        if (unlikely (!utf8_chr_is_tail (c1))) goto invalid;
#elif !T(EXPLICIT)
        if (unlikely (c1 == '\0'))
        {
invalid:
          *end = (u8*)i;
          *num = pts;

          return INT_MIN;
        }
#endif

        // Compose the UTF-32 codepoint from two UTF-8 bytes
        w = utf8_codep_comp2 (c, c1);

#if T(VALID)
        // Check for overlong sequence
        if (unlikely (utf32_chr_is8_lead1 (w))) goto invalid;
#endif

        i += 2;
      }

      // Three UTF-8 bytes
      else if (likely (utf8_chr_is_head3 (c)))
      {
#if T(EXPLICIT)
        if (unlikely ((i + 3) > e))
        {
          t = e - (i + 3);
          goto tooshort;
        }
#endif

#if T(EXPLICIT) && CPU(UNALIGNED_ACCESS) && HAVE(INT16) && 0
        u16 cv = *(u16*)(i + 1);

  #if T(VALID)
        if (unlikely ((cv & 0xC0C0u) != 0x8080u)) goto invalid;
  #endif

  #if CPU(LITTLE_ENDIAN)
        w = utf8_codep_comp_head3 (c) | ((cv & 0x3Fu) << 6) | ((cv & 0x3F00u) >> 8);
  #else
        w = utf8_codep_comp_head3 (c) | ((cv & 0x3F00u) >> 2) | (cv & 0x3Fu);
  #endif
#elif T(EXPLICIT) && HAVE(INT16) && 1
        u16 c1 = i[1];
        u16 c2 = i[2];

  #if T(VALID)
        if (unlikely (((c1 | (c2 << 8)) & 0xC0C0u) != 0x8080u)) goto invalid;
  #endif

        w = utf8_codep_comp3 (c, c1, c2);
#else
        uint c1 = i[1];

  #if T(VALID)
        if (unlikely (!utf8_chr_is_tail (c1))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c1 == '\0')) goto invalid;
  #endif

        uint c2 = i[2];

  #if T(VALID)
        if (unlikely (!utf8_chr_is_tail (c2))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c2 == '\0')) goto invalid;
  #endif

        // Compose the UTF-32 codepoint from three UTF-8 bytes
        w = utf8_codep_comp3 (c, c1, c2);
#endif

#if T(VALID)
        // Check for overlong sequence
        if (unlikely (utf32_chr_is8_lead2 (w))) goto invalid;

        // Check for UTF-16 surrogate character
        if (unlikely (utf16_chr_is_surr (w))) goto invalid;

        // Check for Unicode non-character
        if (unlikely (utf16_chr_is_non (w))) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

        i += 3;
      }

      // Four UTF-8 bytes
#if T(VALID)
      else if (unlikely (utf8_chr_is_head4 (c)))
#else
      else
#endif
      {
#if T(EXPLICIT)
        if (unlikely ((i + 4) > e))
        {
          t = e - (i + 4);
          goto tooshort;
        }
#endif

#if T(EXPLICIT) && CPU(UNALIGNED_ACCESS) && HAVE(INT32) && 0
        u32 cv = *(u32*)i;

  #if CPU(LITTLE_ENDIAN)
    #if T(VALID)
        if (unlikely ((cv & 0xC0C0C000u) != 0x80808000u)) goto invalid;
    #endif

        w = utf8_codep_comp_head4 (c) | ((cv & 0x3F00u) << 6) | ((cv & 0x3Fu) << 12) | ((cv & 0x3F0000u) >> 16);
  #else
    #if T(VALID)
        if (unlikely ((cv & 0xC0C0C0u) != 0x808080u)) goto invalid;
    #endif

        w = utf8_codep_comp_head4 (c) | ((cv & 0x3F0000u) >> 4) | ((cv & 0x3F00u) >> 2) | (cv & 0x3Fu);
  #endif
#elif T(EXPLICIT) && HAVE(INT32) && 1

        u32 c1 = i[1];
        u32 c2 = i[2];
        u32 c3 = i[3];

  #if T(VALID)
        if (unlikely (((c1 | (c2 << 8) | (c3 << 16)) & 0xC0C0C0u) != 0x808080u)) goto invalid;
  #endif

        w = utf8_codep_comp4 (c, c1, c2, c3);
#else
        uint c1 = i[1];

  #if T(VALID)
        if (unlikely (!utf8_chr_is_tail (c1))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c1 == '\0')) goto invalid;
  #endif

        uint c2 = i[2];

  #if T(VALID)
        if (unlikely (!utf8_chr_is_tail (c2))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c2 == '\0')) goto invalid;
  #endif

        uint c3 = i[3];

  #if T(VALID)
        if (unlikely (!utf8_chr_is_tail (c3))) goto invalid;
  #elif !T(EXPLICIT)
        if (unlikely (c3 == '\0')) goto invalid;
  #endif

        // Compose the UTF-32 codepoint from four UTF-8 bytes
        w = utf8_codep_comp4 (c, c1, c2, c3);
#endif

#if T(VALID)
        // Check if the character exceeds the maximum allowed Unicode codepoint
        if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

        // Check for reserved Unicode character
        if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

        i += 4;
      }

#if T(VALID)
      // Invalid sequence
      else
      {
invalid:
        *end = (u8*)i;
        *num = pts;

        return INT_MIN;
      }
#endif

      pts++;
    }

#if CPU(SSE2)
    // Attempt to parse with SSE again
    if (likely (i >= r)) goto recover;
#endif
  }

done:
  *end = (u8*)i;
  *num = pts;

  return 0;
}

// -----------------------------------------------------------------------------

#undef T_VALID
#undef T_EXPLICIT
