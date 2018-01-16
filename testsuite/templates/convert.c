// =============================================================================
// <convert.c>
//
// Conversion template.
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

if (true)
{
  if (eof)
  {
    off = insz % t_utf_in_sz;
    insz -= off;

    if (insz == 0) goto done;
  }

  t_utf_in* ptr = (t_utf_in*)in;
  insz /= t_utf_in_sz;

  if (bswp_in) t_utf_in_bswap_fn (ptr, insz);

  t_utf_in* pos;
  size_t outsz;

  size_t sz = insz;

  while (true)
  {
    int ret = t_utf_conv_fn (ptr, sz, (t_utf_out*)out
    , BUF_SIZE / t_utf_out_sz, &pos, &outsz);

    size_t n = (size_t)(pos - ptr);
    num += n;

    if (bswp_out) t_utf_out_bswap_fn ((t_utf_out*)out, outsz);
    if (fwrite ((t_utf_out*)out, t_utf_out_sz, outsz, stdout) != outsz) error_io();

    if (ret < 0)
    {
      if (ret == INT_MIN) error_input (num * t_utf_in_sz);

      n = sz - n;
      insz -= n;

      off += n * t_utf_in_sz;
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

  insz *= t_utf_in_sz;
}

// -----------------------------------------------------------------------------

#undef t_utf_in
#undef t_utf_in_sz
#undef t_utf_in_bswap_fn

#undef t_utf_out
#undef t_utf_out_sz
#undef t_utf_out_bswap_fn

#undef t_utf_conv_fn
