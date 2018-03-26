// =============================================================================
// <ultraviolet/utf/utf8/utf8/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u8* p;
const u8* r;

uint a = 0;

#if !T_EXPLICIT
const xi128 xzero = _mm_setzero_si128();
#endif

{
#if T_EXPLICIT
  p = e - 16u;
#else
again:
  p = ptr_align_ceil (CPU_PAGE_SIZE, i);
  p -= 16u;
#endif

  // Process 16 bytes at a time using SSE
simd:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)(const void*)i);

#if !T_EXPLICIT
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi8 (xi
    , xzero)) != 0)) goto scalar;
#endif

    // Check if the input vector consists only of ASCII characters
    uint ao = (uint)_mm_movemask_epi8 (xi);

    if (likely (ao == 0))
    {
      a = 0;
      i += 16u;
      pts += 16u;

      continue;
    }

    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 4096u) : (i + 16u);

    goto fallback;
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 16u))) goto again;
scalar:
#endif

  // Prevents return to SSE code when there are
  // less than 16 characters left in the input
  r = i + 16u;
}

fallback:
