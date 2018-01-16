// =============================================================================
// <validate.c>
//
// Validation template.
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

  insz /= t_utf_in_sz;

  if (bswp_in) t_utf_in_bswap_fn ((t_utf_in*)in, insz);

  t_utf_in* pos;
  size_t runes;

  int ret = t_utf_valid_fn ((t_utf_in*)in, insz, &pos, &runes);

  size_t n = (size_t)(pos - (t_utf_in*)in);
  num += n;

  if (ret < 0)
  {
    if (ret == INT_MIN) error_input (num * t_utf_in_sz);

    n = insz - n;
    insz -= n;

    off += n * t_utf_in_sz;
  }

  if (copy)
  {
    if (bswp_out) t_utf_in_bswap_fn ((t_utf_in*)in, insz);
    if (fwrite ((t_utf_in*)in, t_utf_in_sz, insz, stdout) != insz) error_io();
  }

  insz *= t_utf_in_sz;
}

// -----------------------------------------------------------------------------

#undef t_utf_in
#undef t_utf_in_sz
#undef t_utf_in_bswap_fn

#undef t_utf_valid_fn
