###############################################################################
#
# Copyright (C) 2014 haohaolee <hayatelee[at]gmail.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
# MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
# CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
#
###############################################################################
# Debug build
#   cmake -DCMAKE_BUILD_TYPE=Debug .

###############################################################################

cmake_minimum_required(VERSION 3.0)

project(udp_relayer)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  CHECK_CXX_COMPILER_FLAG(-Wno-unused-local-typedefs HAVE_NOUNUSED_LOCAL_TYPEDEF)

  if (HAVE_NOUNUSED_LOCAL_TYPEDEF)
        ADD_DEFINITIONS("-Wno-unused-local-typedefs")
  endif()

  if (NOT HAVE_STD11)
      MESSAGE(FATAL_ERROR "No advanced standard C++ support (-std=c++11 not defined).")
  endif()

  add_definitions("-std=c++11 -Wextra -Wall")

  set(DEF "src/ras_dll.mingw.def")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/W4 /EHsc")
    add_definitions("/D_UNICODE /DUNICODE")
    set(DEF "src/ras_dll.msvc.def")
endif()

##############################################################################
# Set WINVER Definition
add_definitions("/D_WIN32_WINNT=0x0502") # at least for server 2003

###############################################################################
# Boost package
###############################################################################

set(Boost_USE_STATIC_LIBS    ON)
find_package(Boost 1.49.0 COMPONENTS thread system date_time chrono regex REQUIRED)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "Boost not found (or too old)")
endif()
include_directories(BEFORE ${Boost_INCLUDE_DIR})

###############################################################################
# udp_relayer package
set (CMAKE_CXX_FLAGS_DEBUG  "-D_DEBUG")

include_directories(BEFORE include)

add_library(udp_relayer SHARED
    src/udp_relayer.cc
    src/ras_dll.cc
    src/config.cc
    src/server.cc
    ${DEF}
)

set_target_properties(udp_relayer
                    PROPERTIES PREFIX "") # generate library without prefix "lib"

target_link_libraries(udp_relayer ${Boost_LIBRARIES} ws2_32 shlwapi)

