#!/usr/bin/env bash
# ==============================================================================
# Ultraviolet
#
# Copyright Kristian Garnét.
# ------------------------------------------------------------------------------

# Create subdirectories

mkdir converted

mkdir converted/8
mkdir converted/8/16
mkdir converted/8/32

mkdir converted/16
mkdir converted/16/8
mkdir converted/16/32

mkdir converted/32
mkdir converted/32/8
mkdir converted/32/16

# Convert everything

for F in samples/*
do
  B=`basename "$F"`

  # Convert to UTF-8
  bin/utf -o utf-8 < $F > converted/8/$B

  # Convert from UTF-8 to UTF-16
  bin/utf -o utf-16 < converted/8/$B > converted/8/16/$B

  # Convert from UTF-8 to UTF-32
  bin/utf -o utf-32 < converted/8/$B > converted/8/32/$B

  # Convert to UTF-16
  bin/utf -o utf-16 < $F > converted/16/$B

  # Convert from UTF-16 to UTF-8
  bin/utf -o utf-8 < converted/16/$B > converted/16/8/$B

  # Convert from UTF-16 to UTF-32
  bin/utf -o utf-32 < converted/16/$B > converted/16/32/$B

  # Convert to UTF-32
  bin/utf -o utf-32 < $F > converted/32/$B

  # Convert from UTF-32 to UTF-8
  bin/utf -o utf-8 < converted/32/$B > converted/32/8/$B

  # Convert from UTF-32 to UTF-16
  bin/utf -o utf-16 < converted/32/$B > converted/32/16/$B
done
