// =============================================================================
// <utf/utf16/length/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

;
  utf_align (u16, T_size_t, s, str, 16u);

  while (true)
  {
    xi128 x = _mm_load_si128 ((const xi128*)s);

    u32 m = _mm_movemask_epi8 (_mm_cmpeq_epi16 (x, _mm_setzero_si128()));

    if (unlikely (m != 0)) return (T_size_t)(s - str) + (bsf32 (m) / 2u);

    s += 8u;
  }
