// =============================================================================
// <ultraviolet/utf/utf16/utf8/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u16* p;
const u16* r;

const xi128 xzero = _mm_setzero_si128();

const xi128 x807F = _mm_set1_epi16 ((short)((0x0080u - 1u) ^ 0x8000u));
const xi128 x8000 = _mm_set1_epi16 ((short)0x8000u);

#if CPU_SSE41
const xi128 xone = _mm_set1_epi16 (1);

const xi128 xnum = _mm_setr_epi8 ((char)(0 << 5), (char)(1u << 5)
, (char)(2u << 5), (char)(3u << 5)
, (char)(4u << 5), (char)(5u << 5)
, (char)(6u << 5), (char)(7 << 5)
, 0, 0, 0, 0, 0, 0, 0, 0);

const xi128 xE0   = _mm_set1_epi8  ((char)0xE0u);
const xi128 x003F = _mm_set1_epi16 ((short)0x003Fu);
const xi128 x0080 = _mm_set1_epi16 ((short)0x0080u);
const xi128 x00C0 = _mm_set1_epi16 ((short)0x00C0u);
const xi128 x00E0 = _mm_set1_epi16 ((short)0x00E0u);
const xi128 x87FF = _mm_set1_epi16 ((short)((0x0800u - 1u) ^ 0x8000u));
const xi128 xD800 = _mm_set1_epi16 ((short)0xD800u);
const xi128 xF800 = _mm_set1_epi16 ((short)0xF800u);

  #if T_VALID
const xi128 xFDCF = _mm_set1_epi16 ((short)(0xFDD0u - 1u));
const xi128 xFDF0 = _mm_set1_epi16 ((short)(0xFDEFu + 1u));
const xi128 xFFFE = _mm_set1_epi16 ((short)0xFFFEu);
  #endif
#else
uint a = 0;
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
    if (unlikely ((o + 16u) > m)) goto scalar;
#endif

    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)(const void*)i);

#if !T_EXPLICIT
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi
    , xzero)) != 0)) goto scalar;
#endif

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, x8000);

    // Find out which UTF-16 codepoints transform into 2-byte UTF-8 sequences
    xi128 x2 = _mm_cmpgt_epi16 (xv, x807F);

    // Check if this vector contains only characters from ASCII range
    if (likely (_mm_movemask_epi8 (x2) == 0))
    {
#if !CPU_SSE41
      a = 0;
#endif

#if T_SIZE
      sz += 8u;
#else
      _mm_storel_epi64 ((xi128*)(void*)o, _mm_packus_epi16 (xi, xzero));
      o += 8u;
#endif

      i += 8u;

      continue;
    }

#if CPU_SSE41
    // Find out which UTF-16 codepoints transform into 3-byte UTF-8 sequences
    xi128 x3 = _mm_cmpgt_epi16 (xv, x87FF);

    // Check if this vector contains only 2-byte UTF-8 sequences
    if (likely (_mm_movemask_epi8 (x3) == 0))
    {
      // Create the expansion vector
      xi128 xe = _mm_and_si128 (x2, xone);

      // Find out how many bytes to expand each character
      // and create the shuffle vector
      xi128 xsh = _mm_slli_si128 (_mm_packus_epi16 (xe, xzero), 1);
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));

      // Get the number of output bytes to advance
      uint oa = ((uint)_mm_extract_epi16 (xsh, 4) & 0x1Fu) + 8u;

  #if T_SIZE
      sz += oa;
  #else
      // Store shuffle positions in a high nibble
      xsh = _mm_add_epi8 (xsh, xnum);

      // Make gaps in the shuffle vector
      xi128 xsh8 = _mm_slli_si128 (xsh, 8);
      xsh8 = _mm_blendv_epi8 (xzero, xsh8, _mm_slli_epi16 (xsh8, 4));
      xsh = _mm_or_si128 (xsh8, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 4)));

      xi128 xsh4 = _mm_slli_si128 (xsh, 4);
      xsh4 = _mm_blendv_epi8 (xzero, xsh4, _mm_slli_epi16 (xsh4, 5));
      xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 5)));

      xi128 xsh2 = _mm_slli_si128 (xsh, 2);
      xsh2 = _mm_blendv_epi8 (xzero, xsh2, _mm_slli_epi16 (xsh2, 6));
      xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 6)));

      xi128 xsh1 = _mm_slli_si128 (xsh, 1);
      xsh1 = _mm_blendv_epi8 (xzero, xsh1, _mm_slli_epi16 (xsh1, 7));
      xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 7)));

      xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, xE0), 5);
      xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xzero));
      xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

      // Get the ASCII bytes
      xi128 xb1 = _mm_andnot_si128 (x2, xi);

      // Get the 2-byte sequence suffix bytes
      xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x003F), x0080));

      // Move 6 bits
      xi = _mm_srli_epi16 (xi, 6);

      // Get the 2-byte sequence prefix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, x00C0)));

      // Obtain the final UTF-8 bytes
      xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (xb1, xzero), xshf);
      xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb2, xzero), xshf), 1);

      // Merge the results and store
      xi128 xr = _mm_or_si128 (x8b1, x8b2);

      _mm_storeu_si128 ((xi128*)(void*)o, xr);

      o += oa;
  #endif

      i += 8u;

      continue;
    }

    // Check for UTF-16 surrogate characters
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xF800), xD800);

  #if T_VALID
    // Check for Unicode non-characters
    xi128 xnon = _mm_and_si128 (_mm_cmplt_epi16 (xi, xFDCF)
    , _mm_cmpgt_epi16 (xi, xFDF0));

    // Check for reserved Unicode characters
    xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xFFFE), xFFFE);

    if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (_mm_or_si128 (xnon
    , xrsrv), xs)) != 0)) goto scalar;
  #else
    // An input vector with UTF-16 surrogate characters is not feasible to vectorize
    if (unlikely (_mm_movemask_epi8 (xs) != 0)) goto scalar;
  #endif

    // Create the expansion vector
    xi128 xe = _mm_and_si128 (x2, xone);
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));

    // Find out how many bytes to expand each character
    // and create the shuffle vector
    xi128 x8e = _mm_packus_epi16 (xe, xzero);

    xi128 xsh = _mm_slli_si128 (x8e, 1);
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));

    // Store shuffle positions in a high nibble and discard UTF-16 characters
    // which overflow 16-byte SSE vector size
    xi128 xsb = xsh;
    xsh = _mm_add_epi8 (xsh, xnum);
    xsh = _mm_andnot_si128 (_mm_cmpgt_epi8 (xsb, _mm_set1_epi8 (8)), xsh);

    // Get the number of input and output bytes to advance
    uint ia = bsr32 ((u32)~_mm_movemask_epi8 (_mm_cmpeq_epi8 (xsh, xzero)) & 0xFFu);
    uint ea = (_mm_cvtsi128_si64 (x8e) >> (ia * 8u)) & 0x1Fu;
    uint oa = (_mm_cvtsi128_si64 (xsh) >> (ia * 8u)) & 0x1Fu;

    ia++;
    oa += ia + ea;

    ia = (oa > 16u) ? (ia - 1u) : ia;
    oa = (oa > 16u) ? (oa - ea - 1u) : oa;

  #if T_SIZE
    sz += oa;
  #else
    // Make gaps in the shuffle vector
    xi128 xsh8 = _mm_slli_si128 (xsh, 8);
    xsh8 = _mm_blendv_epi8 (xzero, xsh8, _mm_slli_epi16 (xsh8, 4));
    xsh = _mm_or_si128 (xsh8, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 4)));

    xi128 xsh4 = _mm_slli_si128 (xsh, 4);
    xsh4 = _mm_blendv_epi8 (xzero, xsh4, _mm_slli_epi16 (xsh4, 5));
    xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 5)));

    xi128 xsh2 = _mm_slli_si128 (xsh, 2);
    xsh2 = _mm_blendv_epi8 (xzero, xsh2, _mm_slli_epi16 (xsh2, 6));
    xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 6)));

    xi128 xsh1 = _mm_slli_si128 (xsh, 1);
    xsh1 = _mm_blendv_epi8 (xzero, xsh1, _mm_slli_epi16 (xsh1, 7));
    xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 7)));

    xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, xE0), 5);
    xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xzero));
    xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

    // Get the ASCII bytes
    xi128 xb1 = _mm_andnot_si128 (x2, xi);
    x2 = _mm_xor_si128 (x2, x3);

    // Get the 2-byte sequence suffix bytes
    xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x003F), x0080));

    // Get the 3-byte sequence suffix bytes
    xi128 xb3 = _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x003F), x0080));

    // Move 6 bits
    xi = _mm_srli_epi16 (xi, 6);

    // Get the 2-byte sequence prefix and remaining suffix bytes
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, x00C0)));
    xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x003F), x0080)));

    // Move 6 bits
    xi = _mm_srli_epi16 (xi, 6);

    // Get the 3-byte sequence prefix bytes
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, x00E0)));

    // Obtain the final UTF-8 bytes
    xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (xb1, xzero), xshf);
    xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb2, xzero), xshf), 1);
    xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb3, xzero), xshf), 2);

    // Merge the results and store
    xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), x8b3);

    _mm_storeu_si128 ((xi128*)(void*)o, xr);

    o += oa;
  #endif

    i += ia;
#else
    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 2048u) : (i + 8u);

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
