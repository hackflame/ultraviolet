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

#include <quantum/integer.h>
#include <quantum/string.h>
#include <quantum/io.h>
#include <quantum/xchar.h>

#if OS(WIN32)
  #include <quantum/windows.h>
#endif

#include <utf.h>

// =============================================================================
// Constants
// -----------------------------------------------------------------------------

#if (BUF_SIZE == 0)
  #define BUF_SIZE 4096u
#endif

#if (BUF_SIZE < 256) || (BUF_SIZE % 4)
  #error "Invalid definition of `BUF_SIZE`"
#endif

// -----------------------------------------------------------------------------

#define ARG_IN  "-in="
#define ARG_OUT "-out="

#define ARG_ENC_UTF8    "utf-8"
#define ARG_ENC_UTF16   "utf-16"
#define ARG_ENC_UTF16LE "utf-16le"
#define ARG_ENC_UTF16BE "utf-16be"
#define ARG_ENC_UTF32   "utf-32"
#define ARG_ENC_UTF32LE "utf-32le"
#define ARG_ENC_UTF32BE "utf-32be"

#define ARG_BOM "-bom"

#define ARG_BOM_AUTO "=auto"
#define ARG_BOM_KEEP "=keep"
#define ARG_BOM_YES  "=yes"
#define ARG_BOM_NO   "=no"

#define ARG_HELP "-help"

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
  fprintf (stderr, "%s.\n", "I/O error");
  exit (EXIT_FAILURE);
}

static noreturn void error_input (size_t pos)
{
  fprintf (stderr, "%s [%" PRIuSIZE "].\n", "Invalid input", pos);
  exit (EXIT_FAILURE);
}

static noreturn void error_unknown (void)
{
  fprintf (stderr, "%s.\n", "Unknown error");
  exit (EXIT_FAILURE);
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
      #define T_utf_in_bswap_fn fn_nop_va

      #define T_utf_valid_fn utf8_str_valid

      #include "templates/validate.c"
    }
    elif (enc == utf16)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fn utf16_str_bswap

      #define T_utf_valid_fn utf16_str_valid

      #include "templates/validate.c"
    }
    elif (enc == utf32)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fn utf32_str_bswap

      #define T_utf_valid_fn(in, len, end, runes) utf32_str_valid (in, len, end)

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
      #define T_utf_in_bswap_fn fn_nop_va

      #define T_utf_out_t u16
      #define T_utf_out_sz 2u
      #define T_utf_out_bswap_fn utf16_str_bswap

      #define T_utf_conv_fn utf8_str_to16

      #include "templates/convert.c"
    }
    elif (conv == utf8_to32)
    {
      #define T_utf_in_t u8
      #define T_utf_in_sz 1u
      #define T_utf_in_bswap_fn fn_nop_va

      #define T_utf_out_t u32
      #define T_utf_out_sz 4u
      #define T_utf_out_bswap_fn utf32_str_bswap

      #define T_utf_conv_fn utf8_str_to32

      #include "templates/convert.c"
    }
    elif (conv == utf16_to8)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fn utf16_str_bswap

      #define T_utf_out_t u8
      #define T_utf_out_sz 1u
      #define T_utf_out_bswap_fn fn_nop_va

      #define T_utf_conv_fn utf16_str_to8

      #include "templates/convert.c"
    }
    elif (conv == utf16_to32)
    {
      #define T_utf_in_t u16
      #define T_utf_in_sz 2u
      #define T_utf_in_bswap_fn utf16_str_bswap

      #define T_utf_out_t u32
      #define T_utf_out_sz 4u
      #define T_utf_out_bswap_fn utf32_str_bswap

      #define T_utf_conv_fn utf16_str_to32

      #include "templates/convert.c"
    }
    elif (conv == utf32_to8)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fn utf32_str_bswap

      #define T_utf_out_t u8
      #define T_utf_out_sz 1u
      #define T_utf_out_bswap_fn fn_nop_va

      #define T_utf_conv_fn utf32_str_to8

      #include "templates/convert.c"
    }
    elif (conv == utf32_to16)
    {
      #define T_utf_in_t u32
      #define T_utf_in_sz 4u
      #define T_utf_in_bswap_fn utf32_str_bswap

      #define T_utf_out_t u16
      #define T_utf_out_sz 2u
      #define T_utf_out_bswap_fn utf16_str_bswap

      #define T_utf_conv_fn utf32_str_to16

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

// -----------------------------------------------------------------------------

int xmain (int argc, xchar_t** xargv)
{
#if OS(WIN32) && defined(DEBUG)
  SetErrorMode (SetErrorMode (0) | SEM_NOGPFAULTERRORBOX);
#endif

  chr_t* argvv[argc];
  chr_t* argv8 = ixstr_to_utf8_set_mngd (argvv, (const xchr_t* const*)xargv, argc);

  if (argv8 == null)
  {
#if OS(WIN32)
    win_error_print ("UTF-8");
#endif

    return EXIT_FAILURE;
  }

  chr_t** argv = argvv;

  // Parse command line arguments
  char* app = *argv;
  bool help = false;

  if ((argc < 1) || (argc > 5))
  {
error_args:
    fprintf (stderr, "%s.\n", "Invalid arguments");
    putchar ('\n');

error_help:
    fprintf (stderr, "Usage: %s -in=auto -out=utf-8 -bom=auto%s < input > output\n"
    , app, help ? "" : " -help");

    // Print detailed usage instructions
    if (help) putchar ('\n');
    if (help) fprintf (stderr,
"-in: input encoding (optional).\n"
"\n"
"    auto: attempt to detect the input encoding by looking\n"
"    at the byte order mark (BOM), if it's present.\n"
"    Assume UTF-8 if it's not present.\n"
"\n"
"    utf-8:    the input is encoded in UTF-8.\n"
"    utf-16:   the input is encoded in UTF-16 (host endianness).\n"
"    utf-16le: the input is encoded in UTF-16 (little-endian).\n"
"    utf-16be: the input is encoded in UTF-16 (big-endian).\n"
"    utf-32:   the input is encoded in UTF-32 (host endianness).\n"
"    utf-32le: the input is encoded in UTF-32 (little-endian).\n"
"    utf-32be: the input is encoded in UTF-32 (big-endian).\n"
"\n"
"-out: output encoding (optional).\n"
"\n"
"    none:     validate the input only.\n"
"    utf-8:    encode the output in UTF-8.\n"
"    utf-16:   encode the output in UTF-16 (host endianness).\n"
"    utf-16le: encode the output in UTF-16 (little-endian).\n"
"    utf-16be: encode the output in UTF-16 (big-endian).\n"
"    utf-32:   encode the output in UTF-32 (host endianness).\n"
"    utf-32le: encode the output in UTF-32 (little-endian).\n"
"    utf-32be: encode the output in UTF-32 (big-endian).\n"
"\n"
"-bom: byte order mark policy (optional).\n"
"\n"
"    auto: omit BOM for UTF-8 output, write otherwise.\n"
"    keep: write BOM in the output if it was present in the input (UTF-8 only).\n"
"    yes:  force writing of BOM.\n"
"    no:   discard BOM.\n"
    );

    return help ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  utf_enc_t in = utf_auto;
  bool bswp_in = false;

  utf_enc_t out = utf_none;
  bool bswp_out = false;

  utf_bom_t bom = bom_auto;

  argc--;
  argv++;

  while (argc--)
  {
    // Input encoding
    if (istr_niequal (*argv, ARG_IN, cstrlen (ARG_IN)))
    {
      // UTF-8
      if (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF8)) in = utf8;
      // UTF-16 (system endianness)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF16)) in = utf16;
      // UTF-16 (little-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF16LE))
      {
        in = utf16;

#if CPU(BIG_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-16 (big-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF16BE))
      {
        in = utf16;

#if CPU(LITTLE_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-32 (system endianness)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF32)) in = utf32;
      // UTF-32 (little-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF32LE))
      {
        in = utf32;

#if CPU(BIG_ENDIAN)
        bswp_in = true;
#endif
      }
      // UTF-32 (big-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_ENC_UTF32BE))
      {
        in = utf32;

#if CPU(LITTLE_ENDIAN)
        bswp_in = true;
#endif
      }
      // Autodetect
      elif (istr_iequal (*argv + cstrlen (ARG_IN), ARG_BOM_AUTO)) in = utf_auto;
      else goto error_args;
    }
    // Output encoding
    elif (istr_niequal (*argv, ARG_OUT, cstrlen (ARG_OUT)))
    {
      // UTF-8
      if (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF8)) out = utf8;
      // UTF-16 (system endianness)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF16)) out = utf16;
      // UTF-16 (little-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF16LE))
      {
        out = utf16;

#if CPU(BIG_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-16 (big-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF16BE))
      {
        out = utf16;

#if CPU(LITTLE_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-32 (system endianness)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF32)) out = utf32;
      // UTF-32 (little-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF32LE))
      {
        out = utf32;

#if CPU(BIG_ENDIAN)
        bswp_out = true;
#endif
      }
      // UTF-32 (big-endian)
      elif (istr_iequal (*argv + cstrlen (ARG_OUT), ARG_ENC_UTF32BE))
      {
        out = utf32;

#if CPU(LITTLE_ENDIAN)
        bswp_out = true;
#endif
      }
      else goto error_args;
    }
    // Byte order mark policy
    elif (istr_niequal (*argv, ARG_BOM, cstrlen (ARG_BOM)))
    {
      // Auto: write for all encodings except UTF-8
      if (istr_iequal (*argv + cstrlen (ARG_BOM), ARG_BOM_AUTO)) bom = bom_auto;
      // Keep: write if was present in the input
      elif (istr_iequal (*argv + cstrlen (ARG_BOM), ARG_BOM_KEEP)) bom = bom_keep;
      // Do write
      elif (istr_iequal (*argv + cstrlen (ARG_BOM), ARG_BOM_YES)) bom = bom_yes;
      elif (istr_length (*argv) == cstrlen (ARG_BOM)) bom = bom_yes;
      // Don't write
      elif (istr_iequal (*argv + cstrlen (ARG_BOM), ARG_BOM_NO)) bom = bom_no;
      else goto error_args;
    }
    // Help
    elif (istr_iequal (*argv, ARG_HELP))
    {
      help = true;
      goto error_help;
    }
    else goto error_args;

    argv++;
  }

  // Check for incorrect `bom_keep` usage
  if ((out != 0) && (out != utf8) && (bom == bom_keep)) goto error_args;

#if OS(WIN32)
  // Put both streams into binary mode
  if (!io_file_flush (stdin)) error_io();
  if (setmode (IO_FD_STDIN, O_BINARY) == -1) error_io();

  if (!io_file_flush (stdout)) error_io();
  if (setmode (IO_FD_STDOUT, O_BINARY) == -1) error_io();
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
  return EXIT_SUCCESS;
}
