///////////////////////////////////////////////////////////////////////////////
//
// http://github.com/haohaolee/udp_relayer
//
// Copyright (C) 2014 haohaolee <hayatelee[at]gmail.com>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DEBUG_H_INC_
#define DEBUG_H_INC_

#pragma once

#ifdef _DEBUG
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>
#endif

#ifdef _DEBUG
#define ASSERT assert
#define VERIFY ASSERT
#define VERIFY_(result, expression) ASSERT(result == expression)
#else
#define ASSERT(x) (void)0
#define VERIFY(expression) (expression)
#define VERIFY_(result, expression) (expression)
#endif

#ifdef _DEBUG
// disable 4996 in MSVC for unsafe _snprintf
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4996)
#endif

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace {
struct Tracer
{
    char const * m_filenamea;
    wchar_t const * m_filenamew;
    unsigned m_line;

    Tracer(char const * filename, unsigned const line) :
        m_filenamea { filename },
        m_filenamew {nullptr},
        m_line { line }
    {
    }

    
    Tracer(wchar_t const * filename, unsigned const line) :
        m_filenamea { nullptr },
        m_filenamew { filename },
        m_line { line }
    {
    }


    template <typename... Args>
    auto operator()(char const * format, Args... args) const -> void
    {
        char buffer [400];

        ASSERT(m_filenamea != nullptr);
        auto count = _snprintf(buffer,
                               400,
                               "%s(%u): ",
                               m_filenamea,
                               m_line);

        ASSERT(-1 != count && count < 400);

        ASSERT(-1 != _snprintf(buffer + count,
                              _countof(buffer) - count,
                              format,
                              args...));

        ::OutputDebugStringA(buffer);
    }

    template <typename... Args>
    auto operator()(wchar_t const * format, Args... args) const -> void
    {
        wchar_t buffer [400];

        ASSERT(m_filenamew != nullptr);
        auto count = _snwprintf(buffer,
                               400,
                               L"%s(%u): ",
                               m_filenamew,
                               m_line);

        ASSERT(-1 != count && count < 400);

        ASSERT(-1 != _snwprintf(buffer + count,
                              _countof(buffer) - count,
                              format,
                              args...));

        ::OutputDebugStringW(buffer);
    }

};

}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif

#ifdef _DEBUG
    #define WIDEN2(x) L ## x
    #define WIDEN(x) WIDEN2(x)
    #define WFILE WIDEN(__FILE__)

    #define TRACEA  Tracer(__FILE__, __LINE__)
    #define TRACEW  Tracer(WFILE, __LINE__)
    #ifdef _UNICODE
        #define TRACE   TRACEW
    #else
        #define TRACE   TRACEA
    #endif
#else
    #define TRACEA(...)  (void)0
    #define TRACEW(...)  (void)0
    #define TRACE(...)   (void)0
#endif

#endif

