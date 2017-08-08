// =============================================================================
// <utf32/utf32bswap.c>
//
// UTF-32 byte-swap template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  u32* restrict s = str;

#if T(EXPLICIT)
  const u32* restrict e = str + len;
#endif

#if CPU(SSE2)
  // Align to 16-byte boundaries
  const u32* restrict b = ptr_align_ceil (16u, s);

  #if T(EXPLICIT)
    const u32* restrict p = ptr_align_floor (16u, e);
    if (unlikely (b >= p)) goto outro;
  #endif

  while (s != b)
  {
    register u32 c = *s;

  #if !T(EXPLICIT)
    if (unlikely (c == '\0')) goto done;
  #endif

    *s++ = bswap32 (c);
  }

  // Process 4 characters at once using SSE
  #if T(EXPLICIT)
    while (s != p)
  #else
    while (true)
  #endif
  {
    // Get the input vector
    xi128 xs = _mm_load_si128 ((const xi128*)s);

  #if !T(EXPLICIT)
    // Check for null-terminating character
    if (likely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xs, _mm_setzero_si128())) != 0)) break;
  #endif

    // Change the endianness and store the result vector back
    _mm_store_si128 ((xi128*)s, sse_bswap32 (xs));

    s += 4;
  }

outro:
#endif

#if T(EXPLICIT)
  while (s != e)
#else
  while (true)
#endif
  {
    register u32 c = *s;

#if !T(EXPLICIT)
    if (unlikely (c == '\0')) break;
#endif

    *s++ = bswap32 (c);
  }

done:;
#if !T(EXPLICIT)
  return s;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
