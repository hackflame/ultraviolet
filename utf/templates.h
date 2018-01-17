// =============================================================================
// <utf/templates.h>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#ifndef H_AE4A9FEAC1F842A89432D968954C66B9
#define H_AE4A9FEAC1F842A89432D968954C66B9

// -----------------------------------------------------------------------------

#define utf_length(t_size, ptr, str) do\
{                                      \
  while ((uint)*(ptr) != (uint)'\0') (ptr)++;\
  return (t_size)((ptr) - (str));\
}       \
while (0)

#define utf_align(t_chr, t_size, ptr, str, align) do\
{                                                   \
  const t_chr* pos = ptr_align_ceil ((align), (str));\
                                                    \
  while ((ptr) != (pos))                            \
  {                                                 \
    if (unlikely ((uint)*(ptr) == (uint)'\0')) return (t_size)((ptr) - (str));\
    (ptr)++;\
  }     \
}       \
while (0)

// -----------------------------------------------------------------------------

#endif
