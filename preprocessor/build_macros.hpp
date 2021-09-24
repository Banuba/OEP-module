#pragma once

#include "config.hpp"

#ifdef DEBUG
    #define ONLY_DEBUG(expr)        expr
#else
    #define ONLY_DEBUG(expr)        ((void)0)
#endif

/* desktop build */
#if defined(DESKTOP_BUILD_PART)
    #define IF_DESKTOP(expr)        expr
#else
    #define IF_DESKTOP(expr)
#endif

/* native macos build */
#if defined(NATIVE_MACOS_BUILD_PART)
    #define IF_NATIVE_MACOS(expr)   expr
#else
    #define IF_NATIVE_MACOS(expr)
#endif

/* android build */
#if defined(ANDROID_BUILD_PART)
    #define IF_ANDROID(expr)        expr
#else
    #define IF_ANDROID(expr)
#endif

/* win32 build */
#if defined(WIN32)
    #define ONLY_WIN32(expr)        expr
#else
    #define ONLY_WIN32(expr)        ((void)0)
#endif /* defined(WIN32) */

/* c-api build */
#if defined(C_API_BUILD_PART)
    #define IF_C_API(expr)          expr
    #define IF_CPP_API(expr)
    #define C_API                   1
    #define CPP_API                 0
#else /* cpp-api build */
    #define IF_C_API(expr)
    #define IF_CPP_API(expr)        expr
    #define C_API                   0
    #define CPP_API                 1
#endif
