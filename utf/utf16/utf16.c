// =============================================================================
// <utf16/utf16.c>
//
// UTF-16 template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  const u16* restrict i = in;

#if T(EXPLICIT)
  const u16* restrict e = in + len;
#endif

  size_t pts = 0;

#if CPU(SSE2)
  const u16* restrict p;
  const u16* restrict r;
  uint b = 0;

  const xi128 xd800 = _mm_set1_epi16 (0xD800);
  const xi128 xfffe = _mm_set1_epi16 (0xFFFE);

  #if T(EXPLICIT)
    p = e - 8;
  #else
again:
    p = ptr_align_ceil (CPU_PAGE_SIZE, i);
    p -= 8;
  #endif

  // Process 8 words at a time using SSE
recover:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

  #if !T(EXPLICIT)
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi, _mm_setzero_si128())) != 0)) goto scalar;
  #endif

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi16 (0x8000));

    // Check if this vector contains only characters from the basic multilingual plane
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, _mm_set1_epi16 (0xF800)), xd800);

    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;

  #if T(VALID)
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0xFDD0 - 1) ^ 0x8000))
      , _mm_cmplt_epi16 (xv, _mm_set1_epi16 ((0xFDEF + 1) ^ 0x8000)));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfffe), xfffe);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xnon, xrsrv)) != 0)) goto scalar;
  #endif

      pts += 8u;
      i += 8;

      continue;
    }

    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4) ? (i + 2048) : (i + 8);

    goto fallback;
  }

  #if !T(EXPLICIT)
    if (likely (p[8] != '\0')) goto again;
  #endif

scalar:
  // Prevents return to SSE code when there are
  // less than 8 characters left in the input
  r = i + 8;

fallback:
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uint c = *i;

    // BMP character
    if (likely (!utf16_chr_is_surr (c)))
    {
#if T(VALID)
      // Check for Unicode non-character
      if (unlikely (utf16_chr_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

      pts++;

#if !T(EXPLICIT)
      if (unlikely (c == '\0')) break;
#endif

      i++;
    }

    // Surrogate pair
#if T(VALID)
    else if (unlikely (utf16_chr_is_surr_high (c)))
#else
    else
#endif
    {
#if T(EXPLICIT)
      // Check if the input ends abruptly
      if (unlikely ((i + 2) > e))
      {
too_short:
        *end = (u16*)i;
        *num = pts;

        return -1;
      }
#endif

      // Get the low surrogate character
      uint cs = i[1];

#if T(VALID)
      // Check if it's actually a low surrogate
      if (unlikely (!utf16_chr_is_surr_low (cs))) goto invalid;
#elif !T(EXPLICIT)
      if (unlikely (cs == '\0'))
      {
invalid:
        *end = (u16*)i;
        *num = pts;

        return INT_MIN;
      }
#endif

      // Compose the UTF-32 codepoint from the UTF-16 surrogate pair
      u32 w = utf16_surr_to32 (c, cs);

#if T(VALID)
      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

      pts++;

      i += 2;
    }

#if T(VALID)
    // Invalid sequence
    else
    {
invalid:
      *end = (u16*)i;
      *num = pts;

      return INT_MIN;
    }
#endif

#if CPU(SSE2)
    // Attempt to parse with SSE again
    if (likely (i >= r)) goto recover;
#endif
  }

  *end = (u16*)i;
  *num = pts;

  return 0;
}

// -----------------------------------------------------------------------------

#undef T_VALID
#undef T_EXPLICIT
