// =============================================================================
// <utf/utf32/length/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

;
  utf_align (u32, T_size_t, s, str, 16u);

  while (true)
  {
    xi128 x = _mm_load_si128 ((const xi128*)s);

    u32 m = _mm_movemask_epi8 (_mm_cmpeq_epi32 (x, _mm_setzero_si128()));

    if (unlikely (m != 0)) return (T_size_t)(s - str) + (bsf32 (m) / 4u);

    s += 4u;
  }
