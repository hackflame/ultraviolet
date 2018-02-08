// =============================================================================
// <utf/utf16/bswap/sse.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

;
  // Align to 16-byte boundaries
  const u16* b = ptr_align_ceil (16u, s);

#if T(EXPLICIT)
  const u16* p = ptr_align_floor (16u, e);
  if (unlikely (b >= p)) goto outro;
#endif

  while (s != b)
  {
    register u16 c = *s;

#if !T(EXPLICIT)
    if (unlikely (c == (uint)'\0')) goto done;
#endif

    *s = bswap16 (c);
    s++;
  }

  // Process 8 characters at once using SSE
#if T(EXPLICIT)
  while (s != p)
#else
  while (true)
#endif
  {
    // Get the input vector
    xi128 x = _mm_load_si128 ((const xi128*)s);

#if !T(EXPLICIT)
    // Check for null-terminating character
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (x
    , _mm_setzero_si128())) != 0)) break;
#endif

    // Change the endianness and store the result vector back
    _mm_store_si128 ((xi128*)s, sse_bswap16 (x));

    s += 8u;
  }

outro:
