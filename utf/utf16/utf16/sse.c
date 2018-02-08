// =============================================================================
// <utf/utf16/utf16/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

;
  const u16* p;
  const u16* r;
  uint b = 0;

  const xi128 xd800 = _mm_set1_epi16 (0xD800);
  const xi128 xfffe = _mm_set1_epi16 (0xFFFE);

#if T(EXPLICIT)
  p = e - 8u;
#else
again:
  p = ptr_align_ceil (CPU_PAGE_SIZE, i);
  p -= 8u;
#endif

  // Process 8 words at a time using SSE
simd:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

#if !T(EXPLICIT)
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi
    , _mm_setzero_si128())) != 0)) goto scalar;
#endif

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi16 (0x8000));

    // Check if this vector contains only characters from the basic multilingual plane
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi
    , _mm_set1_epi16 (0xF800)), xd800);

    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;

#if T(VALID)
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi16 (xv
      , _mm_set1_epi16 ((0xFDD0 - 1) ^ 0x8000))
      , _mm_cmplt_epi16 (xv, _mm_set1_epi16 ((0xFDEF + 1) ^ 0x8000)));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfffe), xfffe);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xnon
      , xrsrv)) != 0)) goto scalar;
#endif

      pts += 8u;
      i += 8u;

      continue;
    }

    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 2048u) : (i + 8u);

    goto fallback;
  }

#if !T(EXPLICIT)
  // Got on next page?
  if (likely (i >= (p + 8u))) goto again;
#endif

scalar:
  // Prevents return to SSE code when there are
  // less than 8 characters left in the input
  r = i + 8u;

fallback:
