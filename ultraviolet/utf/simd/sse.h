// =============================================================================
// <ultraviolet/utf/simd/sse.h>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------------

#if CPU_SSE2
  #define T_UTF8_COUNT_SIMD 1
  #define T_UTF8_TO16_SIMD  1
  #define T_UTF8_TO32_SIMD  1

  #define T_UTF16_BSWAP_SIMD 1
  #define T_UTF16_COUNT_SIMD 1
  #define T_UTF16_TO8_SIMD   1
  #define T_UTF16_TO32_SIMD  1

  #define T_UTF32_BSWAP_SIMD 1
  #define T_UTF32_COUNT_SIMD 1
  #define T_UTF32_TO8_SIMD   1
  #define T_UTF32_TO16_SIMD  1
#endif
