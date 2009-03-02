/*
 * ORC - Library of Optimized Inner Loops
 * Copyright (c) 2003,2004 David A. Schleef <ds@schleef.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ORC_DEBUG_H_
#define _ORC_DEBUG_H_

#include <stdarg.h>
#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/**
 * OrcDebugPrintFunc:
 * @level: the debug level
 * @file: name of the file where the debug message occurs
 * @func: name of the function where the debug message occurs
 * @line: line in the file where the debug message occurs
 * @format: a printf format
 * @varargs: varargs for the printf format
 *
 * Typedef describing functions that can be registered using
 * orc_debug_set_print_function() so that it is called to
 * print debugging messages.
 */
typedef void (*OrcDebugPrintFunc) (int level, const char *file,
    const char *func, int line, const char *format, va_list varargs);

/**
 * OrcDebugLevel:
 *
 * Enumeration describing debug levels in Liborc.
 */
typedef enum {
  ORC_DEBUG_NONE = 0,
  ORC_DEBUG_ERROR,
  ORC_DEBUG_WARNING,
  ORC_DEBUG_INFO,
  ORC_DEBUG_DEBUG,
  ORC_DEBUG_LOG
} OrcDebugLevel;

/**
 * ORC_ERROR:
 *
 * Macro to call ORC_DEBUG_PRINT() with a level of #ORC_DEBUG_ERROR.
 */
#define ORC_ERROR(...) ORC_DEBUG_PRINT(ORC_DEBUG_ERROR, __VA_ARGS__)
/**
 * ORC_WARNING:
 *
 * Macro to call ORC_DEBUG_PRINT() with a level of #ORC_DEBUG_WARNING.
 */
#define ORC_WARNING(...) ORC_DEBUG_PRINT(ORC_DEBUG_WARNING, __VA_ARGS__)
/**
 * ORC_INFO:
 *
 * Macro to call ORC_DEBUG_PRINT() with a level of #ORC_DEBUG_INFO.
 */
#define ORC_INFO(...) ORC_DEBUG_PRINT(ORC_DEBUG_INFO, __VA_ARGS__)
/**
 * ORC_DEBUG:
 *
 * Macro to call ORC_DEBUG_PRINT() with a level of #ORC_DEBUG_DEBUG.
 */
#define ORC_DEBUG(...) ORC_DEBUG_PRINT(ORC_DEBUG_DEBUG, __VA_ARGS__)
/**
 * ORC_LOG:
 *
 * Macro to call ORC_DEBUG_PRINT() with a level of #ORC_DEBUG_LOG.
 */
#define ORC_LOG(...) ORC_DEBUG_PRINT(ORC_DEBUG_LOG, __VA_ARGS__)

/**
 * ORC_FUNCTION:
 *
 * Internal macro that points to __PRETTY_FUNCTION__ or __func__
 * if the former is not available.
 */
#if defined (__GNUC__) || defined (__PRETTY_FUNCTION__)
#define ORC_FUNCTION __PRETTY_FUNCTION__
#elif defined(__func__)
#define ORC_FUNCTION __func__
#else
#define ORC_FUNCTION ""
#endif

/**
 * ORC_DEBUG_PRINT:
 * @level:
 * @...:
 *
 * Macro to call orc_debug_print() with the correct values for
 * the name of the source file, line of source file, and function.
 */
#define ORC_DEBUG_PRINT(level, ...) do { \
  orc_debug_print((level), __FILE__, ORC_FUNCTION, __LINE__, __VA_ARGS__); \
}while(0)

void orc_debug_set_print_function (OrcDebugPrintFunc func);
int orc_debug_get_level (void);
void orc_debug_set_level (int level);

void _orc_debug_init (void);

void orc_debug_print (int level, const char *file, const char *func,
    int line, const char *format, ...);

#endif

ORC_END_DECLS

#endif

