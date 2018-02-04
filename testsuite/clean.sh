#!/usr/bin/env bash
# ==============================================================================
# Ultraviolet
#
# Copyright Kristian Garnét.
# ------------------------------------------------------------------------------

if [ -d "./build" ]; then
  if [ -d "./build/CMakeFiles" ]; then
    rm -rf "./build"
  fi
fi

if [ -d "./converted" ]; then
  if [ -d "./converted/8/16" ]; then
    rm -rf "./converted"
  fi
fi

if [ -d "./bin" ]; then
  rm -f "./bin/utf"
  rmdir "./bin"
fi
