// =============================================================================
// <utf16/utf16bswap.c>
//
// UTF-16 byte-swap template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  u16* restrict s = str;

#if T(EXPLICIT)
  const u16* restrict e = str + len;
#endif

#if CPU(SSE2)
  // Align to 16-byte boundaries
  const u16* restrict b = ptr_align_ceil (16u, s);

  #if T(EXPLICIT)
    const u16* restrict p = ptr_align_floor (16u, e);
    if (unlikely (b >= p)) goto outro;
  #endif

  while (s != b)
  {
    register u16 c = *s;

  #if !T(EXPLICIT)
    if (unlikely (c == '\0')) goto done;
  #endif

    *s++ = bswap16 (c);
  }

  // Process 8 characters at once using SSE
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
    if (unlikely (_mm_movemask_epi8 (_mm_cmpeq_epi16 (xs, _mm_setzero_si128())) != 0)) break;
  #endif

    // Change the endianness and store the result vector back
    _mm_store_si128 ((xi128*)s, sse_bswap16 (xs));

    s += 8;
  }

outro:
#endif

#if T(EXPLICIT)
  while (s != e)
#else
  while (true)
#endif
  {
    register u16 c = *s;

#if !T(EXPLICIT)
    if (unlikely (c == '\0')) break;
#endif

    *s++ = bswap16 (c);
  }

done:;
#if !T(EXPLICIT)
  return s;
#endif
}

// -----------------------------------------------------------------------------

#undef T_EXPLICIT
