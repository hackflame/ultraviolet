// =============================================================================
// <ultraviolet/utf/utf32/utf32/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u32* p;
const u32* r;

uint b = 0;

#if !T_EXPLICIT
const xi128 xzero = _mm_setzero_si128();
#endif

const xi128 x80000000 = _mm_set1_epi32 ((int)0x80000000u);
const xi128 x8000FFFF = _mm_set1_epi32 ((int)((0x00010000u - 1u) ^ 0x80000000u));

#if T_VALID
const xi128 x0000D800 = _mm_set1_epi32 ((int)0x0000D800u);
const xi128 x001FF800 = _mm_set1_epi32 ((int)0x001FF800u);
const xi128 x0000FDCF = _mm_set1_epi32 ((int)(0x0000FDD0u - 1u));
const xi128 x0000FDF0 = _mm_set1_epi32 ((int)(0x0000FDEFu + 1u));
const xi128 x0000FFFE = _mm_set1_epi32 ((int)0x0000FFFEu);
#endif

{
#if T_EXPLICIT
  p = e - 4u;
#else
again:
  p = ptr_align_ceil (CPU_PAGE_SIZE, i);
  p -= 4u;
#endif

  // Process 4 words at a time using SSE
simd:
  while (i <= p)
  {
    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)(const void*)i);

#if !T_EXPLICIT
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi32 (xi
    , xzero)) != 0)) goto scalar;
#endif

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, x80000000);

#if T_VALID
    // Check for UTF-16 surrogate characters
    xi128 xsur = _mm_cmpeq_epi32 (_mm_and_si128 (xi, x001FF800), x0000D800);

    // Check for Unicode non-characters
    xi128 xnon = _mm_and_si128 (_mm_cmplt_epi32 (xi, x0000FDCF)
    , _mm_cmpgt_epi32 (xi, x0000FDF0));

    // Check for reserved Unicode characters
    xi128 xrsrv = _mm_cmpeq_epi32 (_mm_and_si128 (xi, x0000FFFE), x0000FFFE);

    if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (_mm_or_si128 (xsur
    , xnon), xrsrv)) != 0)) goto scalar;
#endif

    // Find out which UTF-32 characters transform into UTF-16 surrogate pairs
    xi128 xs = _mm_cmpgt_epi32 (xv, x8000FFFF);

    // Check if this vector contains only characters
    // from the basic multilingual plane
    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;
      i += 4u;

      continue;
    }

    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 1024u) : (i + 4u);

    goto fallback;
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 4u))) goto again;
#endif

scalar:
  // Prevents return to SSE code when there are
  // less than 4 characters left in the input
  r = i + 4u;
}

fallback:
