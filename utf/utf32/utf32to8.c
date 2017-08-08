// =============================================================================
// <utf32/utf32to8.c>
//
// UTF-32 to UTF-8 conversion template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  const u32* restrict i = in;

#if T(EXPLICIT)
  const u32* restrict e = in + len;
#endif

#if T(SIZE)
  size_t sz = 0;
#else
  u8* restrict o = out;
  u8* restrict m = out + size;
  int n;
#endif

#if CPU(SSE2)
  const u32* restrict p;
  const u32* restrict r;
  uint a = 0;

  const xi128 xz = _mm_setzero_si128();

  #if CPU(SSE41) && 1
    const xi128 xone = _mm_set1_epi32 (1);

    const xi128 x3f = _mm_set1_epi32 (0x3F);
    const xi128 x80 = _mm_set1_epi32 (0x80);
    const xi128 xfffe = _mm_set1_epi32 (0xFFFE);

    const xi128 xn = _mm_setr_epi8 (0 << 6, 1 << 6, 2 << 6, 3 << 6
    , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  #endif

  #if T(EXPLICIT)
    p = e - 4;
  #else
again:
    p = ptr_align_ceil (CPU_PAGE_SIZE, i);
    p -= 4;
  #endif

  // Process 4 words at a time using SSE
recover:
  while (i <= p)
  {
  #if !T(SIZE)
    if (unlikely ((o + 16) > m)) goto scalar;
  #endif

    // Get the input vector
    xi128 xi = _mm_loadu_si128 ((const xi128*)i);

    // Adjust the input vector for signed comparison
    xi128 xv = _mm_xor_si128 (xi, _mm_set1_epi32 (0x80000000));

    // Find out which UTF-32 characters transform into 2-byte UTF-8 sequences
    xi128 x2 = _mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0x80 - 1) ^ 0x80000000));

    // Check if this vector contains only characters from ASCII range
    if (likely (_mm_movemask_epi8 (x2) == 0))
    {
      a = 0;

  #if T(SIZE)
      sz += 4u;
  #else
      _mm_storel_epi64 ((xi128*)o, _mm_packus_epi16 (_mm_packs_epi32 (xi, xz), xz));

      o += 4;
  #endif

      i += 4;

      continue;
    }

  #if CPU(SSE41) && 1
    // Find out which UTF-32 characters transform into 3-byte UTF-8 sequences
    xi128 x3 = _mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0x800 - 1) ^ 0x80000000));

    #if T(VALID)
      // Check for UTF-16 surrogate characters
      xi128 xsur = _mm_cmpeq_epi32 (_mm_and_si128 (xi, _mm_set1_epi32 (0x1FF800)), _mm_set1_epi32 (0xD800));

      // Check for Unicode non-characters
      xi128 xnon = _mm_and_si128 (_mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0xFDD0 - 1) ^ 0x80000000))
      , _mm_cmplt_epi32 (xv, _mm_set1_epi32 ((0xFDEF + 1) ^ 0x80000000)));

      // Check for reserved Unicode characters
      xi128 xrsrv = _mm_cmpeq_epi32 (_mm_and_si128 (xi, xfffe), xfffe);

      if (unlikely (_mm_movemask_epi8 (_mm_or_si128 (_mm_or_si128 (xsur, xnon), xrsrv)) != 0)) goto scalar;
    #endif

    // Find out which UTF-32 characters transform into 4-byte UTF-8 sequences
    xi128 x4 = _mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0x10000 - 1) ^ 0x80000000));

    // Check if this vector doesn't contain any 4-byte UTF-8 sequences
    if (likely (_mm_movemask_epi8 (x4) == 0))
    {
      // Create the expansion vector
      xi128 xe = _mm_and_si128 (x2, xone);
      xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));

      // Find out how many bytes to expand each character
      // and create the shuffle vector
      xi128 x8e = _mm_packus_epi16 (_mm_packs_epi32 (xe, xz), xz);

      xi128 xsh = _mm_slli_si128 (x8e, 1);
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
      xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));

      // Store shuffle positions in a high nibble
      xsh = _mm_add_epi8 (xsh, xn);

      // Get the number of output bytes to advance
      uint oa = (_mm_extract_epi16 (xsh, 2) & 0x3Fu) + 4u;

      #if T(SIZE)
        sz += oa;
      #else
        xsh = _mm_srli_si128 (_mm_slli_si128 (xsh, 12), 12);

        // Make gaps in the shuffle vector
        xi128 xsh4 = _mm_slli_si128 (xsh, 4);
        xsh4 = _mm_blendv_epi8 (xz, xsh4, _mm_slli_epi16 (xsh4, 5));
        xsh = _mm_or_si128 (xsh4, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 5)));

        xi128 xsh2 = _mm_slli_si128 (xsh, 2);
        xsh2 = _mm_blendv_epi8 (xz, xsh2, _mm_slli_epi16 (xsh2, 6));
        xsh = _mm_or_si128 (xsh2, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 6)));

        xi128 xsh1 = _mm_slli_si128 (xsh, 1);
        xsh1 = _mm_blendv_epi8 (xz, xsh1, _mm_slli_epi16 (xsh1, 7));
        xsh = _mm_or_si128 (xsh1, _mm_blendv_epi8 (xsh, xz, _mm_slli_epi16 (xsh, 7)));

        xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, _mm_set1_epi8 (0xC0)), 6);
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
        xi = _mm_srli_epi32 (xi, 6);

        // Get the 2-byte sequence prefix and remaining suffix bytes
        xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, _mm_set1_epi32 (0xC0))));
        xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80)));

        // Move 6 bits
        xi = _mm_srli_epi32 (xi, 6);

        // Get the 3-byte sequence prefix and remaining suffix bytes
        xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, _mm_set1_epi32 (0xE0))));

        // Obtain the final UTF-8 bytes
        xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb1, xz), xz), xshf);
        xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb2, xz), xz), xshf), 1);
        xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb3, xz), xz), xshf), 2);

        // Merge the results and store
        xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), x8b3);

        _mm_storeu_si128 ((xi128*)o, xr);

        o += oa;
      #endif

      i += 4;

      continue;
    }

    #if T(VALID)
      // Check if any UTF-32 character exceeds the maximum allowed Unicode codepoint
      xi128 xmax = _mm_cmpgt_epi32 (xv, _mm_set1_epi32 ((0x110000 - 1) ^ 0x80000000));
      if (unlikely (_mm_movemask_epi8 (xmax) != 0)) goto scalar;
    #endif

    // Create the expansion vector
    xi128 xe = _mm_and_si128 (x2, xone);
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x3, xone));
    xe = _mm_add_epi16 (xe, _mm_and_si128 (x4, xone));

    // Find out how many bytes to expand each character
    // and create the shuffle vector
    xi128 x8e = _mm_packus_epi16 (_mm_packs_epi32 (xe, xz), xz);

    xi128 xsh = _mm_slli_si128 (x8e, 1);
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 1));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 2));
    xsh = _mm_add_epi8 (xsh, _mm_slli_si128 (xsh, 4));

    // Store shuffle positions in a high nibble
    xsh = _mm_add_epi8 (xsh, xn);

    // Get the number of output bytes to advance
    uint oa = (_mm_extract_epi16 (xsh, 2) & 0x3Fu) + 4u;

    #if T(SIZE)
      sz += oa;
    #else
      xsh = _mm_srli_si128 (_mm_slli_si128 (xsh, 12), 12);

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

      xi128 xshf = _mm_srli_epi64 (_mm_and_si128 (xsh, _mm_set1_epi8 (0xC0)), 6);
      xshf = _mm_or_si128 (xshf, _mm_cmpeq_epi8 (xshf, xz));
      xshf = _mm_slli_si128 (_mm_srli_si128 (xshf, 1), 1);

      // Get the ASCII bytes
      xi128 xb1 = _mm_andnot_si128 (x2, xi);

      x2 = _mm_xor_si128 (x2, x3);
      x3 = _mm_xor_si128 (x3, x4);

      // Get the 2-byte sequence suffix bytes
      xi128 xb2 = _mm_and_si128 (x2, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Get the 3-byte sequence suffix bytes
      xi128 xb3 = _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Get the 4-byte sequence suffix bytes
      xi128 xb4 = _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80));

      // Move 6 bits
      xi = _mm_srli_epi32 (xi, 6);

      // Get the 2-byte sequence prefix and remaining suffix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x2, _mm_or_si128 (xi, _mm_set1_epi32 (0xC0))));
      xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x3, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80)));
      xb3 = _mm_or_si128 (xb3, _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80)));

      // Move 6 bits
      xi = _mm_srli_epi32 (xi, 6);

      // Get the 3-byte sequence prefix and remaining suffix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x3, _mm_or_si128 (xi, _mm_set1_epi32 (0xE0))));
      xb2 = _mm_or_si128 (xb2, _mm_and_si128 (x4, _mm_or_si128 (_mm_and_si128 (xi, x3f), x80)));

      // Move 6 bits
      xi = _mm_srli_epi32 (xi, 6);

      // Get the 4-byte sequence prefix bytes
      xb1 = _mm_or_si128 (xb1, _mm_and_si128 (x4, _mm_or_si128 (xi, _mm_set1_epi32 (0xF0))));

      // Obtain the final UTF-8 bytes
      xi128 x8b1 = _mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb1, xz), xz), xshf);
      xi128 x8b2 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb2, xz), xz), xshf), 1);
      xi128 x8b3 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb3, xz), xz), xshf), 2);
      xi128 x8b4 = _mm_slli_si128 (_mm_shuffle_epi8 (_mm_packus_epi16 (_mm_packs_epi32 (xb4, xz), xz), xshf), 3);

      // Merge the results and store
      xi128 xr = _mm_or_si128 (_mm_or_si128 (x8b1, x8b2), _mm_or_si128 (x8b3, x8b4));

      _mm_storeu_si128 ((xi128*)o, xr);

      o += oa;
    #endif

    i += 4;
  #else
    // Back to scalar code since this vector contains non-ASCII characters.
    // Resort to scalar-only processing if this happens too often.
    a++;

    // Retry again after some time
    r = (a >= 4u) ? (i + 1024) : (i + 4);

    goto fallback;
  #endif
  }

  #if !T(EXPLICIT)
    if (likely (p[4] != '\0')) goto again;
  #endif

scalar:
  // Prevents return to SSE code when there are
  // less than 4 characters left in the input
  r = i + 4;

fallback:
#endif

#if T(EXPLICIT)
  while (i != e)
#else
  while (true)
#endif
  {
    register uf32 c = *i;

    // One ASCII byte
    if (likely (utf32_chr_is8_lead1 (c)))
    {
#if T(SIZE)
      sz++;
#else
      // Check if there's enough space in the output buffer
      if (unlikely (o == m))
      {
        n = 1;

needspace:
        *end = (u32*)i;
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
    }

    // Two UTF-8 bytes
    else if (likely (utf32_chr_is8_lead2 (c)))
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
    }

    // Three UTF-8 bytes
    else if (likely (utf32_chr_is8_lead3 (c)))
    {
#if T(VALID)
      // Check for UTF-16 surrogate character
      if (unlikely (utf16_chr_is_surr (c))) goto invalid;

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
    }

    // Four UTF-8 bytes
#if T(VALID)
    else if (unlikely (utf32_chr_is8_lead4 (c)))
#else
    else
#endif
    {
#if T(VALID)
      // Check for reserved Unicode character
      if (unlikely (utf16_chr_is_rsrv (c))) goto invalid;
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
      o[0] = utf8_codep_decomp_head4 (c);
      o[1] = utf8_codep_decomp_tail3 (c);
      o[2] = utf8_codep_decomp_tail2 (c);
      o[3] = utf8_codep_decomp_tail1 (c);

      o += 4;
#endif
    }

#if T(VALID)
    // Invalid character
    else
    {
invalid:
      *end = (u32*)i;

  #if T(SIZE)
      *num = sz;
  #else
      *num = (size_t)(o - out);
  #endif

      return INT_MIN;
    }
#endif

    i++;

#if CPU(SSE2)
    // Attempt to parse with SSE again
    if (likely (i >= r)) goto recover;
#endif
  }

  *end = (u32*)i;

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
