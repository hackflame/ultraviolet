// =============================================================================
// <testsuite/templates/validate.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

{
  if (eof)
  {
    off = insz % T_utf_in_sz;
    insz -= off;

    if (insz == 0) goto done;
  }

  insz /= T_utf_in_sz;

  if (bswp_in)
  {
    T_utf_in_bswap_fn ((T_utf_in_t*)in, insz);
  }

  T_utf_in_t* pos;

#if (T_utf_in_sz != 4u)
  size_t runes;
#endif

  int ret = T_utf_valid_fn ((T_utf_in_t*)in, insz, &pos, &runes);

  size_t n = (size_t)(pos - (T_utf_in_t*)in);
  num += n;

  if (ret < 0)
  {
    if (ret == INT_MIN) error_input (num * T_utf_in_sz);

    n = insz - n;
    insz -= n;

    off += n * T_utf_in_sz;
  }

  if (copy)
  {
    if (bswp_out)
    {
      T_utf_in_bswap_fn ((T_utf_in_t*)in, insz);
    }

    insz *= T_utf_in_sz;

    if (io_file_write (stdout, (T_utf_in_t*)in, insz) != insz) error_io();
  }
  else insz *= T_utf_in_sz;
}

// -----------------------------------------------------------------------------

#undef T_utf_in_t
#undef T_utf_in_sz
#undef T_utf_in_bswap_fn

#undef T_utf_valid_fn
