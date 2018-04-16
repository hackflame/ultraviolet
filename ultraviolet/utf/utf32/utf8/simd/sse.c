// =============================================================================
// <ultraviolet/utf/utf32/utf8/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

const u32* p;
const u32* r;

const xi128 xzero = _mm_setzero_si128();

const xi128 x80000000 = _mm_set1_epi32 ((int)0x80000000u);
const xi128 x8000007F = _mm_set1_epi32 ((int)((0x00000080u - 1u) ^ 0x80000000u));

#if CPU_SSE41
const xi128 xone = _mm_set1_epi32 (1);

const xi128 xnum = _mm_setr_epi8 ((char)(0 << 6), (char)(1u << 6)
, (char)(2u << 6), (char)(3u << 6), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

const xi128 xC0       = _mm_set1_epi8  ((char)0xC0u);
const xi128 x0000003F = _mm_set1_epi32 ((int)0x0000003Fu);
const xi128 x00000080 = _mm_set1_epi32 ((int)0x00000080u);
const xi128 x000000C0 = _mm_set1_epi32 ((int)0x000000C0u);
const xi128 x000000E0 = _mm_set1_epi32 ((int)0x000000E0u);
const xi128 x000000F0 = _mm_set1_epi32 ((int)0x000000F0u);
const xi128 x800007FF = _mm_set1_epi32 ((int)((0x00000800u - 1u) ^ 0x80000000u));
const xi128 x8000FFFF = _mm_set1_epi32 ((int)((0x00010000u - 1u) ^ 0x80000000u));

  #if T_VALID
const xi128 x0000D800 = _mm_set1_epi32 ((int)0x0000D800u);
const xi128 x001FF800 = _mm_set1_epi32 ((int)0x001FF800u);
const xi128 x0000FDCF = _mm_set1_epi32 ((int)(0x0000FDD0u - 1u));
const xi128 x0000FDF0 = _mm_set1_epi32 ((int)(0x0000FDEFu + 1u));
const xi128 x0000FFFE = _mm_set1_epi32 ((int)0x0000FFFEu);
const xi128 x8010FFFF = _mm_set1_epi32 ((int)((0x00110000u - 1u) ^ 0x80000000u));
  #endif
#else
uint a = 0;
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
    if (unlikely ((o + 16u) > m)) goto scalar;
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

    // Find out which UTF-32 characters transform into 2-byte UTF-8 sequences
    xi128 x2 = _mm_cmpgt_epi32 (xv, x8000007F);

    // Check if this vector contains only characters from ASCII range
    if (likely (_mm_movemask_epi8 (x2) == 0))
    {
#if !CPU_SSE41
      a = 0;
#endif

#if T_SIZE
      sz += 4u;
#else
      _mm_storel_epi64 ((xi128*)(void*)o, _mm_packus_epi16 (_mm_packs_epi32 (xi, xzero), xzero));
      o += 4u;
#endif

      i += 4u;

      continue;
    }

#if CPU_SSE41
    // Find out which UTF-32 characters transform into 3-byte UTF-8 sequences
    xi128 x3 = _mm_cmpgt_epi32 (xv, x800007FF);

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

    // Find out which UTF-32 characters transform into 4-byte UTF-8 sequences
    xi128 x4 = _mm_cmpgt_epi32 (xv, x8000FFFF);

    // Check if this vector doesn't contain any 4-byte UTF-8 sequences
    if (likely (_mm_movemask_epi8 (x4) == 0))
    {
      // Create the expansion vector
      xi128 xe = _mm_and_si128 (x2, xone);
      xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));

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
      xi128 xsh4 = _mm_slli_si128 (xsh, 4);
      xsh4 = _mm_blendv_epi8 (xzero, xsh4, _mm_slli_epi16 (xsh4, 5));
      xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 5)));

      xi128 xsh2 = _mm_slli_si128 (xsh, 2);
      xsh2 = _mm_blendv_epi8 (xzero, xsh2, _mm_slli_epi16 (xsh2, 6));
      xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 6)));

      xi128 xsh1 = _mm_slli_si128 (xsh, 1);
      xsh1 = _mm_blendv_epi8 (xzero, xsh1, _mm_slli_epi16 (xsh1, 7));
      xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xzero, _mm_slli_epi16 (xsh, 7)));

      xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, xC0), 6);
      xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xzero));
      xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

      // Get the ASCII bytes
      xi128 xb1 = _mm_andnot_si128 (x2, xi);
      x2 = _mm_xor_si128 (x2, x3);

      // Get the 2-byte sequence suffix bytes
      xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080));

      // Get the 3-byte sequence suffix bytes
      xi128 xb3 = _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080));

      // Move 6 bits
      xi = _mm_srli_epi32 (xi, 6);

      // Get the 2-byte sequence prefix and remaining suffix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, x000000C0)));
      xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080)));

      // Move 6 bits
      xi = _mm_srli_epi32 (xi, 6);

      // Get the 3-byte sequence prefix and remaining suffix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, x000000E0)));

      // Obtain the final UTF-8 bytes
      xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb1, xzero), xzero), xshf);
      xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb2, xzero), xzero), xshf), 1);
      xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb3, xzero), xzero), xshf), 2);

      // Merge the results and store
      xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), x8b3);

      _mm_storeu_si128 ((xi128*)(void*)o, xr);

      o += oa;
    #endif

      i += 4u;

      continue;
    }

  #if T_VALID
    // Check if any UTF-32 character exceeds the maximum allowed Unicode codepoint
    xi128 xmax = _mm_cmpgt_epi32 (xv, x8010FFFF);
    if (unlikely (_mm_movemask_epi8 (xmax) != 0)) goto scalar;
  #endif

    // Create the expansion vector
    xi128 xe = _mm_and_si128 (x2, xone);
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x4, xone));

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

    xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, xC0), 6);
    xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xzero));
    xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

    // Get the ASCII bytes
    xi128 xb1 = _mm_andnot_si128 (x2, xi);

    x2 = _mm_xor_si128 (x2, x3);
    x3 = _mm_xor_si128 (x3, x4);

    // Get the 2-byte sequence suffix bytes
    xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080));

    // Get the 3-byte sequence suffix bytes
    xi128 xb3 = _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080));

    // Get the 4-byte sequence suffix bytes
    xi128 xb4 = _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080));

    // Move 6 bits
    xi = _mm_srli_epi32 (xi, 6);

    // Get the 2-byte sequence prefix and remaining suffix bytes
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, x000000C0)));
    xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080)));
    xb3 = _mm_or_si128 (xb3, _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080)));

    // Move 6 bits
    xi = _mm_srli_epi32 (xi, 6);

    // Get the 3-byte sequence prefix and remaining suffix bytes
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, x000000E0)));
    xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x0000003F), x00000080)));

    // Move 6 bits
    xi = _mm_srli_epi32 (xi, 6);

    // Get the 4-byte sequence prefix bytes
    xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x4, _mm_or_si128 (xi, x000000F0)));

    // Obtain the final UTF-8 bytes
    xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb1, xzero), xzero), xshf);
    xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb2, xzero), xzero), xshf), 1);
    xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb3, xzero), xzero), xshf), 2);
    xi128 x8b4 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb4, xzero), xzero), xshf), 3);

    // Merge the results and store
    xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), _mm_or_si128 (x8b3, x8b4));

    _mm_storeu_si128 ((xi128*)(void*)o, xr);

    o += oa;
  #endif

    i += 4u;
#else
    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 1024u) : (i + 4u);

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
