# Ultraviolet

**Ultraviolet** is a fast and secure C99 UTF validation and conversion library.

It was created because other C UTF libraries ~~are junk~~ didn’t provide
quite enough features or flexibility in their APIs for author’s needs.

## Usage

The library is fully templated and provides functions for both
*implicit* (null-terminated) and *explicit* strings for all
UTF encodings: **UTF-8**, **UTF-16**, and **UTF-32**.

The API is useful for *streaming* and presents a uniform interface:

```c
int utf8_str_to16 (const u8* restrict in, size_t len, u16* restrict out
, size_t size, u8** end, size_t* num);

int utf8_str_to16_fast (const u8* restrict in, size_t len
, u16* restrict out, size_t size, u8** end, size_t* num);

int utf8_stri_to16 (const u8* restrict in, u16* restrict out, size_t size
, u8** end, size_t* num);

int utf8_stri_to16_fast (const u8* restrict in, u16* restrict out
, size_t size, u8** end, size_t* num);
```

Functions with `_fast` suffix skip validation and perform only security checks.

The return value is interpreted as follows:

  * `INT_MIN`: the input is invalid. `end` points to the first ubyte
    of the invalid sequence. `num` represents the number of ubytes
    written to the output buffer until the error occurred.
    The validation or conversion must not continue.

  * `ret < 0`: the input is valid, except for the last multibyte sequence,
    which lacks exactly `-ret` ubytes.

    If the there’s no more input data available, then the input must be
    considered invalid and the process must stop.

    Otherwise, you should get at least `-ret` more ubytes of the input data
    and continue the validation (or conversion) process. Remember that `end`
    points to the first ubyte of the incomplete sequence, so you can continue
    from that position instead of from the very beginning: adjust the output
    buffer position accordingly with the help of `num`.

    This return value never occurs for UTF-32 input
    because it cannot have multibyte sequences.

  * `ret == 0`: the validation or conversion was successful.

  * `ret > 0`: the input is valid, but the output buffer lacked exactly `ret`
    ubytes for conversion to be successful.

    If the output buffer cannot be increased in size or flushed, then `num`
    output ubytes is all that you can have.

    Otherwise, you should grow the output buffer by at least `ret` more ubytes
    and continue the process from the position pointed by `end`: don’t forget
    to adjust the output buffer position with `num`.

    This return value occurs only during conversion.

  * `INT_MAX`: this return value is reserved for future possible use.

The term `ubyte` refers to the most basic unit of the particular UTF encoding:
8-bit byte for UTF-8, 16-bit byte for UTF-16, and 32-bit byte for UTF-32.

The rest of UTF conversion functions follow the same interface.

Read the fully commented [utf.h](https://github.com/garnetius/ultraviolet/utf/utf.h)
header to see what else is there for you to play with.

## SIMD

**Ultraviolet** includes SSE code paths for all conversion functions
to find out how feasible vectorization of UTF conversion is.

How much does it help? The answer is not much.

There may be a slight speed up for texts composed in several languages
mixed with each other, but in reality inputs usually come in one language
with fairly predictable multibyte sequence patterns.

Additionally, SSE ISA is not very “UTF-friendly”, so to say, because its
shuffle instructions tend to be suboptimal for the task.

Libraries which claim significant speedups over scalar UTF processing
using vector instructions do so because their scalar code
is probably pure crap.

## Example UTF Converter

A fully functional UTF converter program built with **Ultraviolet** is included
in the repository and servers as a great example of how to use the library.

It detects byte order marks, validates and converts from / to UTF-8,
UTF-16 (host endianness), UTF-16LE (little-endian), UTF-16BE (big-endian),
UTF-32 (host endianness), UTF-32LE (little-endian), and UTF-32BE (big-endian).

Run it with `-help` argument for detailed usage instructions.

## Building

You will need [QUANTUM](https://github.com/garnetius/quantum) headers
to build **Ultraviolet**.

Otherwise, the process is straightforward.
