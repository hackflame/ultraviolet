// =============================================================================
// <ultraviolet/utf/utf16/utf16/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u16* p;
const u16* r;

uint b = 0;

#if !T_EXPLICIT
const xi128 xzero = _mm_setzero_si128();
#endif

const xi128 xD800 = _mm_set1_epi16 ((short)0xD800u);
const xi128 xF800 = _mm_set1_epi16 ((short)0xF800u);

#if T_VALID
const xi128 xFDCF = _mm_set1_epi16 ((short)(0xFDD0u - 1u));
const xi128 xFDF0 = _mm_set1_epi16 ((short)(0xFDEFu + 1u));
const xi128 xFFFE = _mm_set1_epi16 ((short)0xFFFEu);
#endif

{
#if T_EXPLICIT
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
    xi128 xi = _mm_loadu_si128 ((const xi128*)(const void*)i);

#if !T_EXPLICIT
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi
    , xzero)) != 0)) goto scalar;
#endif

    // Check if this vector contains only characters from the basic multilingual plane
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xF800), xD800);

    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;

#if T_VALID
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmplt_epi16 (xi, xFDCF)
      , _mm_cmpgt_epi16 (xi, xFDF0));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xFFFE), xFFFE);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xnon
      , xrsrv)) != 0)) goto scalar;
#endif

      i += 8u;
      pts += 8u;

      continue;
    }

    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 2048u) : (i + 8u);

    goto fallback;
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 8u))) goto again;
#endif

#if !T_EXPLICIT || T_VALID
scalar:
#endif
  // Prevents return to SSE code when there are
  // less than 8 characters left in the input
  r = i + 8u;
}

fallback:
