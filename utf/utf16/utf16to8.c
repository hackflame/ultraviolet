// =============================================================================
// <utf16/utf16to8.c>
//
// UTF-16 to UTF-8 conversion template.
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
  u8* restrict o = out;
  u8* restrict m = out + size;
  int n;
#endif

#if CPU(SSE2)
  const u16* restrict p;
  const u16* restrict r;
  uint a = 0;

  const xi128 xz = _mm_setzero_si128();

  #if CPU(SSE41) && 1
    const xi128 xone = _mm_set1_epi16 (1);

    const xi128 x3f = _mm_set1_epi16 (0x3F);
    const xi128 x80 = _mm_set1_epi16 (0x80);
    const xi128 xfffe = _mm_set1_epi16 (0xFFFE);

    const xi128 xn = _mm_setr_epi8 (0 << 5, 1 << 5, 2 << 5, 3 << 5
    , 4 << 5, 5 << 5, 6 << 5, 7 << 5
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
    if (unlikely ((o + 16) > m)) goto scalar;
  #endif

    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi16 (0x8000));

    // Find out which UTF-16 codepoints transform into 2-byte UTF-8 sequences
    xi128 x2 = _mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0x80 - 1) ^ 0x8000));

    // Check if this vector contains only characters from ASCII range
    if (likely (_mm_movemask_epi8 (x2) == 0))
    {
      a = 0;

  #if T(SIZE)
      sz += 8u;
  #else
      _mm_storel_epi64 ((xi128*)o, _mm_packus_epi16 (xi, xz));

      o += 8;
  #endif

      i += 8;

      continue;
    }

  #if CPU(SSE41) && 1
    // Find out which UTF-16 codepoints transform into 3-byte UTF-8 sequences
    xi128 x3 = _mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0x800 - 1) ^ 0x8000));

    // Check if this vector contains only 2-byte UTF-8 sequences
    if (likely (_mm_movemask_epi8 (x3) == 0))
    {
      // Create the expansion vector
      xi128 xe = _mm_and_si128 (x2, xone);

      // Find out how many bytes to expand each character
      // and create the shuffle vector
      xi128 xsh = _mm_slli_si128 (_mm_packus_epi16 (xe, xz), 1);
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));

      // Get the number of output bytes to advance
      uint oa = (_mm_extract_epi16 (xsh, 4) & 0x1Fu) + 8u;

    #if T(SIZE)
      sz += oa;
    #else
      // Store shuffle positions in a high nibble
      xsh = _mm_add_epi8 (xsh, xn);

      // Make gaps in the shuffle vector
      xi128 xsh8 = _mm_slli_si128 (xsh, 8);
      xsh8 = _mm_blendv_epi8 (xz, xsh8, _mm_slli_epi16 (xsh8, 4));
      xsh = _mm_or_si128 (xsh8, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 4)));

      xi128 xsh4 = _mm_slli_si128 (xsh, 4);
      xsh4 = _mm_blendv_epi8 (xz, xsh4, _mm_slli_epi16 (xsh4, 5));
      xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 5)));

      xi128 xsh2 = _mm_slli_si128 (xsh, 2);
      xsh2 = _mm_blendv_epi8 (xz, xsh2, _mm_slli_epi16 (xsh2, 6));
      xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 6)));

      xi128 xsh1 = _mm_slli_si128 (xsh, 1);
      xsh1 = _mm_blendv_epi8 (xz, xsh1, _mm_slli_epi16 (xsh1, 7));
      xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 7)));

      xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, _mm_set1_epi8 (0xE0)), 5);
      xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xz));
      xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

      // Get the ASCII bytes
      xi128 xb1 = _mm_andnot_si128 (x2, xi);

      // Get the 2-byte sequence suffix bytes
      xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Move 6 bits
      xi = _mm_srli_epi16 (xi, 6);

      // Get the 2-byte sequence prefix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, _mm_set1_epi16 (0xC0))));

      // Obtain the final UTF-8 bytes
      xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (xb1, xz), xshf);
      xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb2, xz), xshf), 1);

      // Merge the results and store
      xi128 xr = _mm_or_si128 (x8b1, x8b2);

      _mm_storeu_si128 ((xi128*)o, xr);

      o += oa;
    #endif

      i += 8;

      continue;
    }

    // Check for UTF-16 surrogate characters
    xi128 xs = _mm_cmpeq_epi16 (_mm_and_si128 (xi, _mm_set1_epi16 (0xF800)), _mm_set1_epi16 (0xD800));

    #if T(VALID)
      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi16 (xv, _mm_set1_epi16 ((0xFDD0 - 1) ^ 0x8000))
      , _mm_cmplt_epi16 (xv, _mm_set1_epi16 ((0xFDEF + 1) ^ 0x8000)));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi16 (_mm_and_si128 (xi, xfffe), xfffe);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (_mm_or_si128 (xnon, xrsrv), xs)) != 0)) goto scalar;
    #else
      // An input vector with UTF-16 surrogate characters is not feasible to vectorize
      if (unlikely (_mm_movemask_epi8 (xs) != 0)) goto scalar;
    #endif

    // Create the expansion vector
    xi128 xe = _mm_and_si128 (x2, xone);
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));

    // Find out how many bytes to expand each character
    // and create the shuffle vector
    xi128 x8e = _mm_packus_epi16 (xe, xz);

    xi128 xsh = _mm_slli_si128 (x8e, 1);
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 8));

    // Store shuffle positions in a high nibble and discard UTF-16 characters
    // which overflow 16-byte SSE vector size
    xi128 xsb = xsh;
    xsh = _mm_add_epi8 (xsh, xn);
    xsh = _mm_andnot_si128 (_mm_cmpgt_epi8 (xsb, _mm_set1_epi8 (8)), xsh);

    // Get the number of input and output bytes to advance
    uint ia = bsr32 (~(uint)_mm_movemask_epi8 (_mm_cmpeq_epi8 (xsh, xz)) & 0xFFu);
    uint ea = (_mm_cvtsi128_si64 (x8e) >> (ia * 8u)) & 0x1Fu;
    uint oa = (_mm_cvtsi128_si64 (xsh) >> (ia * 8u)) & 0x1Fu;

    ia++;
    oa += ia + ea;

    ia = (oa > 16u) ? (ia - 1u) : ia;
    oa = (oa > 16u) ? (oa - ea - 1u) : oa;

    #if T(SIZE)
      sz += oa;
    #else
      // Make gaps in the shuffle vector
      xi128 xsh8 = _mm_slli_si128 (xsh, 8);
      xsh8 = _mm_blendv_epi8 (xz, xsh8, _mm_slli_epi16 (xsh8, 4));
      xsh = _mm_or_si128 (xsh8, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 4)));

      xi128 xsh4 = _mm_slli_si128 (xsh, 4);
      xsh4 = _mm_blendv_epi8 (xz, xsh4, _mm_slli_epi16 (xsh4, 5));
      xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 5)));

      xi128 xsh2 = _mm_slli_si128 (xsh, 2);
      xsh2 = _mm_blendv_epi8 (xz, xsh2, _mm_slli_epi16 (xsh2, 6));
      xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 6)));

      xi128 xsh1 = _mm_slli_si128 (xsh, 1);
      xsh1 = _mm_blendv_epi8 (xz, xsh1, _mm_slli_epi16 (xsh1, 7));
      xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 7)));

      xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, _mm_set1_epi8 (0xE0)), 5);
      xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xz));
      xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

      // Get the ASCII bytes
      xi128 xb1 = _mm_andnot_si128 (x2, xi);
      x2 = _mm_xor_si128 (x2, x3);

      // Get the 2-byte sequence suffix bytes
      xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Get the 3-byte sequence suffix bytes
      xi128 xb3 = _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Move 6 bits
      xi = _mm_srli_epi16 (xi, 6);

      // Get the 2-byte sequence prefix and remaining suffix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, _mm_set1_epi16 (0xC0))));
      xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80)));

      // Move 6 bits
      xi = _mm_srli_epi16 (xi, 6);

      // Get the 3-byte sequence prefix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, _mm_set1_epi16 (0xE0))));

      // Obtain the final UTF-8 bytes
      xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (xb1, xz), xshf);
      xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb2, xz), xshf), 1);
      xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (xb3, xz), xshf), 2);

      // Merge the results and store
      xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), x8b3);

      _mm_storeu_si128 ((xi128*)o, xr);

      o += oa;
    #endif

    i += ia;
  #else
    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 2048) : (i + 8);

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

    // One ASCII byte
    if (likely (utf16_chr_is8_lead1 (c)))
    {
#if T(SIZE)
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
        n = 1;

needspace:
        *end = (u16*)i;
        *num = (size_t)(o - out);

        return n;
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

    // Two UTF-8 bytes
    else if (likely (utf16_chr_is8_lead2 (c)))
    {
#if T(SIZE)
      sz += 2u;
#else
      if (unlikely ((o + 2) > m))
      {
        n = (o + 2) - m;
        goto needspace;
      }

      // Compose the 2-byte UTF-8 codepoint
      o[0] = utf8_codep_decomp_head2 (c);
      o[1] = utf8_codep_decomp_tail1 (c);

      o += 2;
#endif

      i++;
    }

    // Three UTF-8 bytes
    else if (likely (!utf16_chr_is_surr (c)))
    {
#if T(VALID)
      // Check for Unicode non-character
      if (unlikely (utf16_chr_is_non (c))) goto invalid;

      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
#endif

#if T(SIZE)
      sz += 3u;
#else
      if (unlikely ((o + 3) > m))
      {
        n = (o + 3) - m;
        goto needspace;
      }

      // Compose the 3-byte UTF-8 codepoint
      o[0] = utf8_codep_decomp_head3 (c);
      o[1] = utf8_codep_decomp_tail2 (c);
      o[2] = utf8_codep_decomp_tail1 (c);

      o += 3;
#endif

      i++;
    }

    // Four UTF-8 bytes
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
tooshort:
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
      sz += 4u;
#else
      if (unlikely ((o + 4) > m))
      {
        n = (o + 4) - m;
        goto needspace;
      }

      // Compose the 4-byte UTF-8 codepoint
      o[0] = utf8_codep_decomp_head4 (w);
      o[1] = utf8_codep_decomp_tail3 (w);
      o[2] = utf8_codep_decomp_tail2 (w);
      o[3] = utf8_codep_decomp_tail1 (w);

      o += 4;
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
