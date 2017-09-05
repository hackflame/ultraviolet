// =============================================================================
// <utf16/utf16to32.c>
//
// UTF-16 to UTF-32 conversion template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  const u16* restrict i = in;

#if T(EXPLICIT)
  const u16* restrict e = in + len;
#endif

#if T(SIZE)
  size_t sz = 0;
#else
  u32* restrict o = out;
  u32* restrict m = out + size;
#endif

#if CPU(SSE2)
  const u16* restrict p;
  const u16* restrict r;
  uint b = 0;

  const xi128 xz = _mm_setzero_si128();

  const xi128 xd800 = _mm_set1_epi16 (0xD800);
  const xi128 xfffe = _mm_set1_epi16 (0xFFFE);

  #if CPU(SSE41) && 1
    const xi128 x3ff = _mm_set1_epi16 (0x3FF);
    const xi128 xfc00 = _mm_set1_epi16 (0xFC00);

    const xi128 xn = _mm_setr_epi8 (0, 1, 2, 3, 4, 5, 6, 7
    , 0, 0, 0, 0, 0, 0, 0, 0);
  #endif

  #if T(EXPLICIT)
    p = e - 8;
  #else
again:
    p = ptr_align_ceil (CPU_PAGE_SIZE, i);
    p -= 8;
  #endif

  // Process 8 words at a time using SSE
recover:
  while (i <= p)
  {
  #if !T(SIZE)
    if (unlikely ((o + 8) > m)) goto scalar;
  #endif

    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

  #if !T(EXPLICIT)
    // Catch the terminating null character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xi, _mm_setzero_si128())) != 0)) goto scalar;
  #endif

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi16 (0x8000));

    // Check if this vector contains only characters from the basic multilingual plane
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, _mm_set1_epi16 (0xF800)), xd800);

    if (likely (_mm_movemask_epi8 (xs) == 0))
    {
      b = 0;

  #if T(VALID)
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0xFDD0 - 1) ^ 0x8000))
      , _mm_cmplt_epi16 (xv, _mm_set1_epi16 ((0xFDEF + 1) ^ 0x8000)));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfffe), xfffe);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xnon, xrsrv)) != 0)) goto scalar;
  #endif

  #if T(SIZE)
      sz += 8u;
  #else
      _mm_storeu_si128 ((xi128*)o, _mm_unpacklo_epi16 (xi, xz));
      _mm_storeu_si128 ((xi128*)(o + 4), _mm_unpackhi_epi16 (xi, xz));

      o += 8;
  #endif

      i += 8;

      continue;
    }

  #if CPU(SSE41) && 1
    xi128 xhs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfc00), xd800);
    xi128 xls = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfc00), _mm_set1_epi16 (0xDC00));

    #if T(VALID)
      // High surrogate characters must be followed by low surrogates,
      // and the surrogate pair itself must not start with a low surrogate
      xi128 xbs = _mm_xor_si128 (_mm_slli_si128 (xhs, 2), xls);

      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0xFDD0 - 1) ^ 0x8000))
      , _mm_cmplt_epi16 (xv, _mm_set1_epi16 ((0xFDEF + 1) ^ 0x8000)));

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xbs, xnon)) != 0)) goto scalar;
    #endif

    // Create the shift vector
    xi128 xt = _mm_packus_epi16 (_mm_and_si128 (xls, _mm_set1_epi16 (0x1)), xz);

    xi128 xsh = xt;
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));
    xsh = _mm_and_si128 (xsh, _mm_cmplt_epi8 (xt, _mm_set1_epi8 (2)));

    // Get the number of input and output bytes to advance
    uint im = _mm_extract_epi16 (xhs, 7);
    uint ia = im ? 7u : 8u;

    uint om = _mm_extract_epi16 (xsh, 3);
    uint oa = ia - ((om >> (8u * (ia + 1u - 8u))) & 0xFFu);

    // Split the surrogate pairs found in this vector
    xi128 xh = _mm_slli_si128 (_mm_and_si128 (_mm_and_si128 (xhs, xi), x3ff), 2);

    xi128 xl = _mm_or_si128 (_mm_andnot_si128 (xs, xi), _mm_and_si128 (_mm_and_si128 (xi, xls), x3ff));
    xl = _mm_or_si128 (_mm_slli_epi16 (xh, 10), xl);

    xh = _mm_srli_epi16 (xh, 6);

    #if T(VALID)
      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xl, xfffe), xfffe);

      // Check for Unicode characters larger than maximum allowed codepoint
      xi128 xmax = _mm_cmpgt_epi16 (xh, _mm_set1_epi16 (0xF));

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (xmax, xrsrv)) != 0)) goto scalar;
    #endif

    #if T(SIZE)
      sz += oa;
    #else
      xh = _mm_add_epi16 (xh, _mm_slli_si128 (_mm_and_si128 (xhs, _mm_set1_epi16 (0x10000 >> 16)), 2));

      // Obtain the shuffle vector
      xi128 xsh1 = _mm_srli_si128 (xsh, 1);
      xsh = _mm_blendv_epi8 (xsh, xsh1, _mm_slli_epi16 (xsh1, 7));

      xi128 xsh2 = _mm_srli_si128 (xsh, 2);
      xsh = _mm_blendv_epi8 (xsh, xsh2, _mm_slli_epi16 (xsh2, 6));

      xi128 xsh4 = _mm_srli_si128 (xsh, 4);
      xsh = _mm_blendv_epi8 (xsh, xsh4, _mm_slli_epi16 (xsh4, 5));

      xi128 xshf = _mm_add_epi8 (xsh, xn);

      // Shuffle the input characters to remove gaps caused by surrogate pairs
      xi128 xll = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xl, _mm_set1_epi16 (0xFF)), xz), xshf);
      xi128 xlh = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xl, 8), xz), xshf);

      xi128 xhl = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_and_si128 (xh, _mm_set1_epi16 (0xFF)), xz), xshf);
      xi128 xhh = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_srli_epi16 (xh, 8), xz), xshf);

      // Produce the final UTF-32 characters
      xl = _mm_unpacklo_epi8 (xll, xlh);
      xh = _mm_unpacklo_epi8 (xhl, xhh);

      xi128 xul = _mm_unpacklo_epi16 (xl, xh);
      xi128 xuh = _mm_unpackhi_epi16 (xl, xh);

      _mm_storeu_si128 ((xi128*)o, xul);
      _mm_storeu_si128 ((xi128*)(o + 4), xuh);

      o += oa;
    #endif

    i += ia;
  #else
    // Back to scalar code since this vector contains non-BMP characters.
    // Resort to scalar-only processing if this happens too often.
    b++;

    // Retry again after some time
    r = (b >= 4u) ? (i + 2048) : (i + 8);

    goto fallback;
  #endif
  }

  #if !T(EXPLICIT)
    if (likely (p[8] != '\0')) goto again;
  #endif

scalar:
  // Prevents return to SSE code when there are
  // less than 8 characters left in the input
  r = i + 8;

fallback:
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uint c = *i;

    // BMP character
    if (likely (!utf16_chr_is_surr (c)))
    {
#if T(VALID)
      // Check for Unicode non-character
      if (unlikely (utf16_chr_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if T(SIZE)
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
need_space:
        *end = (u16*)i;
        *num = (size_t)(o - out);

        return 1;
      }

      *o = c;
#endif

#if !T(EXPLICIT)
      if (unlikely (c == '\0')) break;
#endif

#if !T(SIZE)
      o++;
#endif

      i++;
    }

    // Surrogate pair
#if T(VALID)
    else if (unlikely (utf16_chr_is_surr_high (c)))
#else
    else
#endif
    {
#if T(EXPLICIT)
      // Check if the input ends abruptly
      if (unlikely ((i + 2) > e))
      {
too_short:
        *end = (u16*)i;

  #if T(SIZE)
        *num = sz;
  #else
        *num = (size_t)(o - out);
  #endif

        return -1;
      }
#endif

      // Get the low surrogate character
      uint cs = i[1];

#if T(VALID)
      // Check if it's actually a low surrogate
      if (unlikely (!utf16_chr_is_surr_low (cs))) goto invalid;
#elif !T(EXPLICIT)
      if (unlikely (cs == '\0'))
      {
invalid:
        *end = (u16*)i;
        *num = (size_t)(o - out);

        return INT_MIN;
      }
#endif

      // Compose the UTF-32 codepoint from the UTF-16 surrogate pair
      u32 w = utf16_surr_to32 (c, cs);

#if T(VALID)
      // Check if the character exceeds the maximum allowed Unicode codepoint
      if (unlikely ((w - 0x10000u) > 0xFFFFFu)) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (w))) goto invalid;
#endif

#if T(SIZE)
      sz++;
#else
      if (unlikely (o == m)) goto need_space;

      *o++ = w;
#endif

      i += 2;
    }

#if T(VALID)
    // Invalid sequence
    else
    {
invalid:
      *end = (u16*)i;

  #if T(SIZE)
      *num = sz;
  #else
      *num = (size_t)(o - out);
  #endif

      return INT_MIN;
    }
#endif

#if CPU(SSE2)
    // Attempt to parse with SSE again
    if (likely (i >= r)) goto recover;
#endif
  }

  *end = (u16*)i;

#if T(SIZE)
  *num = sz;
#else
  *num = (size_t)(o - out);
#endif

  return 0;
}

// -----------------------------------------------------------------------------

#undef T_SIZE
#undef T_VALID
#undef T_EXPLICIT
