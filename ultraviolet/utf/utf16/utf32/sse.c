// =============================================================================
// <ultraviolet/utf/utf16/utf32/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u16* p;
const u16* r;

const xi128 xzero = _mm_setzero_si128();

const xi128 xD800 = _mm_set1_epi16 ((short)0xD800u);
const xi128 xF800 = _mm_set1_epi16 ((short)0xF800u);

#if T_VALID
const xi128 xFDCF = _mm_set1_epi16 ((short)(0xFDD0u - 1u));
const xi128 xFDF0 = _mm_set1_epi16 ((short)(0xFDEFu + 1u));
const xi128 xFFFE = _mm_set1_epi16 ((short)0xFFFEu);
#endif

#if CPU_SSE41
const xi128 xone = _mm_set1_epi16 (1);

const xi128 xnum = _mm_setr_epi8 (0, 1, 2, 3, 4, 5, 6, 7
, 0, 0, 0, 0, 0, 0, 0, 0);

const xi128 x00FF = _mm_set1_epi16 ((short)0x00FFu);
const xi128 x03FF = _mm_set1_epi16 ((short)0x03FFu);
const xi128 xDC00 = _mm_set1_epi16 ((short)0xDC00u);
const xi128 xFC00 = _mm_set1_epi16 ((short)0xFC00u);

  #if T_VALID
const xi128 x000F = _mm_set1_epi16 ((short)0xFu);
  #endif
#else
uint b = 0;
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
#if !T_SIZE
    if (unlikely ((o + 8u) > m)) goto scalar;
#endif

    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)(const void*)i);

#if !T_EXPLICIT
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi
    , _mm_setzero_si128())) != 0)) goto scalar;
#endif

    // Check if this vector contains only characters from the basic multilingual plane
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xF800), xD800);

    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
#if !CPU_SSE41
      b = 0;
#endif

#if T_VALID
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmplt_epi16 (xi, xFDCF)
      , _mm_cmpgt_epi16 (xi, xFDF0));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xFFFE), xFFFE);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xnon
      , xrsrv)) != 0)) goto scalar;
#endif

#if T_SIZE
      sz += 8u;
#else
      _mm_storeu_si128 ((xi128*)(void*)o, _mm_unpacklo_epi16 (xi, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 4u), _mm_unpackhi_epi16 (xi, xzero));

      o += 8u;
#endif

      i += 8u;

      continue;
    }

#if CPU_SSE41
    xi128 xhs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xFC00), xD800);
    xi128 xls = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xFC00), xDC00);

  #if T_VALID
    // High surrogate characters must be followed by low surrogates,
    // and the surrogate pair itself must not start with a low surrogate
    xi128 xbs = _mm_xor_si128 (_mm_slli_si128 (xhs, 2), xls);

    // Check for Unicode non-characters
    xi128 xnon = _mm_and_si128 (_mm_cmplt_epi16 (xi, xFDCF)
    , _mm_cmpgt_epi16 (xi, xFDF0));

    if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xbs, xnon)) != 0)) goto scalar;
  #endif

    // Create the shift vector
    xi128 xt = _mm_packus_epi16 (_mm_and_si128 (xls, xone), xzero);

    xi128 xsh = xt;
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));
    xsh = _mm_and_si128 (xsh, _mm_cmplt_epi8 (xt, _mm_set1_epi8 (2)));

    // Get the number of input and output bytes to advance
    uint im = (uint)_mm_extract_epi16 (xhs, 7);
    uint ia = im ? 7u : 8u;

    uint om = (uint)_mm_extract_epi16 (xsh, 3);
    uint oa = ia - ((om >> (8u * (ia + 1u - 8u))) & 0xFFu);

    // Split the surrogate pairs found in this vector
    xi128 xh = _mm_slli_si128 (_mm_and_si128 (_mm_and_si128 (xhs, xi), x03FF), 2);

    xi128 xl = _mm_or_si128 (_mm_andnot_si128 (xs, xi)
    , _mm_and_si128 (_mm_and_si128 (xi, xls), x03FF));

    xl = _mm_or_si128 (_mm_slli_epi16 (xh, 10), xl);
    xh = _mm_srli_epi16 (xh, 6);

  #if T_VALID
    // Check for reserved Unicode characters
    xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xl, xFFFE), xFFFE);

    // Check for Unicode characters larger than maximum allowed codepoint
    xi128 xmax = _mm_cmpgt_epi16 (xh, x000F);

    if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xmax
    , xrsrv)) != 0)) goto scalar;
  #endif

  #if T_SIZE
    sz += oa;
  #else
    xh = _mm_add_epi16 (xh, _mm_slli_si128 (_mm_and_si128 (xhs, xone), 2));

    // Obtain the shuffle vector
    xi128 xsh1 = _mm_srli_si128 (xsh, 1);
    xsh = _mm_blendv_epi8 (xsh, xsh1, _mm_slli_epi16 (xsh1, 7));

    xi128 xsh2 = _mm_srli_si128 (xsh, 2);
    xsh = _mm_blendv_epi8 (xsh, xsh2, _mm_slli_epi16 (xsh2, 6));

    xi128 xsh4 = _mm_srli_si128 (xsh, 4);
    xsh = _mm_blendv_epi8 (xsh, xsh4, _mm_slli_epi16 (xsh4, 5));

    xi128 xshf = _mm_add_epi8 (xsh, xnum);

    // Shuffle the input characters to remove gaps caused by surrogate pairs
    xi128 xll = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xl, x00FF), xzero), xshf);
    xi128 xlh = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xl, 8), xzero), xshf);

    xi128 xhl = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xh, x00FF), xzero), xshf);
    xi128 xhh = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xh, 8), xzero), xshf);

    // Produce the final UTF-32 characters
    xl = _mm_unpacklo_epi8 (xll, xlh);
    xh = _mm_unpacklo_epi8 (xhl, xhh);

    xi128 xul = _mm_unpacklo_epi16 (xl, xh);
    xi128 xuh = _mm_unpackhi_epi16 (xl, xh);

    _mm_storeu_si128 ((xi128*)(void*)o, xul);
    _mm_storeu_si128 ((xi128*)(void*)(o + 4u), xuh);

    o += oa;
  #endif

    i += ia;
#else
    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 2048u) : (i + 8u);

    goto fallback;
#endif
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 8u))) goto again;
#endif

#if !T_EXPLICIT || !T_SIZE
scalar:
#endif
  // Prevents return to SSE code when there are
  // less than 8 characters left in the input
  r = i + 8u;
}

#if !CPU_SSE41
fallback:
#endif
