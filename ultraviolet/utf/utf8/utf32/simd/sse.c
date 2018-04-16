// =============================================================================
// <ultraviolet/utf/utf8/utf32/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u8* p;
const u8* r;

const xi128 xzero = _mm_setzero_si128();

#if CPU_SSE41
const xi128 xone = _mm_set1_epi8 (1);
const xi128 xtwo = _mm_set1_epi8 (2);

const xi128 xmb2 = _mm_set1_epi8 ((char)(0xC2u - 1u));
const xi128 xmb3 = _mm_set1_epi8 ((char)(0xE0u - 1u));
const xi128 xmb4 = _mm_set1_epi8 ((char)(0xF0u - 1u));

const xi128 xnum = _mm_setr_epi8 (0, 1, 2, 3, 4, 5, 6, 7
, 8, 9, 10, 11, 12, 13, 14, 15);

const xi128 x07 = _mm_set1_epi8 ((char)0x07u);
const xi128 x80 = _mm_set1_epi8 ((char)0x80u);
const xi128 xC0 = _mm_set1_epi8 ((char)0xC0u);
const xi128 xC2 = _mm_set1_epi8 ((char)0xC2u);
const xi128 xE3 = _mm_set1_epi8 ((char)0xE3u);
const xi128 xF0 = _mm_set1_epi8 ((char)0xF0u);
const xi128 xF8 = _mm_set1_epi8 ((char)0xF8u);

  #if T_VALID
const xi128 xD8 = _mm_set1_epi8 ((char)0xD8u);

    #if CPU_SSE42
const xi128 xrsrv = _mm_setr_epi16 ((short)0xFFFEu
, (short)0xFFFFu, (short)0xFDD0u, (short)0xFDEFu
, 0, 0, 0, 0);
    #else
const xi128 xCF = _mm_set1_epi8 ((char)(0xD0u - 1u));
const xi128 xFD = _mm_set1_epi8 ((char)0xFDu);
const xi128 xFE = _mm_set1_epi8 ((char)0xFEu);
const xi128 xFF = _mm_set1_epi8 ((char)0xFFu);
    #endif
  #endif
#else
uint a = 0;
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
#if !T_SIZE
    if (unlikely ((o + 16u) > m)) goto scalar;
#endif

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
#if !CPU_SSE41
      a = 0;
#endif

#if T_SIZE
      sz += 16u;
#else
      xi128 xal = _mm_unpacklo_epi8 (xi, xzero);
      xi128 xah = _mm_unpackhi_epi8 (xi, xzero);

      _mm_storeu_si128 ((xi128*)(void*)o, _mm_unpacklo_epi16 (xal, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 4u), _mm_unpackhi_epi16 (xal, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 8u), _mm_unpacklo_epi16 (xah, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 12u), _mm_unpackhi_epi16 (xah, xzero));

      o += 16u;
#endif

      i += 16u;

      continue;
    }

#if CPU_SSE41
    // Find out which bytes mark the start of two-byte sequences.
    // `0xC2` is used because it's the minimum leading byte from which
    // the first 8-bit codepoint starts. `0xC1` would still be in ASCII range,
    // which would be an overlong sequence and is considered an error.
    xi128 x2 = _mm_cmplt_epi8 (xi, xmb2);

    // Get the bytes that correspond to the beginning of 2-byte sequences
    xi128 xb = _mm_blendv_epi8 (x80, xC2, x2);

    // Find out which bytes mark the start of three-byte sequences
    xi128 x3 = _mm_cmplt_epi8 (xi, xmb3);

    // Process the input vector containing only 2-byte UTF-8 sequences
    if (unlikely (_mm_movemask_epi8 (x3) == 0))
    {
      // Create the vector containing the remaining number
      // of trailing characters for each two-byte sequence
      // character found in this vector
      xi128 xc = _mm_and_si128 (xb, x07);
      xi128 xt = _mm_subs_epu8 (xc, xone);
      xi128 xcs = _mm_or_si128 (xc, _mm_slli_si128 (xt, 1));

  #if T_VALID
      // Check if only ASCII characters have zero continuation bytes
      if (unlikely ((ao ^ (uint)_mm_movemask_epi8 (_mm_cmpgt_epi8 (xcs
      , xzero))) != 0)) goto scalar;

      // Check if there's enough continuation bytes in each two-byte sequence
      if (unlikely (_mm_movemask_epi8 (_mm_cmpgt_epi8 (_mm_sub_epi8 (_mm_slli_si128 (xcs
      , 1), xcs), xone)) != 0)) goto scalar;
  #endif

      // Create the shift vector for the shuffle operation
      // needed to remove the single byte gaps in the vector
      xi128 xsh = xt;
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));
      xsh = _mm_and_si128 (xsh, _mm_cmplt_epi8 (xcs, xtwo));

      // Find out how many bytes should the the input be adjusted
      // if the vector ends in the middle of a two-byte sequence
      uint im = (uint)_mm_extract_epi16 (xcs, 7);
      uint ia = (im & 0x200u) ? 15u : 16u;

      // Find out how many words should the the output be adjusted
      uint om = (uint)_mm_extract_epi16 (xsh, 7);
      uint oa = ia - (om >> (8u * (ia + 1u - 16u)));

  #if T_SIZE
      sz += oa;
  #else
      // Obtain the shuffle vector
      xi128 xsh1 = _mm_srli_si128 (xsh, 1);
      xsh = _mm_blendv_epi8 (xsh, xsh1, _mm_slli_epi16 (xsh1, 7));

      xi128 xsh2 = _mm_srli_si128 (xsh, 2);
      xsh = _mm_blendv_epi8 (xsh, xsh2, _mm_slli_epi16 (xsh2, 6));

      xi128 xsh4 = _mm_srli_si128 (xsh, 4);
      xsh = _mm_blendv_epi8 (xsh, xsh4, _mm_slli_epi16 (xsh4, 5));

      xi128 xsh8 = _mm_srli_si128 (xsh, 8);
      xsh = _mm_blendv_epi8 (xsh, xsh8, _mm_slli_epi16 (xsh8, 4));

      xi128 xshf = _mm_add_epi8 (xsh, xnum);

      // Get rid of the UTF-8 prefix bits in the vector
      xi128 xm = _mm_and_si128 (xb, xF8);
      xi = _mm_andnot_si128 (xm, xi);

      // Split the vector in low and high halves and compute the UTF-32 character
      // bits for each half; put the computed values at the end of each 2-byte sequence.
      // The gaps of junk created by this operation are removed in the next step.
      xi128 xr = _mm_slli_si128 (xi, 1);

      xi128 xl = _mm_blendv_epi8 (xi
      , _mm_or_si128 (xi, _mm_and_si128 (_mm_slli_epi16 (xr, 6), xC0))
      , _mm_cmpeq_epi8 (xcs, xone));

      xi128 xh = _mm_slli_si128 (_mm_srli_epi64 (_mm_and_si128 (xi
      , _mm_cmpeq_epi8 (xcs, xtwo)), 2), 1);

      // Produce the final UTF-32 characters
      xl = _mm_shuffle_epi8 (xl, xshf);
      xh = _mm_shuffle_epi8 (xh, xshf);

      xi128 xul = _mm_unpacklo_epi8 (xl, xh);
      xi128 xuh = _mm_unpackhi_epi8 (xl, xh);

      // Store the UTF-32 converted vectors and advance the pointers
      _mm_storeu_si128 ((xi128*)(void*)o, _mm_unpacklo_epi16 (xul, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 4u), _mm_unpackhi_epi16 (xul, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 8u), _mm_unpacklo_epi16 (xuh, xzero));
      _mm_storeu_si128 ((xi128*)(void*)(o + 12u), _mm_unpackhi_epi16 (xuh, xzero));

      o += oa;
  #endif

      i += ia;

      continue;
    }

    // Get the bytes that correspond to the beginning of 3-byte sequences
    xb = _mm_blendv_epi8 (xb, xE3, x3);

    // Find out which bytes mark the start of four-byte sequences
    xi128 x4 = _mm_cmplt_epi8 (xi, xmb4);

    // Vector with four-byte sequences in it isn't easily vectorizable
    // due to complex validation process
    if (unlikely (_mm_movemask_epi8 (x4) != 0)) goto scalar;

    // Create the vector containing the remaining number
    // of trailing characters for each multibyte sequence
    // character found in this vector
    xi128 xc = _mm_and_si128 (xb, x07);
    xi128 xt = _mm_subs_epu8 (xc, xone);
    xi128 xcs = _mm_or_si128 (xc, _mm_slli_si128 (xt, 1));
    xcs = _mm_or_si128 (xcs, _mm_slli_si128 (_mm_subs_epu8 (xc, xtwo), 2));

  #if T_VALID
    // Check if only ASCII characters have zero continuation bytes
    if (unlikely ((ao ^ (uint)_mm_movemask_epi8 (_mm_cmpgt_epi8 (xcs
    , xzero))) != 0)) goto scalar;

    // Check if there's enough continuation bytes in each multibyte sequence
    if (unlikely (_mm_movemask_epi8 (_mm_cmpgt_epi8 (_mm_sub_epi8 (_mm_slli_si128 (xcs
    , 1), xcs), xone)) != 0)) goto scalar;
  #endif

    // Create the shift vector for the shuffle operation
    // needed to remove the gaps in the vector
    xi128 xsh = xt;
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));
    xsh = _mm_and_si128 (xsh, _mm_cmplt_epi8 (xcs, xtwo));

    // Find out how many bytes should the the input be adjusted
    uint im = (uint)_mm_extract_epi16 (xcs, 7);
    uint ia = (im & 0x200u) ? ((im & 0x2u) ? 14u : 15u) : 16u;

    // Find out how many words should the the output be adjusted
    uint om = (uint)_mm_extract_epi32 (xsh, 3);
    uint oa = ia - ((om >> (8u * (ia + 3u - 16u))) & 0xFFu);

    // Obtain the shuffle vector
    xi128 xsh1 = _mm_srli_si128 (xsh, 1);
    xsh = _mm_blendv_epi8 (xsh, xsh1, _mm_slli_epi16 (xsh1, 7));

    xi128 xsh2 = _mm_srli_si128 (xsh, 2);
    xsh = _mm_blendv_epi8 (xsh, xsh2, _mm_slli_epi16 (xsh2, 6));

    xi128 xsh4 = _mm_srli_si128 (xsh, 4);
    xsh = _mm_blendv_epi8 (xsh, xsh4, _mm_slli_epi16 (xsh4, 5));

    xi128 xsh8 = _mm_srli_si128 (xsh, 8);
    xsh = _mm_blendv_epi8 (xsh, xsh8, _mm_slli_epi16 (xsh8, 4));

    xi128 xshf = _mm_add_epi8 (xsh, xnum);

    // Get rid of the UTF-8 prefix bits in the vector
    xi128 xm = _mm_and_si128 (xb, xF8);
    xi = _mm_andnot_si128 (xm, xi);

    // Split the vector in low and high halves and compute the UTF-32 character
    // bits for each half; put the computed values at the end of each sequence.
    // The gaps of junk created by this operation are removed in the next step.
    xi128 xr = _mm_slli_si128 (xi, 1);

    xi128 xl = _mm_blendv_epi8 (xi
    , _mm_or_si128 (xi, _mm_and_si128 (_mm_slli_epi16 (xr, 6), xC0))
    , _mm_cmpeq_epi8 (xcs, xone));

    xi128 xh = _mm_srli_epi64 (_mm_and_si128 (xi
    , _mm_cmpeq_epi8 (xcs, xtwo)), 2);

    xi128 xm3 = _mm_slli_si128 (x3, 1);

    xh = _mm_or_si128 (xh, _mm_and_si128 (xm3
    , _mm_and_si128 (_mm_slli_epi64 (xr, 4), xF0)));

  #if T_VALID
    // Check for overlong 3-byte sequences and surrogate characters
    xi128 xhb = _mm_and_si128 (xh, xF8);

    if (unlikely (!_mm_testz_si128 (xm3, _mm_or_si128 (_mm_cmpeq_epi8 (xhb, xzero)
    , _mm_cmpeq_epi8 (xhb, xD8))))) goto scalar;
  #endif

    xh = _mm_slli_si128 (xh, 1);

    // Produce the final UTF-32 characters
    xl = _mm_shuffle_epi8 (xl, xshf);
    xh = _mm_shuffle_epi8 (xh, xshf);

    xi128 xul = _mm_unpacklo_epi8 (xl, xh);
    xi128 xuh = _mm_unpackhi_epi8 (xl, xh);

  #if T_VALID
    // Check for Unicode non-characters and reserved Unicode characters
    #if CPU_SSE42
      #if T_EXPLICIT
    if (unlikely (_mm_cmpestrc (xrsrv, 4, xuh, 8, _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
      | _SIDD_CMP_RANGES | _SIDD_POSITIVE_POLARITY)
    | _mm_cmpestrc (xrsrv, 4, xul, 8, _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
      | _SIDD_CMP_RANGES | _SIDD_POSITIVE_POLARITY))) goto scalar;
      #else
    if (unlikely (_mm_cmpistrc (xrsrv, xuh, _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
      | _SIDD_CMP_RANGES | _SIDD_POSITIVE_POLARITY)
    | _mm_cmpistrc (xrsrv, xul, _SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
      | _SIDD_CMP_RANGES | _SIDD_POSITIVE_POLARITY))) goto scalar;
      #endif
    #else
    if (unlikely (!_mm_testz_si128 (_mm_cmpeq_epi8 (xh, xFD)
      , _mm_and_si128 (_mm_cmplt_epi8 (xl, xCF), _mm_cmpgt_epi8 (xl, xF0)))
    | !_mm_testz_si128 (_mm_cmpeq_epi8 (xh, xFF)
      , _mm_cmpeq_epi8 (_mm_and_si128 (xl, xFE), xFE)))) goto scalar;
    #endif
  #endif

  #if T_SIZE
    sz += oa;
  #else
    // Store the UTF-32 converted vectors and advance the pointers
    _mm_storeu_si128 ((xi128*)(void*)o, _mm_unpacklo_epi16 (xul, xzero));
    _mm_storeu_si128 ((xi128*)(void*)(o + 4u), _mm_unpackhi_epi16 (xul, xzero));
    _mm_storeu_si128 ((xi128*)(void*)(o + 8u), _mm_unpacklo_epi16 (xuh, xzero));
    _mm_storeu_si128 ((xi128*)(void*)(o + 12u), _mm_unpackhi_epi16 (xuh, xzero));

    o += oa;
  #endif

    i += ia;
#else
    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 4096u) : (i + 16u);

    goto fallback;
#endif
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 16u))) goto again;
#endif

#if !T_EXPLICIT || !T_SIZE
scalar:
#endif
  // Prevents return to SSE code when there are
  // less than 16 characters left in the input
  r = i + 16u;
}

#if !CPU_SSE41
fallback:
#endif
