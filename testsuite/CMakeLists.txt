# ==============================================================================
# Ultraviolet
#
# Copyright Kristian Garnét.
# ------------------------------------------------------------------------------

cmake_minimum_required (VERSION 3.0.2)

project (utf)

# Directories
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set (QUANTUM_DIR ${QUANTUM})
string (REPLACE "\\" "/" QUANTUM_DIR ${QUANTUM_DIR})
include_directories ("${QUANTUM_DIR}")

set (ULTRAVIOLET_DIR ${ULTRAVIOLET})
string (REPLACE "\\" "/" ULTRAVIOLET_DIR ${ULTRAVIOLET_DIR})
include_directories ("${ULTRAVIOLET_DIR}")

# Flags
if (CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -O3")

  set (LINK_FLAGS_RELEASE "-s")
  set (LINK_FLAGS_DEBUG "")

  if (WIN32)
    set (LINK_FLAGS_RELEASE "-mconsole -municode ${LINK_FLAGS_RELEASE}")
    set (LINK_FLAGS_DEBUG "-mconsole -municode ${LINK_FLAGS_DEBUG}")
  endif()
endif()

# Units
set (UTF_C "${CMAKE_SOURCE_DIR}/utf.c")

set (UTF8_C "${ULTRAVIOLET_DIR}/utf/utf8.c")
set (UTF16_C "${ULTRAVIOLET_DIR}/utf/utf16.c")
set (UTF32_C "${ULTRAVIOLET_DIR}/utf/utf32.c")

if (WIN32)
  set_source_files_properties (${UTF_C} PROPERTIES COMPILE_DEFINITIONS "CONSOLE")
endif()

# Executable
add_executable (${PROJECT_NAME} ${UTF_C} ${UTF8_C} ${UTF16_C} ${UTF32_C})

set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE ${LINK_FLAGS_RELEASE})
set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG ${LINK_FLAGS_DEBUG})

if (WIN32)
  target_link_libraries (${PROJECT_NAME} mingw32 msvcr120)
endif()