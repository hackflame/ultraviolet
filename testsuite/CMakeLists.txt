# ==============================================================================
# Ultraviolet
#
# Copyright Kristian Garnét.
# ------------------------------------------------------------------------------

cmake_minimum_required (VERSION 3.0.2)

project (utf)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set (CWIFT_DIR ${CWIFT})
string (REPLACE "\\" "/" CWIFT_DIR ${CWIFT_DIR})
include_directories ("${CWIFT_DIR}")

set (ULTRAVIOLET_DIR ${ULTRAVIOLET})
string (REPLACE "\\" "/" ULTRAVIOLET_DIR ${ULTRAVIOLET_DIR})
include_directories ("${ULTRAVIOLET_DIR}")

include ("${CWIFT_DIR}/testsuite/cwift.cmake")

cwift_configure_compiler_flags()

set (UTF_C "${CMAKE_SOURCE_DIR}/utf.c")
list (APPEND SOURCES ${UTF_C})

set (UTF8_C "${ULTRAVIOLET_DIR}/ultraviolet/utf/utf8.c")
list (APPEND SOURCES ${UTF8_C})

set (UTF16_C "${ULTRAVIOLET_DIR}/ultraviolet/utf/utf16.c")
list (APPEND SOURCES ${UTF16_C})

set (UTF32_C "${ULTRAVIOLET_DIR}/ultraviolet/utf/utf32.c")
list (APPEND SOURCES ${UTF32_C})

set (STRING_BUFFER_C "${CWIFT_DIR}/cwift/string/buffer.c")
list (APPEND SOURCES ${STRING_BUFFER_C})

set (ISTRING_BUFFER_C "${CWIFT_DIR}/cwift/string/implicit/buffer.c")
list (APPEND SOURCES ${ISTRING_BUFFER_C})

set (STRING16_BUFFER_C "${CWIFT_DIR}/cwift/unicode/string16/buffer.c")
list (APPEND SOURCES ${STRING16_BUFFER_C})

set (ISTRING16_BUFFER_C "${CWIFT_DIR}/cwift/unicode/string16/implicit/buffer.c")
list (APPEND SOURCES ${ISTRING16_BUFFER_C})

set (STRING32_BUFFER_C "${CWIFT_DIR}/cwift/unicode/string32/buffer.c")
list (APPEND SOURCES ${STRING32_BUFFER_C})

set (ISTRING32_BUFFER_C "${CWIFT_DIR}/cwift/unicode/string32/implicit/buffer.c")
list (APPEND SOURCES ${ISTRING32_BUFFER_C})

cwift_windows_program (SOURCES)

add_executable (${PROJECT_NAME} ${SOURCES})

cwift_configure_linker_flags()
