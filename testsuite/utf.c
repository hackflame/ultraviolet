// =============================================================================
// <testsuite/utf.c>
//
// Copyright Kristian Garnét.
// -----------------------------------------------------------------------------

#include <quantum/build.h>

#if OS(WIN32)
  #include <Windows.h>
#endif

#include <quantum/core.h>

#include <quantum/main.h>
#include <quantum/integer.h>
#include <quantum/string.h>
#include <quantum/io.h>

#include <utf.h>

// =============================================================================
// Constants
// -----------------------------------------------------------------------------

#if !BUF_SIZE
  #define BUF_SIZE 4096u
#endif

#if (BUF_SIZE < 256u) || (BUF_SIZE % 4u)
  #error "Invalid definition of `BUF_SIZE`"
#endif

// -----------------------------------------------------------------------------

#define ARG_IN   "-i"
#define ARG_OUT  "-o"
#define ARG_BOM  "-b"
#define ARG_HELP "-h"

#define ARG_ENC_UTF8    "utf-8"
#define ARG_ENC_UTF16   "utf-16"
#define ARG_ENC_UTF16LE "utf-16le"
#define ARG_ENC_UTF16BE "utf-16be"
#define ARG_ENC_UTF32   "utf-32"
#define ARG_ENC_UTF32LE "utf-32le"
#define ARG_ENC_UTF32BE "utf-32be"

#define ARG_BOM_AUTO "auto"
#define ARG_BOM_KEEP "keep"
#define ARG_BOM_YES  "yes"
#define ARG_BOM_NO   "no"

// =============================================================================
// Types
// -----------------------------------------------------------------------------

typedef enum utf_bom_e
{
  bom_auto,
  bom_keep,
  bom_yes,
  bom_no
} utf_bom_t;

typedef enum utf_enc_e
{
  utf_auto,
  utf_none = utf_auto,
  utf8,
  utf16,
  utf32
} utf_enc_t;

typedef enum utf_conv_e
{
  utf8_to16,
  utf8_to32,
  utf16_to8,
  utf16_to32,
  utf32_to8,
  utf32_to16
} utf_conv_t;

// =============================================================================
// Globals
// -----------------------------------------------------------------------------

static u8 bom_buf[4];
static size_t bom_off;
static bool bom_eof;

// =============================================================================
// Functions
// -----------------------------------------------------------------------------
// Error handlers
// -----------------------------------------------------------------------------

static noreturn void error_io (void)
{
  fprintf (stderr, "%s\n", "I/O error");
  exit (exit_err);
}

static noreturn void error_input (size_t pos)
{
  fprintf (stderr, "%s [%" PRIuSIZE "]\n", "Invalid input", pos);
  exit (exit_err);
}

static noreturn void error_unknown (void)
{
  fprintf (stderr, "%s\n", "Unknown error");
  exit (exit_err);
}

// -----------------------------------------------------------------------------
// Validate the input
static void validate (const utf_enc_t enc, bool bswp_in, bool bswp_out
, const bool copy)
{
  u8 in[BUF_SIZE];

  size_t num = 0;
  size_t off = bom_off;

  bool eof = false;

  if (bom_off != 0) buf_copy (in, bom_buf, bom_off);

  while (true)
  {
    size_t size = BUF_SIZE - off;
    size_t insz = io_file_read (stdin, in + off, size);

    if (insz != size)
    {
      if (!io_file_eof (stdin)) error_io();
      if ((insz == 0) && !bom_eof) goto done;
      eof = true;
    }

    insz += off;
    off = 0;

    if (enc == utf8)
    {
      #define T_utf_in_t u8
      #define T_utf_in_sz 1u
      #define T_utf_in_bswap_fun fun_nop_va

      #define T_utf_valid_fun utf8_str_valid

      #include "templates/validate.c"
    }
    elif (enc == utf16)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fun utf16_str_bswap

      #define T_utf_valid_fun utf16_str_valid

      #include "templates/validate.c"
    }
    elif (enc == utf32)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fun utf32_str_bswap

      #define T_utf_valid_fun(in, len, end, runes) utf32_str_valid (in, len, end)

      #include "templates/validate.c"
    }

    if (eof)
    {
done:
      if (off != 0) error_input (num);
      break;
    }

    if (off != 0) buf_move (in, in + insz, off);
  }
}

// -----------------------------------------------------------------------------
// Convert to another UTF encoding validating the input
static void convert (const utf_conv_t conv, bool bswp_in, bool bswp_out)
{
  u8 in[BUF_SIZE];
  u8 out[BUF_SIZE];

  size_t num = 0;
  size_t off = bom_off;

  bool eof = false;

  if (bom_off != 0) buf_copy (in, bom_buf, bom_off);

  while (true)
  {
    size_t size = BUF_SIZE - off;
    size_t insz = io_file_read (stdin, in + off, size);

    if (insz != size)
    {
      if (!io_file_eof (stdin)) error_io();
      if ((insz == 0) && !bom_eof) goto done;
      eof = true;
    }

    insz += off;
    off = 0;

    if (conv == utf8_to16)
    {
      #define T_utf_in_t u8
      #define T_utf_in_sz 1u
      #define T_utf_in_bswap_fun fun_nop_va

      #define T_utf_out_t u16
      #define T_utf_out_sz 2u
      #define T_utf_out_bswap_fun utf16_str_bswap

      #define T_utf_conv_fun utf8_str_to16

      #include "templates/convert.c"
    }
    elif (conv == utf8_to32)
    {
      #define T_utf_in_t u8
      #define T_utf_in_sz 1u
      #define T_utf_in_bswap_fun fun_nop_va

      #define T_utf_out_t u32
      #define T_utf_out_sz 4u
      #define T_utf_out_bswap_fun utf32_str_bswap

      #define T_utf_conv_fun utf8_str_to32

      #include "templates/convert.c"
    }
    elif (conv == utf16_to8)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fun utf16_str_bswap

      #define T_utf_out_t u8
      #define T_utf_out_sz 1u
      #define T_utf_out_bswap_fun fun_nop_va

      #define T_utf_conv_fun utf16_str_to8

      #include "templates/convert.c"
    }
    elif (conv == utf16_to32)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fun utf16_str_bswap

      #define T_utf_out_t u32
      #define T_utf_out_sz 4u
      #define T_utf_out_bswap_fun utf32_str_bswap

      #define T_utf_conv_fun utf16_str_to32

      #include "templates/convert.c"
    }
    elif (conv == utf32_to8)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fun utf32_str_bswap

      #define T_utf_out_t u8
      #define T_utf_out_sz 1u
      #define T_utf_out_bswap_fun fun_nop_va

      #define T_utf_conv_fun utf32_str_to8

      #include "templates/convert.c"
    }
    elif (conv == utf32_to16)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fun utf32_str_bswap

      #define T_utf_out_t u16
      #define T_utf_out_sz 2u
      #define T_utf_out_bswap_fun utf16_str_bswap

      #define T_utf_conv_fun utf32_str_to16

      #include "templates/convert.c"
    }

    if (eof)
    {
done:
      if (off != 0) error_input (num);
      break;
    }

    if (off != 0) buf_move (in, in + insz, off);
  }

  return;
}

// -----------------------------------------------------------------------------
// Write the byte order mark (BOM)
static void mark (utf_enc_t enc, bool bswp_out)
{
  if (enc == utf8)
  {
    if (io_file_write (stdout, utf8_bom, 3u) != 3u) error_io();
  }
  elif (enc == utf16)
  {
    u16 bom = utf16_bom;

    if (bswp_out) bom = bswap16 (bom);

    if (io_file_write (stdout, (u8*)&bom, 2u) != 2u) error_io();
  }
  elif (enc == utf32)
  {
    u32 bom = utf32_bom;

    if (bswp_out) bom = bswap32 (bom);

    if (io_file_write (stdout, (u8*)&bom, 4u) != 4u) error_io();
  }
}

// =============================================================================

int quantum (const main_t* restrict argm, char_t** restrict argv, uint argc)
{
  // Parse command line arguments
  char* prog = *argv;
  bool help = false;

  if ((argc < 1u) || (argc > 5u))
  {
error_args:
    fprintf (stderr, "%s\n", "Invalid arguments");
    putchar ('\n');

error_help:
    fprintf (stderr, "Usage: %s -i auto -o utf-8 -b auto%s < input > output\n"
    , prog, help ? "" : " -h");

    // Print detailed usage instructions
    if (help)
    {
      putchar ('\n');
      fprintf (stderr,
"-i: input encoding (optional).\n"
"\n"
"    auto: attempt to detect the input encoding by looking\n"
"    at its byte order mark (BOM) if it's present.\n"
"    Assume UTF-8 otherwise.\n"
"\n"
"    utf-8:    UTF-8.\n"
"\n"
"    utf-16:   UTF-16 (host endianness).\n"
"    utf-16le: UTF-16 (little-endian).\n"
"    utf-16be: UTF-16 (big-endian).\n"
"\n"
"    utf-32:   UTF-32 (host endianness).\n"
"    utf-32le: UTF-32 (little-endian).\n"
"    utf-32be: UTF-32 (big-endian).\n"
"\n"
"-o: output encoding (optional).\n"
"\n"
"    none:     validate the input only.\n"
"\n"
"    utf-8:    UTF-8.\n"
"\n"
"    utf-16:   UTF-16 (host endianness).\n"
"    utf-16le: UTF-16 (little-endian).\n"
"    utf-16be: UTF-16 (big-endian).\n"
"\n"
"    utf-32:   UTF-32 (host endianness).\n"
"    utf-32le: UTF-32 (little-endian).\n"
"    utf-32be: UTF-32 (big-endian).\n"
"\n"
"-b: byte order mark policy (optional).\n"
"\n"
"    auto: omit BOM for UTF-8 output, write otherwise.\n"
"    keep: write UTF-8 BOM if it was present in the input.\n"
"    yes:  force writing of BOM.\n"
"    no:   discard BOM.\n");
    }

    return help ? exit_ok : exit_err;
  }

  utf_enc_t in = utf_auto;
  bool bswp_in = false;

  utf_enc_t out = utf_none;
  bool bswp_out = false;

  utf_bom_t bom = bom_auto;

  argc--;
  argv++;

  while (argc)
  {
    // Input encoding
    if (istr_iequal (*argv, ARG_IN))
    {
      argc--;
      argv++;

      if (argc == 0) goto error_args;

      // UTF-8
      if (istr_iequal (*argv, ARG_ENC_UTF8)) in = utf8;
      // UTF-16 (host endianness)
      elif (istr_iequal (*argv, ARG_ENC_UTF16)) in = utf16;
      // UTF-16 (little-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF16LE))
      {
        in = utf16;

#if CPU(BIG_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-16 (big-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF16BE))
      {
        in = utf16;

#if CPU(LITTLE_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-32 (host endianness)
      elif (istr_iequal (*argv, ARG_ENC_UTF32)) in = utf32;
      // UTF-32 (little-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF32LE))
      {
        in = utf32;

#if CPU(BIG_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-32 (big-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF32BE))
      {
        in = utf32;

#if CPU(LITTLE_ENDIAN)
        bswp_in = true;
#endif
      }
      // Autodetect
      elif (istr_iequal (*argv, ARG_BOM_AUTO)) in = utf_auto;
      else goto error_args;
    }
    // Output encoding
    elif (istr_iequal (*argv, ARG_OUT))
    {
      argc--;
      argv++;

      if (argc == 0) goto error_args;

      // UTF-8
      if (istr_iequal (*argv, ARG_ENC_UTF8)) out = utf8;
      // UTF-16 (host endianness)
      elif (istr_iequal (*argv, ARG_ENC_UTF16)) out = utf16;
      // UTF-16 (little-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF16LE))
      {
        out = utf16;

#if CPU(BIG_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-16 (big-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF16BE))
      {
        out = utf16;

#if CPU(LITTLE_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-32 (host endianness)
      elif (istr_iequal (*argv, ARG_ENC_UTF32)) out = utf32;
      // UTF-32 (little-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF32LE))
      {
        out = utf32;

#if CPU(BIG_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-32 (big-endian)
      elif (istr_iequal (*argv, ARG_ENC_UTF32BE))
      {
        out = utf32;

#if CPU(LITTLE_ENDIAN)
        bswp_out = true;
#endif
      }
      else goto error_args;
    }
    // Byte order mark policy
    elif (istr_iequal (*argv, ARG_BOM))
    {
      argc--;
      argv++;

      if (argc != 0)
      {
        // Auto: write for all encodings except UTF-8
        if (istr_iequal (*argv, ARG_BOM_AUTO)) bom = bom_auto;
        // Keep: write if was present in the input UTF-8
        elif (istr_iequal (*argv, ARG_BOM_KEEP)) bom = bom_keep;
        // Do write
        elif (istr_iequal (*argv, ARG_BOM_YES)) bom = bom_yes;
        // Don't write
        elif (istr_iequal (*argv, ARG_BOM_NO)) bom = bom_no;
        else
        {
          bom = bom_yes;
          continue;
        }
      }
      else
      {
        bom = bom_yes;
        break;
      }
    }
    // Help
    elif (istr_iequal (*argv, ARG_HELP))
    {
      help = true;
      goto error_help;
    }
    else goto error_args;

    argc--;
    argv++;
  }

  // Check for incorrect `bom_keep` usage
  if ((out != 0) && (out != utf8) && (bom == bom_keep)) goto error_args;

#if OS(WIN32)
  // Put both streams into binary mode
  if (!io_file_flush (io_file_stdin)) error_io();
  if (setmode (io_fd_stdin, O_BINARY) == -1) error_io();

  if (!io_file_flush (io_file_stdout)) error_io();
  if (setmode (io_fd_stdout, O_BINARY) == -1) error_io();
#endif

  // Detect the input encoding
  if (in == 0)
  {
    size_t num = io_file_read (stdin, bom_buf, 4u);

    if (num != 4u)
    {
      if (!io_file_eof (stdin)) error_io();
      bom_eof = true;
    }

    switch (num)
    {
      // Try UTF-32
      case 4:
      {
        if (*(u32*)bom_buf == utf32_bom) in = utf32;
        elif (bswap32 (*(u32*)bom_buf) == utf32_bom)
        {
          in = utf32;
          bswp_in = true;
        }
        else goto try_utf8;

        break;
      }
      // Try UTF-8
      case 3:
      {
try_utf8:
        if (buf_equal (bom_buf, utf8_bom, 3u))
        {
          in = utf8;

          if (num != 3u)
          {
            bom_off = 1u;
            buf_move (bom_buf, bom_buf + 3u, 1u);
          }

          if ((out == utf8) && (bom == bom_keep)) bom = bom_yes;
        }
        else goto try_utf16;

        break;
      }
      // Try UTF-16
      case 2:
      {
try_utf16:
        if (*(u16*)bom_buf == utf16_bom)
        {
          in = utf16;

          if (num != 2u)
          {
            bom_off = num - 2u;
            buf_move (bom_buf, bom_buf + 2u, bom_off);
          }
        }
        elif (bswap16 (*(u16*)bom_buf) == utf16_bom)
        {
          in = utf16;
          bswp_in = true;

          if (num != 2u)
          {
            bom_off = num - 2u;
            buf_move (bom_buf, bom_buf + 2u, bom_off);
          }
        }
        else goto assume_utf8;

        break;
      }
      // Assume UTF-8
      case 1:
      case 0:
      {
assume_utf8:
        in = utf8;
        bom_off = num;
      }
    }
  }

  // Write the output BOM
  if (out != 0)
  {
    if ((out != utf8) && (bom == bom_auto)) bom = bom_yes;
    if (bom == bom_yes) mark (out, (out == utf8) ? false : bswp_out);
  }

  // Validate only?
  bool copy = false;

  if (in == out)
  {
    // Copy as is
    copy = true;
    out = 0;
  }

  if (out == 0)
  {
    if (in == utf8) validate (utf8, false, false, copy);
    elif (in == utf16) validate (utf16, bswp_in, bswp_out, copy);
    elif (in == utf32) validate (utf32, bswp_in, bswp_out, copy);
  }

  // Validate and convert
  else
  {
    if ((in == utf8) && (out == utf16)) convert (utf8_to16, false, bswp_out);
    elif ((in == utf8) && (out == utf32)) convert (utf8_to32, false, bswp_out);
    elif ((in == utf16) && (out == utf8)) convert (utf16_to8, bswp_in, false);
    elif ((in == utf16) && (out == utf32)) convert (utf16_to32, bswp_in, bswp_out);
    elif ((in == utf32) && (out == utf8)) convert (utf32_to8, bswp_in, false);
    elif ((in == utf32) && (out == utf16)) convert (utf32_to16, bswp_in, bswp_out);
  }

done:
  return exit_ok;
}

// -----------------------------------------------------------------------------

#include <quantum/main.c>
