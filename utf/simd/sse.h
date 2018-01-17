// =============================================================================
// <utf/simd/sse.h>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#ifndef H_6A522E65249A4CFA8F1430B1FF398E90
#define H_6A522E65249A4CFA8F1430B1FF398E90

// -----------------------------------------------------------------------------

#if CPU(SSE2)
  #define T_UTF8_LENGTH_SIMD "length/sse.c"
  #define T_UTF8_COUNT_SIMD  "utf8/sse.c"
  #define T_UTF8_TO16_SIMD   "utf16/sse.c"
  #define T_UTF8_TO32_SIMD   "utf32/sse.c"

  #define T_UTF16_LENGTH_SIMD "length/sse.c"
  #define T_UTF16_BSWAP_SIMD  "bswap/sse.c"
  #define T_UTF16_COUNT_SIMD  "utf16/sse.c"
  #define T_UTF16_TO8_SIMD    "utf8/sse.c"
  #define T_UTF16_TO32_SIMD   "utf32/sse.c"

  #define T_UTF32_LENGTH_SIMD "length/sse.c"
  #define T_UTF32_BSWAP_SIMD  "bswap/sse.c"
  #define T_UTF32_COUNT_SIMD  "utf32/sse.c"
  #define T_UTF32_TO8_SIMD    "utf8/sse.c"
  #define T_UTF32_TO16_SIMD   "utf16/sse.c"
#endif

// -----------------------------------------------------------------------------

#endif
