// =============================================================================
// <utf32/utf32.c>
//
// UTF-32 template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  const u32* restrict i = in;

#if T(EXPLICIT)
  const u32* restrict e = in + len;
#endif

#if CPU(SSE2)
  const u32* restrict p;
  const u32* restrict r;
  uint b = 0;

  const xi128 xfffe = _mm_set1_epi32 (0xFFFE);

  #if T(EXPLICIT)
    p = e - 4;
  #else
again:
    p = ptr_align_ceil (CPU_PAGE_SIZE, i);
    p -= 4;
  #endif

  // Process 4 words at a time using SSE
recover:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi32 (0x80000000));

  #if T(VALID)
    // Check for UTF-16 surrogate characters
    xi128 xsur = _mm_cmpeq_epi32 (_mm_and_si128 (xi, _mm_set1_epi32 (0x1FF800)), _mm_set1_epi32 (0xD800));

    // Check for Unicode non-characters
    xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0xFDD0 - 1) ^ 0x80000000))
    , _mm_cmplt_epi32 (xv, _mm_set1_epi32 ((0xFDEF + 1) ^ 0x80000000)));

    // Check for reserved Unicode characters
    xi128 xrsrv = _mm_cmpeq_epi32 (_mm_and_si128 (xi, xfffe), xfffe);

    if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (_mm_or_si128 (xsur, xnon), xrsrv)) != 0)) goto scalar;
  #endif

    // Find out which UTF-32 characters transform into UTF-16 surrogate pairs
    xi128 xs = _mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0x10000 - 1) ^ 0x80000000));

    // Check if this vector contains only characters
    // from the basic multilingual plane
    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;
      i += 4;

      continue;
    }

    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 1024) : (i + 4);

    goto fallback;
  }

  #if !T(EXPLICIT)
    if (likely (p[4] != '\0')) goto again;
  #endif

scalar:
  // Prevents return to SSE code when there are
  // less than 4 characters left in the input
  r = i + 4;

fallback:
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uf32 c = *i;

#if T(VALID)
    // Check for UTF-16 surrogate character
    if (unlikely (utf16_chr_is_surr (c)))
    {
invalid:
      *end = (u32*)i;
      return INT_MIN;
    }

    // Check for Unicode non-character
    if (unlikely (utf16_chr_is_non (c))) goto invalid;

    // Check for reserved Unicode character
    if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if !T(EXPLICIT)
    if (unlikely (c == '\0')) break;
#endif

    i++;

#if CPU(SSE2)
    // Attempt to parse with SSE again
    if (likely (i >= r)) goto recover;
#endif
  }

  *end = (u32*)i;

  return 0;
}

// -----------------------------------------------------------------------------

#undef T_VALID
#undef T_EXPLICIT
