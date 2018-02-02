// =============================================================================
// <testsuite/templates/convert.c>
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

  T_utf_in_t* ptr = (T_utf_in_t*)in;
  insz /= T_utf_in_sz;

  if (bswp_in) T_utf_in_bswap_fun (ptr, insz);

  T_utf_in_t* pos;
  size_t outsz;

  size_t sz = insz;

  while (true)
  {
    int ret = T_utf_conv_fun (ptr, sz, (T_utf_out_t*)out
    , BUF_SIZE / T_utf_out_sz, &pos, &outsz);

    size_t n = (size_t)(pos - ptr);
    num += n;

    if (bswp_out) T_utf_out_bswap_fun ((T_utf_out_t*)out, outsz);

    outsz *= T_utf_out_sz;

    if (io_file_write (stdout, (T_utf_out_t*)out, outsz) != outsz) error_io();

    if (ret < 0)
    {
      if (ret == INT_MIN) error_input (num * T_utf_in_sz);

      n = sz - n;
      insz -= n;

      off += n * T_utf_in_sz;
    }
    else if (ret > 0)
    {
      if (ret == INT_MAX) error_unknown();

      sz -= n;
      ptr = pos;

      continue;
    }

    break;
  }

  insz *= T_utf_in_sz;
}

// -----------------------------------------------------------------------------

#undef T_utf_in_t
#undef T_utf_in_sz
#undef T_utf_in_bswap_fun

#undef T_utf_out_t
#undef T_utf_out_sz
#undef T_utf_out_bswap_fun

#undef T_utf_conv_fun
