// =============================================================================
// <ultraviolet/utf/utf32/utf16/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u32* p;
const u32* r;

const xi128 xzero = _mm_setzero_si128();

const xi128 x80000000 = _mm_set1_epi32 ((int)0x80000000u);
const xi128 x8000FFFF = _mm_set1_epi32 ((int)((0x00010000u - 1u) ^ 0x80000000u));

#if T_VALID
const xi128 x0000D800 = _mm_set1_epi32 ((int)0x0000D800u);
const xi128 x001FF800 = _mm_set1_epi32 ((int)0x001FF800u);
const xi128 x0000FDCF = _mm_set1_epi32 ((int)(0x0000FDD0u - 1u));
const xi128 x0000FDF0 = _mm_set1_epi32 ((int)(0x0000FDEFu + 1u));
const xi128 x0000FFFE = _mm_set1_epi32 ((int)0x0000FFFEu);
#endif

#if CPU_SSE41
const xi128 xone = _mm_set1_epi32 (1);

const xi128 xnum = _mm_setr_epi8 ((char)(0 << 6), (char)(1u << 6)
, (char)(2u << 6), (char)(3u << 6), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

const xi128 xC0       = _mm_set1_epi8  ((char)0xC0u);
const xi128 x00FF     = _mm_set1_epi16 ((short)0x00FFu);
const xi128 x000003FF = _mm_set1_epi32 ((int)0x000003FFu);
const xi128 x0000D7C0 = _mm_set1_epi32 ((int)(0xD800u - 0x40u));
const xi128 x0000DC00 = _mm_set1_epi32 ((int)0x0000DC00u);

  #if T_VALID
const xi128 x8010FFFF = _mm_set1_epi32 ((int)((0x00110000u - 1u) ^ 0x80000000u));
  #endif
#else
uint b = 0;
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
#if !T_SIZE
    if (unlikely ((o + 8u) > m)) goto scalar;
#endif

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

    // Check if this vector contains only characters from the basic multilingual plane
    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
#if !CPU_SSE41
      b = 0;
#endif

#if T_SIZE
      sz += 4u;
#else
  #if CPU_SSE41
      // SSE4.1 comes with unsigned 32-bit word packing (finally)
      _mm_storel_epi64 ((xi128*)(void*)o, _mm_packus_epi32 (xi, xzero));
  #else
      // Sign-extend for signed packing
      xi = _mm_slli_epi32 (xi, 16);
      xi = _mm_srai_epi32 (xi, 16);

      _mm_storel_epi64 ((xi128*)(void*)o, _mm_packs_epi32 (xi, xzero));
  #endif

      o += 4u;
#endif

      i += 4u;

      continue;
    }

#if CPU_SSE41
  #if T_VALID
    // Check if any UTF-32 character exceeds the maximum allowed Unicode codepoint
    xi128 xmax = _mm_cmpgt_epi32 (xv, x8010FFFF);
    if (unlikely (_mm_movemask_epi8 (xmax) != 0)) goto scalar;
  #endif

    // Create the expansion vector
    xi128 xe = _mm_and_si128 (xs, xone);

    // Find out how many bytes to expand each character
    // and create the shuffle vector
    xi128 x8e = _mm_packus_epi16 (_mm_packs_epi32 (xe, xzero), xzero);

    xi128 xsh = _mm_slli_si128 (x8e, 1);
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));

    // Store shuffle positions in a high nibble
    xsh = _mm_add_epi8 (xsh, xnum);

    // Get the number of output bytes to advance
    uint oa = ((uint)_mm_extract_epi16 (xsh, 2) & 0x3Fu) + 4u;

  #if T_SIZE
    sz += oa;
  #else
    xsh = _mm_srli_si128 (_mm_slli_si128 (xsh, 12), 12);

    // Make gaps in the shuffle vector
    xi128 xsh2 = _mm_slli_si128 (xsh, 2);
    xsh2 = _mm_blendv_epi8 (xzero, xsh2, _mm_slli_epi16 (xsh2, 6));
    xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 6)));

    xi128 xsh1 = _mm_slli_si128 (xsh, 1);
    xsh1 = _mm_blendv_epi8 (xzero, xsh1, _mm_slli_epi16 (xsh1, 7));
    xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 7)));

    xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, xC0), 6);
    xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xzero));
    xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

    // Get the BMP characters
    xi128 xb1 = _mm_andnot_si128 (xs, xi);

    // Get the low surrogate halves
    xi128 xb2 = _mm_and_si128 (xs, _mm_or_si128 (_mm_and_si128 (xi
    , x000003FF), x0000DC00));

    // Move 10 bits
    xi = _mm_srli_epi32 (xi, 10);

    // Get the high surrogate halves
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (xs
    , _mm_add_epi32 (xi, x0000D7C0)));

    // Obtain the final UTF-8 bytes
    xb1 = _mm_packus_epi32 (xb1, xzero);
    xb2 = _mm_packus_epi32 (xb2, xzero);

    xi128 xb1l = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xb1
    , x00FF), xzero), xshf);

    xi128 xb1h = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xb1
    , 8), xzero), xshf);

    xi128 xb2l = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xb2
    , x00FF), xzero), xshf);

    xi128 xb2h = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xb2
    , 8), xzero), xshf);

    // Merge the results and store
    xi128 xul = _mm_unpacklo_epi8 (xb1l, xb1h);
    xi128 xuh = _mm_unpacklo_epi8 (xb2l, xb2h);

    xi128 xr = _mm_or_si128 (xul, _mm_slli_si128 (xuh, 2));

    _mm_storeu_si128 ((xi128*)(void*)o, xr);

    o += oa;
  #endif

    i += 4u;
#else
    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 1024u) : (i + 4u);

    goto fallback;
#endif
  }

#if !T_EXPLICIT
  // Got on next page?
  if (likely (i >= (p + 4u))) goto again;
#endif

#if !T_EXPLICIT || !T_SIZE
scalar:
#endif
  // Prevents return to SSE code when there are
  // less than 4 characters left in the input
  r = i + 4u;
}

#if !CPU_SSE41
fallback:
#endif
